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

enum class ActivationMode {
  Toggle,
  Hold,
};

struct ActivationContext {
  bool hasWindow = false;
  bool clientWindow = false;
  bool excludedSurface = false;
  bool screenLocked = false;
  bool fullscreenEffectActive = false;
  bool pointerConstrained = false;
  bool overDecoration = false;
  bool excludedApplication = false;
};

bool canActivate(const ActivationContext &context);
bool activationModifiersMatch(Qt::KeyboardModifiers actual,
                              Qt::KeyboardModifier required);

class SessionState {
public:
  bool isActive() const;
  bool isScrollReady() const;
  void activate(Qt::KeyboardModifier activationModifier = Qt::NoModifier,
                ActivationMode activationMode = ActivationMode::Toggle);
  bool cancel();

  InputDecision handleButton(Qt::MouseButton button, bool pressed);
  void handleModifiers(Qt::KeyboardModifiers modifiers);
  InputDecision handleAxis();
  InputDecision handleEscape(bool pressed);

private:
  void updateScrollReady();

  bool m_active = false;
  bool m_scrollReady = false;
  bool m_activationButtonHeld = false;
  bool m_activationModifierReleased = false;
  Qt::KeyboardModifier m_activationModifier = Qt::NoModifier;
  ActivationMode m_activationMode = ActivationMode::Toggle;
  bool m_wheelCancellationArmed = false;
  bool m_suppressEscapeRelease = false;
  QSet<Qt::MouseButton> m_suppressedButtons;
};

} // namespace AutoScroll
