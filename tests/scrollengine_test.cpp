// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "scrollengine.h"

#include <QTest>

#include <cmath>

using namespace AutoScroll;

class ScrollEngineTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void deadZoneProducesNoMovement();
  void directionAndMagnitude();
  void maximumSpeedIsCapped();
  void elapsedTimeScalesDelta();
  void diagonalAxesAreIndependent();
  void horizontalCanBeDisabled();
  void axisStopIsEmittedOnce();
  void directionClassification();
  void lowSpeedConfigurationRemainsActive();
};

void ScrollEngineTest::deadZoneProducesNoMovement() {
  ScrollEngine engine;
  const ScrollFrame frame = engine.advance(QPointF(24, -24), 0.01);
  QVERIFY(!frame.horizontalActive);
  QVERIFY(!frame.verticalActive);
  QCOMPARE(frame.horizontalDelta, 0.0);
  QCOMPARE(frame.verticalDelta, 0.0);
}

void ScrollEngineTest::directionAndMagnitude() {
  ScrollEngine engine;
  const qreal positive = engine.speedForOffset(106);
  const qreal negative = engine.speedForOffset(-106);
  QVERIFY(positive > 0);
  QCOMPARE(negative, -positive);
  QVERIFY(positive < engine.settings().maximumSpeed);
}

void ScrollEngineTest::maximumSpeedIsCapped() {
  ScrollEngine engine;
  QCOMPARE(engine.speedForOffset(10000), 800.0);
  QCOMPARE(engine.speedForOffset(-10000), -800.0);
}

void ScrollEngineTest::elapsedTimeScalesDelta() {
  ScrollEngine engine;
  const ScrollFrame shortFrame = engine.advance(QPointF(0, 100), 0.01);
  engine.reset();
  const ScrollFrame longFrame = engine.advance(QPointF(0, 100), 0.02);
  QCOMPARE(longFrame.verticalDelta, shortFrame.verticalDelta * 2.0);
}

void ScrollEngineTest::diagonalAxesAreIndependent() {
  ScrollEngine engine;
  const ScrollFrame frame = engine.advance(QPointF(80, -120), 0.016);
  QVERIFY(frame.horizontalActive);
  QVERIFY(frame.verticalActive);
  QVERIFY(frame.horizontalDelta > 0);
  QVERIFY(frame.verticalDelta < 0);
  QVERIFY(std::abs(frame.verticalDelta) > std::abs(frame.horizontalDelta));
}

void ScrollEngineTest::horizontalCanBeDisabled() {
  ScrollEngine engine;
  ScrollSettings settings = engine.settings();
  settings.horizontalScrolling = false;
  engine.setSettings(settings);

  const ScrollFrame frame = engine.advance(QPointF(500, 100), 0.016);
  QVERIFY(!frame.horizontalActive);
  QCOMPARE(frame.horizontalDelta, 0.0);
  QVERIFY(frame.verticalActive);
}

void ScrollEngineTest::axisStopIsEmittedOnce() {
  ScrollEngine engine;
  QVERIFY(engine.advance(QPointF(0, 100), 0.016).verticalActive);

  const ScrollFrame stopped = engine.advance(QPointF(0, 0), 0.016);
  QVERIFY(stopped.verticalStopped);
  QVERIFY(!stopped.verticalActive);

  const ScrollFrame subsequent = engine.advance(QPointF(0, 0), 0.016);
  QVERIFY(!subsequent.verticalStopped);
}

void ScrollEngineTest::directionClassification() {
  ScrollEngine engine;
  QCOMPARE(engine.directionForOffset(QPointF(0, 0)), Direction::Center);
  QCOMPARE(engine.directionForOffset(QPointF(0, -100)), Direction::North);
  QCOMPARE(engine.directionForOffset(QPointF(100, -100)), Direction::NorthEast);
  QCOMPARE(engine.directionForOffset(QPointF(100, 0)), Direction::East);
  QCOMPARE(engine.directionForOffset(QPointF(-100, 100)), Direction::SouthWest);
}

void ScrollEngineTest::lowSpeedConfigurationRemainsActive() {
  ScrollEngine engine;
  engine.setSettings({
      .deadZone = 24.0,
      .maximumSpeed = 500.0,
      .accelerationExponent = 0.8,
      .horizontalScrolling = true,
  });

  const ScrollFrame frame = engine.advance(QPointF(0, 100), 0.008);
  QVERIFY(frame.verticalActive);
  QVERIFY(frame.verticalDelta > 2.0);
  QVERIFY(frame.verticalDelta < 3.0);
}

QTEST_MAIN(ScrollEngineTest)

#include "scrollengine_test.moc"
