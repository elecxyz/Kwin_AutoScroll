// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QSet>
#include <Qt>

namespace AutoScroll {

struct InputDecision {
  bool consume = false;
  bool cancel = false;
};

struct ActivationContext {
  bool hasWindow = false;
  bool clientWindow = false;
  bool excludedSurface = false;
  bool screenLocked = false;
  bool fullscreenEffectActive = false;
  bool pointerConstrained = false;
  bool overDecoration = false;
};

bool canActivate(const ActivationContext &context);

class SessionState {
public:
  bool isActive() const;
  void activate();
  bool cancel();

  InputDecision handleButton(Qt::MouseButton button, bool pressed);
  InputDecision handleEscape(bool pressed);

private:
  bool m_active = false;
  bool m_suppressEscapeRelease = false;
  QSet<Qt::MouseButton> m_suppressedButtons;
};

} // namespace AutoScroll
