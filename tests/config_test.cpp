// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrollconfig.h"
#include "configmigration.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QTemporaryDir>
#include <QTest>

using namespace AutoScroll;

class ConfigTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void subUnitExponentRoundTrips();
  void activationModifierRoundTrips();
  void activationBehaviorRoundTrips();
  void legacyHoldBehaviorMigrates();
  void legacyToggleBehaviorMigrates();
  void existingActivationBehaviorIsNotOverwritten();
  void initiationAndExclusionDefaults();
  void initiationAndExclusionsRoundTrip();
  void visualDefaults();
  void visualSettingsRoundTrip();
};

void ConfigTest::subUnitExponentRoundTrips() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString path = directory.filePath(QStringLiteral("kwinrc"));

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig writer(config);
    writer.setMaximumSpeed(500);
    writer.setAccelerationExponent(0.8);
    QVERIFY(writer.save());
    config->sync();
  }

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig reader(config);
    reader.read();
    QCOMPARE(reader.maximumSpeed(), 500);
    QCOMPARE(reader.accelerationExponent(), 0.8);
  }
}

void ConfigTest::activationModifierRoundTrips() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString path = directory.filePath(QStringLiteral("kwinrc"));

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig writer(config);
    QCOMPARE(writer.activationModifier(),
             AutoScrollConfig::EnumActivationModifier::NoModifier);
    writer.setActivationModifier(
        AutoScrollConfig::EnumActivationModifier::ControlModifier);
    QVERIFY(writer.save());
    config->sync();
  }

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig reader(config);
    reader.read();
    QCOMPARE(reader.activationModifier(),
             AutoScrollConfig::EnumActivationModifier::ControlModifier);
  }
}

void ConfigTest::activationBehaviorRoundTrips() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString path = directory.filePath(QStringLiteral("kwinrc"));

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig writer(config);
    QCOMPARE(writer.activationBehavior(),
             AutoScrollConfig::EnumActivationBehavior::Toggle);
    writer.setActivationBehavior(
        AutoScrollConfig::EnumActivationBehavior::Combined);
    QVERIFY(writer.save());
    config->sync();
  }

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig reader(config);
    reader.read();
    QCOMPARE(reader.activationBehavior(),
             AutoScrollConfig::EnumActivationBehavior::Combined);
  }
}

void ConfigTest::legacyHoldBehaviorMigrates() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const KSharedConfig::Ptr config = KSharedConfig::openConfig(
      directory.filePath(QStringLiteral("kwinrc")), KConfig::SimpleConfig);
  KConfigGroup group(config, QStringLiteral("Effect-autoscroll"));
  group.writeEntry(QStringLiteral("HoldToScroll"), true);
  config->sync();

  migrateActivationBehavior(config);
  AutoScrollConfig settings(config);
  settings.read();
  QCOMPARE(settings.activationBehavior(),
           AutoScrollConfig::EnumActivationBehavior::Hold);
  QVERIFY(!group.hasKey(QStringLiteral("HoldToScroll")));
}

void ConfigTest::legacyToggleBehaviorMigrates() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const KSharedConfig::Ptr config = KSharedConfig::openConfig(
      directory.filePath(QStringLiteral("kwinrc")), KConfig::SimpleConfig);
  KConfigGroup group(config, QStringLiteral("Effect-autoscroll"));
  group.writeEntry(QStringLiteral("HoldToScroll"), false);
  config->sync();

  migrateActivationBehavior(config);
  AutoScrollConfig settings(config);
  settings.read();
  QCOMPARE(settings.activationBehavior(),
           AutoScrollConfig::EnumActivationBehavior::Toggle);
  QVERIFY(!group.hasKey(QStringLiteral("HoldToScroll")));
}

void ConfigTest::existingActivationBehaviorIsNotOverwritten() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const KSharedConfig::Ptr config = KSharedConfig::openConfig(
      directory.filePath(QStringLiteral("kwinrc")), KConfig::SimpleConfig);
  KConfigGroup group(config, QStringLiteral("Effect-autoscroll"));
  group.writeEntry(QStringLiteral("HoldToScroll"), true);
  group.writeEntry(
      QStringLiteral("ActivationBehavior"),
      static_cast<int>(AutoScrollConfig::EnumActivationBehavior::Combined));
  config->sync();

  migrateActivationBehavior(config);
  AutoScrollConfig settings(config);
  settings.read();
  QCOMPARE(settings.activationBehavior(),
           AutoScrollConfig::EnumActivationBehavior::Combined);
  QVERIFY(!group.hasKey(QStringLiteral("HoldToScroll")));
}

void ConfigTest::initiationAndExclusionDefaults() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const KSharedConfig::Ptr config = KSharedConfig::openConfig(
      directory.filePath(QStringLiteral("kwinrc")), KConfig::SimpleConfig);
  AutoScrollConfig settings(config);

  QVERIFY(!settings.holdToScroll());
  QCOMPARE(settings.activationBehavior(),
           AutoScrollConfig::EnumActivationBehavior::Toggle);
  QVERIFY(settings.excludedApplications().isEmpty());
}

void ConfigTest::initiationAndExclusionsRoundTrip() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString path = directory.filePath(QStringLiteral("kwinrc"));
  const QStringList exclusions{QStringLiteral("desktop:org.mozilla.firefox"),
                               QStringLiteral("class:gamescope")};

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig writer(config);
    writer.setHoldToScroll(true);
    writer.setExcludedApplications(exclusions);
    QVERIFY(writer.save());
    config->sync();
  }

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig reader(config);
    reader.read();
    QVERIFY(reader.holdToScroll());
    QCOMPARE(reader.excludedApplications(), exclusions);
  }
}

void ConfigTest::visualDefaults() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const KSharedConfig::Ptr config = KSharedConfig::openConfig(
      directory.filePath(QStringLiteral("kwinrc")), KConfig::SimpleConfig);
  AutoScrollConfig settings(config);

  QCOMPARE(settings.glyphStyle(), QStringLiteral("breeze-dark"));
  QCOMPARE(settings.glyphSize(), 40);
}

void ConfigTest::visualSettingsRoundTrip() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString path = directory.filePath(QStringLiteral("kwinrc"));

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig writer(config);
    writer.setGlyphStyle(QStringLiteral("circuit"));
    writer.setGlyphSize(64);
    QVERIFY(writer.save());
    config->sync();
  }

  {
    const KSharedConfig::Ptr config =
        KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    AutoScrollConfig reader(config);
    reader.read();
    QCOMPARE(reader.glyphStyle(), QStringLiteral("circuit"));
    QCOMPARE(reader.glyphSize(), 64);
  }
}

QTEST_MAIN(ConfigTest)

#include "config_test.moc"
