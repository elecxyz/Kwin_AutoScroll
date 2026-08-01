// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QPluginLoader>
#include <QTest>
#include <QVersionNumber>

#include <config-kwin.h>

class PluginMetadataTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void effectMetadata();
  void kcmMetadata();
  void supportedKWinFloor();
};

void PluginMetadataTest::effectMetadata() {
  QPluginLoader loader(QStringLiteral(AUTOSCROLL_EFFECT_PATH));
  const QJsonObject root = loader.metaData();
  QCOMPARE(root.value(QStringLiteral("IID")).toString(),
           QStringLiteral("org.kde.kwin.EffectPluginFactory") +
               QStringLiteral(KWIN_PLUGIN_VERSION_STRING));

  const QJsonObject metadata =
      root.value(QStringLiteral("MetaData")).toObject();
  const QJsonObject plugin =
      metadata.value(QStringLiteral("KPlugin")).toObject();
  QCOMPARE(QFileInfo(QStringLiteral(AUTOSCROLL_EFFECT_PATH)).completeBaseName(),
           QStringLiteral("autoscroll"));
  QVERIFY(!plugin.contains(QStringLiteral("Id")));
  QCOMPARE(plugin.value(QStringLiteral("EnabledByDefault")).toBool(), false);
  QCOMPARE(plugin.value(QStringLiteral("Version")).toString(),
           QStringLiteral(AUTOSCROLL_PROJECT_VERSION));
  QCOMPARE(metadata.value(QStringLiteral("X-DocPath")).toString(),
           QStringLiteral("https://github.com/elecxyz/Kwin_AutoScroll"));
}

void PluginMetadataTest::kcmMetadata() {
  QPluginLoader loader(QStringLiteral(AUTOSCROLL_KCM_PATH));
  const QJsonObject root = loader.metaData();
  QCOMPARE(root.value(QStringLiteral("IID")).toString(),
           QStringLiteral("org.kde.KPluginFactory"));

  const QJsonObject metadata =
      root.value(QStringLiteral("MetaData")).toObject();
  const QJsonObject plugin =
      metadata.value(QStringLiteral("KPlugin")).toObject();
  QCOMPARE(plugin.value(QStringLiteral("Version")).toString(),
           QStringLiteral(AUTOSCROLL_PROJECT_VERSION));
  QCOMPARE(metadata.value(QStringLiteral("X-DocPath")).toString(),
           QStringLiteral("https://github.com/elecxyz/Kwin_AutoScroll"));
}

void PluginMetadataTest::supportedKWinFloor() {
  const QVersionNumber buildKWin =
      QVersionNumber::fromString(QStringLiteral(KWIN_PLUGIN_VERSION_STRING));
  QVERIFY2(buildKWin >= QVersionNumber(6, 4, 3),
           "The build root contains a KWin older than the supported floor");
}

QTEST_MAIN(PluginMetadataTest)

#include "pluginmetadata_test.moc"
