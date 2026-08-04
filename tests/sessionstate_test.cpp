// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sessionstate.h"

#include <QTest>

using namespace AutoScroll;

class SessionStateTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void activationReleaseIsSuppressed();
  void configuredModifierMustMatchExactly();
  void modifiedActivationWaitsForChordRelease();
  void modifiedActivationHandlesReverseReleaseOrder();
  void modifiedActivationTracksCurrentModifierState();
  void holdActivationStopsOnMiddleRelease();
  void modifiedHoldWaitsForModifierRelease();
  void modifiedHoldReleasedEarlyNeverScrolls();
  void secondWheelEventCancels();
  void wheelCancellationCounterResets();
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
  QVERIFY(state.isScrollReady());

  const InputDecision release = state.handleButton(Qt::MiddleButton, false);
  QVERIFY(release.consume);
  QVERIFY(!release.cancel);
  QVERIFY(state.isActive());
  QVERIFY(state.isScrollReady());
}

void SessionStateTest::configuredModifierMustMatchExactly() {
  QVERIFY(activationModifiersMatch(Qt::NoModifier, Qt::NoModifier));
  QVERIFY(activationModifiersMatch(Qt::ControlModifier, Qt::ControlModifier));
  QVERIFY(!activationModifiersMatch(Qt::NoModifier, Qt::ControlModifier));
  QVERIFY(!activationModifiersMatch(Qt::ControlModifier | Qt::ShiftModifier,
                                    Qt::ControlModifier));
}

void SessionStateTest::modifiedActivationWaitsForChordRelease() {
  SessionState state;
  state.activate(Qt::ControlModifier);
  QVERIFY(state.isActive());
  QVERIFY(!state.isScrollReady());

  const InputDecision release = state.handleButton(Qt::MiddleButton, false);
  QVERIFY(release.consume);
  QVERIFY(!state.isScrollReady());

  state.handleModifiers(Qt::NoModifier);
  QVERIFY(state.isScrollReady());
}

void SessionStateTest::modifiedActivationHandlesReverseReleaseOrder() {
  SessionState state;
  state.activate(Qt::MetaModifier);

  state.handleModifiers(Qt::NoModifier);
  QVERIFY(!state.isScrollReady());

  const InputDecision release = state.handleButton(Qt::MiddleButton, false);
  QVERIFY(release.consume);
  QVERIFY(state.isScrollReady());
}

void SessionStateTest::modifiedActivationTracksCurrentModifierState() {
  SessionState state;
  state.activate(Qt::AltModifier);

  state.handleModifiers(Qt::NoModifier);
  state.handleModifiers(Qt::AltModifier);
  state.handleButton(Qt::MiddleButton, false);
  QVERIFY(!state.isScrollReady());

  state.handleModifiers(Qt::NoModifier);
  QVERIFY(state.isScrollReady());
}

void SessionStateTest::holdActivationStopsOnMiddleRelease() {
  SessionState state;
  state.activate(Qt::NoModifier, ActivationMode::Hold);
  QVERIFY(state.isActive());
  QVERIFY(state.isScrollReady());

  const InputDecision release = state.handleButton(Qt::MiddleButton, false);
  QVERIFY(release.consume);
  QVERIFY(release.cancel);
  QVERIFY(state.isActive());
  QVERIFY(!state.isScrollReady());
  QVERIFY(state.cancel());
}

void SessionStateTest::modifiedHoldWaitsForModifierRelease() {
  SessionState state;
  state.activate(Qt::ControlModifier, ActivationMode::Hold);
  QVERIFY(state.isActive());
  QVERIFY(!state.isScrollReady());

  state.handleModifiers(Qt::NoModifier);
  QVERIFY(state.isScrollReady());

  const InputDecision release = state.handleButton(Qt::MiddleButton, false);
  QVERIFY(release.consume);
  QVERIFY(release.cancel);
  QVERIFY(!state.isScrollReady());
}

void SessionStateTest::modifiedHoldReleasedEarlyNeverScrolls() {
  SessionState state;
  state.activate(Qt::MetaModifier, ActivationMode::Hold);

  const InputDecision release = state.handleButton(Qt::MiddleButton, false);
  QVERIFY(release.consume);
  QVERIFY(release.cancel);
  QVERIFY(!state.isScrollReady());

  QVERIFY(state.cancel());
  state.handleModifiers(Qt::NoModifier);
  QVERIFY(!state.isActive());
  QVERIFY(!state.isScrollReady());
}

void SessionStateTest::secondWheelEventCancels() {
  SessionState state;
  state.activate();

  const InputDecision first = state.handleAxis();
  QVERIFY(!first.consume);
  QVERIFY(!first.cancel);
  QVERIFY(state.isActive());

  const InputDecision second = state.handleAxis();
  QVERIFY(!second.consume);
  QVERIFY(second.cancel);
  QVERIFY(state.isActive());
}

void SessionStateTest::wheelCancellationCounterResets() {
  SessionState state;
  state.activate();
  QVERIFY(!state.handleAxis().cancel);
  QVERIFY(state.cancel());

  state.activate();
  QVERIFY(!state.handleAxis().cancel);
  QVERIFY(state.handleAxis().cancel);
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
  QVERIFY(!state.handleAxis().cancel);
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
  context.overDecoration = false;
  context.excludedApplication = true;
  QVERIFY(!canActivate(context));
}

QTEST_MAIN(SessionStateTest)

#include "sessionstate_test.moc"
