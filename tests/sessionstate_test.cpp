// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sessionstate.h"

#include <QTest>

using namespace AutoScroll;

class SessionStateTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void activationReleaseIsSuppressed();
  void cancellationClickPairIsSuppressed();
  void escapePairIsSuppressed();
  void externalCancellationIsIdempotent();
  void inactiveInputPassesThrough();
  void activationPolicy();
};

void SessionStateTest::activationReleaseIsSuppressed() {
  SessionState state;
  state.activate();
  QVERIFY(state.isActive());

  const InputDecision release = state.handleButton(Qt::MiddleButton, false);
  QVERIFY(release.consume);
  QVERIFY(!release.cancel);
  QVERIFY(state.isActive());
}

void SessionStateTest::cancellationClickPairIsSuppressed() {
  SessionState state;
  state.activate();
  state.handleButton(Qt::MiddleButton, false);

  const InputDecision press = state.handleButton(Qt::LeftButton, true);
  QVERIFY(press.consume);
  QVERIFY(press.cancel);
  QVERIFY(state.cancel());

  const InputDecision release = state.handleButton(Qt::LeftButton, false);
  QVERIFY(release.consume);
  QVERIFY(!release.cancel);
}

void SessionStateTest::escapePairIsSuppressed() {
  SessionState state;
  state.activate();

  const InputDecision press = state.handleEscape(true);
  QVERIFY(press.consume);
  QVERIFY(press.cancel);
  QVERIFY(state.cancel());

  const InputDecision repeatedPress = state.handleEscape(true);
  QVERIFY(repeatedPress.consume);
  QVERIFY(!repeatedPress.cancel);

  const InputDecision release = state.handleEscape(false);
  QVERIFY(release.consume);
  QVERIFY(!release.cancel);
}

void SessionStateTest::externalCancellationIsIdempotent() {
  SessionState state;
  state.activate();
  QVERIFY(state.cancel());
  QVERIFY(!state.cancel());
  QVERIFY(!state.isActive());
}

void SessionStateTest::inactiveInputPassesThrough() {
  SessionState state;
  QVERIFY(!state.handleButton(Qt::RightButton, true).consume);
  QVERIFY(!state.handleEscape(true).consume);
}

void SessionStateTest::activationPolicy() {
  ActivationContext context{
      .hasWindow = true,
      .clientWindow = true,
  };
  QVERIFY(canActivate(context));

  context.screenLocked = true;
  QVERIFY(!canActivate(context));
  context.screenLocked = false;
  context.excludedSurface = true;
  QVERIFY(!canActivate(context));
  context.excludedSurface = false;
  context.pointerConstrained = true;
  QVERIFY(!canActivate(context));
  context.pointerConstrained = false;
  context.overDecoration = true;
  QVERIFY(!canActivate(context));
}

QTEST_MAIN(SessionStateTest)

#include "sessionstate_test.moc"
