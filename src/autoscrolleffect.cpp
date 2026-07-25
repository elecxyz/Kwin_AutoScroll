// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrolleffect.h"

#include "autoscrollconfig.h"
#include "autoscrollinputdevice.h"
#include "autoscrollvisual.h"

#include <core/output.h>
#include <effect/effecthandler.h>
#include <input_event.h>
#include <pointer_input.h>
#include <scene/workspacescene.h>
#include <window.h>

#include <chrono>

namespace AutoScroll {

AutoScrollInputFilter::AutoScrollInputFilter(AutoScrollEffect *effect)
    : KWin::InputEventFilter(KWin::InputFilterOrder::Effects),
      m_effect(effect) {}

bool AutoScrollInputFilter::pointerMotion(KWin::PointerMotionEvent *event) {
  return m_effect->handlePointerMotion(event);
}

bool AutoScrollInputFilter::pointerButton(KWin::PointerButtonEvent *event) {
  return m_effect->handlePointerButton(event);
}

bool AutoScrollInputFilter::pointerAxis(KWin::PointerAxisEvent *event) {
  return m_effect->handlePointerAxis(event);
}

bool AutoScrollInputFilter::keyboardKey(KWin::KeyboardKeyEvent *event) {
  return m_effect->handleKeyboardKey(event);
}

AutoScrollEffect::AutoScrollEffect() {
  m_config = std::make_unique<AutoScrollConfig>(KWin::effects->config());
  reconfigure(ReconfigureAll);

  m_inputDevice = std::make_unique<AutoScrollInputDevice>();
  KWin::input()->addInputDevice(m_inputDevice.get());

  m_inputFilter = std::make_unique<AutoScrollInputFilter>(this);
  KWin::input()->installInputEventFilter(m_inputFilter.get());

  m_visual =
      std::make_unique<AutoScrollVisual>(KWin::effects->scene()->overlayItem());

  m_scrollTimer.setInterval(8);
  m_scrollTimer.setTimerType(Qt::PreciseTimer);
  connect(&m_scrollTimer, &QTimer::timeout, this,
          &AutoScrollEffect::scrollTick);
  connect(KWin::effects, &KWin::EffectsHandler::screenAboutToLock, this,
          &AutoScrollEffect::cancelSession);
  connect(KWin::effects, &KWin::EffectsHandler::screenLockingChanged, this,
          [this](bool locked) {
            if (locked) {
              cancelSession();
            }
          });
  connect(KWin::effects,
          &KWin::EffectsHandler::hasActiveFullScreenEffectChanged, this,
          [this]() {
            if (KWin::effects->hasActiveFullScreenEffect()) {
              cancelSession();
            }
          });
}

AutoScrollEffect::~AutoScrollEffect() {
  cancelSession();
  if (m_inputFilter) {
    KWin::input()->uninstallInputEventFilter(m_inputFilter.get());
    m_inputFilter.reset();
  }
  if (m_inputDevice) {
    KWin::input()->removeInputDevice(m_inputDevice.get());
    m_inputDevice.reset();
  }
}

bool AutoScrollEffect::isActive() const { return m_session.isActive(); }

void AutoScrollEffect::reconfigure(ReconfigureFlags) {
  m_config->read();
  m_engine.setSettings({
      .deadZone = static_cast<qreal>(m_config->deadZone()),
      .maximumSpeed = static_cast<qreal>(m_config->maximumSpeed()),
      .accelerationExponent = m_config->accelerationExponent(),
      .horizontalScrolling = m_config->horizontalScrolling(),
  });

  const bool visualFeedback = m_config->visualFeedback();
  if (m_visualFeedback != visualFeedback) {
    m_visualFeedback = visualFeedback;
    if (m_session.isActive()) {
      if (m_visualFeedback) {
        if (!m_cursorHidden) {
          KWin::effects->hideCursor();
          m_cursorHidden = true;
        }
        m_visual->show(m_anchorPosition, m_cursorPosition, cursorScale());
        updateVisual();
      } else {
        m_visual->hide();
        if (m_cursorHidden) {
          KWin::effects->showCursor();
          m_cursorHidden = false;
        }
      }
    }
  }
}

bool AutoScrollEffect::handlePointerMotion(KWin::PointerMotionEvent *event) {
  if (!m_session.isActive()) {
    return false;
  }

  m_cursorPosition = event->position;
  if (!isStillOnTarget()) {
    cancelSession();
    return false;
  }

  updateVisual();
  return false;
}

bool AutoScrollEffect::handlePointerButton(KWin::PointerButtonEvent *event) {
  if (event->device == m_inputDevice.get()) {
    return false;
  }

  const bool pressed = event->state == KWin::PointerButtonState::Pressed;
  const InputDecision decision = m_session.handleButton(event->button, pressed);
  if (decision.cancel) {
    cancelSession();
  }
  if (decision.consume) {
    return true;
  }

  if (pressed && event->button == Qt::MiddleButton &&
      event->modifiers == Qt::NoModifier) {
    KWin::Window *window = KWin::input()->findToplevel(event->position);
    if (canActivate(window)) {
      activate(window, event->position);
      return true;
    }
  }

  return false;
}

bool AutoScrollEffect::handlePointerAxis(KWin::PointerAxisEvent *event) {
  if (event->device == m_inputDevice.get()) {
    return false;
  }
  if (m_session.isActive()) {
    cancelSession();
  }
  return false;
}

bool AutoScrollEffect::handleKeyboardKey(KWin::KeyboardKeyEvent *event) {
  if (event->key != Qt::Key_Escape) {
    return false;
  }

  const bool pressed = event->state != KWin::KeyboardKeyState::Released;
  const InputDecision decision = m_session.handleEscape(pressed);
  if (decision.cancel) {
    cancelSession();
  }
  return decision.consume;
}

bool AutoScrollEffect::canActivate(KWin::Window *window) const {
  const bool excludedSurface =
      window &&
      (window->isDeleted() || window->isDesktop() || window->isDock() ||
       window->isInternal() || window->isLockScreen() ||
       window->isLockScreenOverlay() || window->isInputMethod() ||
       window->isOutline() || window->isPopupWindow() || window->isMenu() ||
       window->isDropdownMenu() || window->isPopupMenu() ||
       window->isTooltip() || window->isNotification() ||
       window->isCriticalNotification() || window->isAppletPopup() ||
       window->isOnScreenDisplay());

  return AutoScroll::canActivate({
      .hasWindow = window != nullptr,
      .clientWindow = window && window->isClient(),
      .excludedSurface = excludedSurface,
      .screenLocked = KWin::effects->isScreenLocked(),
      .fullscreenEffectActive = KWin::effects->hasActiveFullScreenEffect(),
      .pointerConstrained = KWin::input()->pointer()->isConstrained(),
      .overDecoration = KWin::input()->pointer()->decoration() != nullptr,
  });
}

bool AutoScrollEffect::isStillOnTarget() const {
  return m_targetWindow && !m_targetWindow->isDeleted() &&
         !KWin::input()->pointer()->decoration() &&
         KWin::input()->findToplevel(m_cursorPosition) == m_targetWindow;
}

void AutoScrollEffect::activate(KWin::Window *window, const QPointF &position) {
  m_session.activate();
  m_targetWindow = window;
  m_anchorPosition = position;
  m_cursorPosition = position;
  m_engine.reset();

  m_targetDestroyedConnection = connect(window, &QObject::destroyed, this,
                                        &AutoScrollEffect::cancelSession);

  if (m_visualFeedback) {
    KWin::effects->hideCursor();
    m_cursorHidden = true;
    m_visual->show(m_anchorPosition, m_cursorPosition, cursorScale());
  }

  m_elapsedTimer.start();
  m_scrollTimer.start();
  KWin::effects->addRepaintFull();
}

void AutoScrollEffect::cancelSession() {
  const bool wasActive = m_session.cancel();
  if (!wasActive && !m_cursorHidden && (!m_visual || !m_visual->isVisible())) {
    return;
  }

  m_scrollTimer.stop();
  if (wasActive && m_inputDevice) {
    emitFrame(m_engine.stop());
  } else {
    m_engine.reset();
  }

  if (m_targetDestroyedConnection) {
    disconnect(m_targetDestroyedConnection);
    m_targetDestroyedConnection = {};
  }
  m_targetWindow.clear();

  if (m_visual) {
    m_visual->hide();
  }
  if (m_cursorHidden) {
    KWin::effects->showCursor();
    m_cursorHidden = false;
  }
  KWin::effects->addRepaintFull();
}

void AutoScrollEffect::scrollTick() {
  if (!m_session.isActive() || !isStillOnTarget()) {
    cancelSession();
    return;
  }

  const qreal elapsedSeconds = m_elapsedTimer.nsecsElapsed() / 1'000'000'000.0;
  m_elapsedTimer.restart();
  const ScrollFrame frame =
      m_engine.advance(m_cursorPosition - m_anchorPosition, elapsedSeconds);
  emitFrame(frame);
}

void AutoScrollEffect::emitFrame(const ScrollFrame &frame) {
  bool emitted = false;
  const auto timestamp = currentTimestamp();

  if (frame.horizontalActive || frame.horizontalStopped) {
    Q_EMIT m_inputDevice->pointerAxisChanged(
        KWin::PointerAxis::Horizontal,
        frame.horizontalActive ? frame.horizontalDelta : 0.0, 0,
        KWin::PointerAxisSource::Continuous, false, timestamp,
        m_inputDevice.get());
    emitted = true;
  }
  if (frame.verticalActive || frame.verticalStopped) {
    Q_EMIT m_inputDevice->pointerAxisChanged(
        KWin::PointerAxis::Vertical,
        frame.verticalActive ? frame.verticalDelta : 0.0, 0,
        KWin::PointerAxisSource::Continuous, false, timestamp,
        m_inputDevice.get());
    emitted = true;
  }
  if (emitted) {
    Q_EMIT m_inputDevice->pointerFrame(m_inputDevice.get());
  }
}

void AutoScrollEffect::updateVisual() {
  if (!m_visualFeedback || !m_visual->isVisible()) {
    return;
  }
  const QPointF offset = m_cursorPosition - m_anchorPosition;
  m_visual->update(m_cursorPosition, m_engine.directionForOffset(offset),
                   cursorScale());
}

qreal AutoScrollEffect::cursorScale() const {
  if (KWin::LogicalOutput *output =
          KWin::effects->screenAt(m_cursorPosition.toPoint())) {
    return output->scale();
  }
  return 1.0;
}

std::chrono::microseconds AutoScrollEffect::currentTimestamp() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now().time_since_epoch());
}

} // namespace AutoScroll
