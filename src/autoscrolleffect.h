// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "scrollengine.h"
#include "sessionstate.h"

#include <effect/effect.h>
#include <input.h>

#include <QElapsedTimer>
#include <QMetaObject>
#include <QPointer>
#include <QSet>
#include <QTimer>

#include <memory>

namespace KWin {
class InputDevice;
class Window;
struct KeyboardKeyEvent;
struct PointerAxisEvent;
struct PointerButtonEvent;
struct PointerMotionEvent;
} // namespace KWin

namespace AutoScroll {

class AutoScrollInputDevice;
class AutoScrollVisual;
class AutoScrollConfig;
class AutoScrollEffect;

class AutoScrollInputFilter : public KWin::InputEventFilter {
public:
  explicit AutoScrollInputFilter(AutoScrollEffect *effect);

  bool pointerMotion(KWin::PointerMotionEvent *event) override;
  bool pointerButton(KWin::PointerButtonEvent *event) override;
  bool pointerAxis(KWin::PointerAxisEvent *event) override;
  bool keyboardKey(KWin::KeyboardKeyEvent *event) override;

private:
  AutoScrollEffect *m_effect;
};

class AutoScrollEffect : public KWin::Effect {
  Q_OBJECT

public:
  AutoScrollEffect();
  ~AutoScrollEffect() override;

  bool isActive() const override;
  void reconfigure(ReconfigureFlags flags) override;

  bool handlePointerMotion(KWin::PointerMotionEvent *event);
  bool handlePointerButton(KWin::PointerButtonEvent *event);
  bool handlePointerAxis(KWin::PointerAxisEvent *event);
  bool handleKeyboardKey(KWin::KeyboardKeyEvent *event);

private:
  bool canActivate(KWin::Window *window) const;
  bool isStillOnTarget() const;
  void activate(KWin::Window *window, const QPointF &position);
  void cancelSession();
  void scrollTick();
  void emitFrame(const ScrollFrame &frame);
  void updateVisual();
  qreal cursorScale() const;
  static std::chrono::microseconds currentTimestamp();

  ScrollEngine m_engine;
  SessionState m_session;
  std::unique_ptr<AutoScrollConfig> m_config;
  std::unique_ptr<AutoScrollInputDevice> m_inputDevice;
  std::unique_ptr<AutoScrollInputFilter> m_inputFilter;
  std::unique_ptr<AutoScrollVisual> m_visual;
  QPointer<KWin::Window> m_targetWindow;
  QMetaObject::Connection m_targetDestroyedConnection;
  QPointF m_anchorPosition;
  QPointF m_cursorPosition;
  QTimer m_scrollTimer;
  QElapsedTimer m_elapsedTimer;
  bool m_visualFeedback = true;
  bool m_cursorHidden = false;
};

} // namespace AutoScroll
