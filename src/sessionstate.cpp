// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sessionstate.h"

#include <utility>

namespace AutoScroll {

bool canActivate(const ActivationContext &context) {
  return context.hasWindow && context.clientWindow &&
         !context.excludedSurface && !context.screenLocked &&
         !context.fullscreenEffectActive && !context.pointerConstrained &&
         !context.overDecoration && !context.excludedApplication;
}

bool activationModifiersMatch(Qt::KeyboardModifiers actual,
                              Qt::KeyboardModifier required) {
  return actual == Qt::KeyboardModifiers(required);
}

bool SessionState::isActive() const { return m_active; }

bool SessionState::isScrollReady() const { return m_active && m_scrollReady; }

void SessionState::activate(Qt::KeyboardModifier activationModifier,
                            ActivationMode activationMode) {
  m_active = true;
  m_activationModifier = activationModifier;
  m_activationMode = activationMode;
  m_activationButtonHeld = true;
  m_activationModifierReleased = activationModifier == Qt::NoModifier;
  updateScrollReady();
  m_wheelCancellationArmed = false;
  m_suppressedButtons.insert(Qt::MiddleButton);
}

bool SessionState::cancel() {
  const bool wasActive = std::exchange(m_active, false);
  m_scrollReady = false;
  m_activationButtonHeld = false;
  m_activationModifierReleased = false;
  m_activationModifier = Qt::NoModifier;
  m_activationMode = ActivationMode::Toggle;
  m_wheelCancellationArmed = false;
  return wasActive;
}

InputDecision SessionState::handleButton(Qt::MouseButton button, bool pressed) {
  if (!pressed && m_suppressedButtons.remove(button)) {
    if (m_active && button == Qt::MiddleButton) {
      m_activationButtonHeld = false;
      if (m_activationMode == ActivationMode::Hold) {
        m_scrollReady = false;
        return {.consume = true, .cancel = true};
      }
      updateScrollReady();
    }
    return {.consume = true};
  }

  if (!m_active || !pressed) {
    return {};
  }

  m_suppressedButtons.insert(button);
  return {.consume = true, .cancel = true};
}

void SessionState::handleModifiers(Qt::KeyboardModifiers modifiers) {
  if (!m_active || m_activationModifier == Qt::NoModifier) {
    return;
  }
  m_activationModifierReleased = !modifiers.testFlag(m_activationModifier);
  updateScrollReady();
}

InputDecision SessionState::handleAxis() {
  if (!m_active) {
    return {};
  }
  if (std::exchange(m_wheelCancellationArmed, true)) {
    return {.cancel = true};
  }
  return {};
}

InputDecision SessionState::handleEscape(bool pressed) {
  if (pressed && m_suppressEscapeRelease) {
    return {.consume = true};
  }

  if (!pressed && std::exchange(m_suppressEscapeRelease, false)) {
    return {.consume = true};
  }

  if (!m_active || !pressed) {
    return {};
  }

  m_suppressEscapeRelease = true;
  return {.consume = true, .cancel = true};
}

void SessionState::updateScrollReady() {
  if (m_activationMode == ActivationMode::Hold) {
    m_scrollReady = m_activationButtonHeld && m_activationModifierReleased;
    return;
  }

  m_scrollReady = m_activationModifier == Qt::NoModifier ||
                  (!m_activationButtonHeld && m_activationModifierReleased);
}

} // namespace AutoScroll
