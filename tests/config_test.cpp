// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrollconfig.h"

#include <KSharedConfig>

#include <QTemporaryDir>
#include <QTest>

using namespace AutoScroll;

class ConfigTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void subUnitExponentRoundTrips();
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

QTEST_MAIN(ConfigTest)

#include "config_test.moc"
