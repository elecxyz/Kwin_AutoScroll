// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "scrollengine.h"

#include <algorithm>
#include <cmath>

namespace AutoScroll {

void ScrollEngine::setSettings(const ScrollSettings &settings) {
  m_settings.deadZone = std::clamp(settings.deadZone, 0.0, 256.0);
  m_settings.maximumSpeed = std::clamp(settings.maximumSpeed, 0.0, 10000.0);
  m_settings.accelerationExponent =
      std::clamp(settings.accelerationExponent, 0.1, 8.0);
  m_settings.horizontalScrolling = settings.horizontalScrolling;
}

const ScrollSettings &ScrollEngine::settings() const { return m_settings; }

qreal ScrollEngine::speedForOffset(qreal offset) const {
  const qreal magnitude = std::abs(offset);
  if (magnitude <= m_settings.deadZone ||
      qFuzzyIsNull(m_settings.maximumSpeed)) {
    return 0.0;
  }

  const qreal excess = magnitude - m_settings.deadZone;
  const qreal normalized = std::clamp(excess / FullSpeedDistance, 0.0, 1.0);
  const qreal speed = m_settings.maximumSpeed *
                      std::pow(normalized, m_settings.accelerationExponent);
  return std::copysign(speed, offset);
}

ScrollFrame ScrollEngine::advance(const QPointF &offset, qreal elapsedSeconds) {
  const qreal boundedElapsed = std::clamp(elapsedSeconds, 0.0, 0.1);
  const qreal horizontalSpeed =
      m_settings.horizontalScrolling ? speedForOffset(offset.x()) : 0.0;
  const qreal verticalSpeed = speedForOffset(offset.y());

  ScrollFrame frame;
  frame.horizontalActive = !qFuzzyIsNull(horizontalSpeed);
  frame.verticalActive = !qFuzzyIsNull(verticalSpeed);
  frame.horizontalStopped = m_horizontalWasActive && !frame.horizontalActive;
  frame.verticalStopped = m_verticalWasActive && !frame.verticalActive;
  frame.horizontalDelta = horizontalSpeed * boundedElapsed;
  frame.verticalDelta = verticalSpeed * boundedElapsed;

  m_horizontalWasActive = frame.horizontalActive;
  m_verticalWasActive = frame.verticalActive;
  return frame;
}

ScrollFrame ScrollEngine::stop() {
  ScrollFrame frame;
  frame.horizontalStopped = m_horizontalWasActive;
  frame.verticalStopped = m_verticalWasActive;
  reset();
  return frame;
}

void ScrollEngine::reset() {
  m_horizontalWasActive = false;
  m_verticalWasActive = false;
}

Direction ScrollEngine::directionForOffset(const QPointF &offset) const {
  const int horizontal = m_settings.horizontalScrolling &&
                                 std::abs(offset.x()) > m_settings.deadZone
                             ? (offset.x() > 0 ? 1 : -1)
                             : 0;
  const int vertical = std::abs(offset.y()) > m_settings.deadZone
                           ? (offset.y() > 0 ? 1 : -1)
                           : 0;

  if (horizontal == 0 && vertical == 0) {
    return Direction::Center;
  }
  if (horizontal == 0) {
    return vertical < 0 ? Direction::North : Direction::South;
  }
  if (vertical == 0) {
    return horizontal < 0 ? Direction::West : Direction::East;
  }
  if (horizontal > 0) {
    return vertical < 0 ? Direction::NorthEast : Direction::SouthEast;
  }
  return vertical < 0 ? Direction::NorthWest : Direction::SouthWest;
}

} // namespace AutoScroll
