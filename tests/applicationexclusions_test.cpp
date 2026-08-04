// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "applicationexclusions.h"

#include <QTest>

using namespace AutoScroll;

class ApplicationExclusionsTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void normalizesTypedIdentifiers();
  void normalizesListsAndDropsMalformedEntries();
  void desktopIdentityTakesPriorityOverClassFallback();
  void matchesFallbackResourceClass();
};

void ApplicationExclusionsTest::normalizesTypedIdentifiers() {
  QCOMPARE(desktopApplicationExclusion(u" org.mozilla.firefox.desktop "),
           QStringLiteral("desktop:org.mozilla.firefox"));
  QCOMPARE(classApplicationExclusion(u"  FireFox  "),
           QStringLiteral("class:firefox"));
  QCOMPARE(normalizedApplicationExclusion(u"DESKTOP:org.kde.okular"),
           QStringLiteral("desktop:org.kde.okular"));
  QVERIFY(desktopApplicationExclusion(u"../unsafe").isEmpty());
  QVERIFY(classApplicationExclusion(u"bad:class").isEmpty());
  QVERIFY(normalizedApplicationExclusion(u"unknown:value").isEmpty());
}

void ApplicationExclusionsTest::normalizesListsAndDropsMalformedEntries() {
  const QStringList actual = normalizedApplicationExclusions({
      QStringLiteral("desktop:org.kde.kate.desktop"),
      QStringLiteral("desktop:org.kde.kate"),
      QStringLiteral("class: Steam "),
      QStringLiteral("invalid"),
      QStringLiteral("class:steam"),
  });
  QCOMPARE(actual, QStringList({QStringLiteral("desktop:org.kde.kate"),
                                QStringLiteral("class:steam")}));
}

void ApplicationExclusionsTest::desktopIdentityTakesPriorityOverClassFallback() {
  const ApplicationIdentity identity{
      .desktopFileId = QStringLiteral("org.example.Editor.desktop"),
      .resourceClass = QStringLiteral("shared-class"),
  };
  QCOMPARE(applicationExclusionForIdentity(identity),
           QStringLiteral("desktop:org.example.Editor"));
  QVERIFY(isApplicationExcluded(
      identity, {QStringLiteral("desktop:org.example.Editor")}));
  QVERIFY(!isApplicationExcluded(identity,
                                 {QStringLiteral("class:shared-class")}));
}

void ApplicationExclusionsTest::matchesFallbackResourceClass() {
  const ApplicationIdentity identity{
      .desktopFileId = {},
      .resourceClass = QStringLiteral("GameScope"),
  };
  QCOMPARE(applicationExclusionForIdentity(identity),
           QStringLiteral("class:gamescope"));
  QVERIFY(isApplicationExcluded(identity, {QStringLiteral("class:GAMESCOPE")}));
  QVERIFY(!isApplicationExcluded(identity, {}));
}

QTEST_MAIN(ApplicationExclusionsTest)

#include "applicationexclusions_test.moc"
