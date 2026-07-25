// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sessionstate.h"

#include <utility>

namespace AutoScroll {

bool canActivate(const ActivationContext &context) {
  return context.hasWindow && context.clientWindow &&
         !context.excludedSurface && !context.screenLocked &&
         !context.fullscreenEffectActive && !context.pointerConstrained &&
         !context.overDecoration;
}

bool SessionState::isActive() const { return m_active; }

void SessionState::activate() {
  m_active = true;
  m_suppressedButtons.insert(Qt::MiddleButton);
}

bool SessionState::cancel() { return std::exchange(m_active, false); }

InputDecision SessionState::handleButton(Qt::MouseButton button, bool pressed) {
  if (!pressed && m_suppressedButtons.remove(button)) {
    return {.consume = true};
  }

  if (!m_active || !pressed) {
    return {};
  }

  m_suppressedButtons.insert(button);
  return {.consume = true, .cancel = true};
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

} // namespace AutoScroll
