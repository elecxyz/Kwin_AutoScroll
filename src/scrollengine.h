// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QPointF>

namespace AutoScroll {

struct ScrollSettings {
  qreal deadZone = 24.0;
  qreal maximumSpeed = 800.0;
  qreal accelerationExponent = 1.6;
  bool horizontalScrolling = true;
};

struct ScrollFrame {
  qreal horizontalDelta = 0.0;
  qreal verticalDelta = 0.0;
  bool horizontalActive = false;
  bool verticalActive = false;
  bool horizontalStopped = false;
  bool verticalStopped = false;
};

enum class Direction {
  Center,
  North,
  NorthEast,
  East,
  SouthEast,
  South,
  SouthWest,
  West,
  NorthWest,
};

class ScrollEngine {
public:
  static constexpr qreal FullSpeedDistance = 180.0;

  void setSettings(const ScrollSettings &settings);
  const ScrollSettings &settings() const;

  ScrollFrame advance(const QPointF &offset, qreal elapsedSeconds);
  ScrollFrame stop();
  void reset();

  qreal speedForOffset(qreal offset) const;
  Direction directionForOffset(const QPointF &offset) const;

private:
  ScrollSettings m_settings;
  bool m_horizontalWasActive = false;
  bool m_verticalWasActive = false;
};

} // namespace AutoScroll
