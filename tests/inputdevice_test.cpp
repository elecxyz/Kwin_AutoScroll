// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrollinputdevice.h"

#include <QSignalSpy>
#include <QTest>

class InputDeviceTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void scrollPayload_data();
  void scrollPayload();
};

void InputDeviceTest::scrollPayload_data() {
  QTest::addColumn<KWin::PointerAxis>("axis");
  QTest::addColumn<qreal>("delta");
  QTest::newRow("horizontal") << KWin::PointerAxis::Horizontal << qreal(12.5);
  QTest::newRow("vertical") << KWin::PointerAxis::Vertical << qreal(-7.25);
  QTest::newRow("horizontal-stop") << KWin::PointerAxis::Horizontal << qreal(0);
  QTest::newRow("vertical-stop") << KWin::PointerAxis::Vertical << qreal(0);
}

void InputDeviceTest::scrollPayload() {
  QFETCH(KWin::PointerAxis, axis);
  QFETCH(qreal, delta);
  qRegisterMetaType<KWin::PointerAxis>("PointerAxis");
  qRegisterMetaType<KWin::PointerAxisSource>("PointerAxisSource");
  qRegisterMetaType<std::chrono::microseconds>("std::chrono::microseconds");
  qRegisterMetaType<KWin::InputDevice *>("InputDevice*");

  AutoScroll::AutoScrollInputDevice device;
  QSignalSpy spy(&device, &KWin::InputDevice::pointerAxisChanged);
  QVERIFY(spy.isValid());
  const std::chrono::microseconds timestamp(1234567);
  device.scroll(axis, delta, timestamp);
  QCOMPARE(spy.size(), 1);
  const auto arguments = spy.takeFirst();
  QVERIFY(arguments.size() == 7 || arguments.size() == 8);
  QCOMPARE(arguments.at(0).value<KWin::PointerAxis>(), axis);
  QCOMPARE(arguments.at(1).toReal(), delta);
  QCOMPARE(arguments.at(2).toInt(), 0);
  QCOMPARE(arguments.at(3).value<KWin::PointerAxisSource>(),
           KWin::PointerAxisSource::Continuous);
  QCOMPARE(arguments.at(4).toBool(), false);
  if (arguments.size() == 8) {
    QCOMPARE(arguments.at(5).toBool(), true);
  }
  QCOMPARE(arguments.at(arguments.size() - 2).value<std::chrono::microseconds>(),
           timestamp);
  QCOMPARE(arguments.constLast().value<KWin::InputDevice *>(), &device);
}

QTEST_GUILESS_MAIN(InputDeviceTest)

#include "inputdevice_test.moc"
