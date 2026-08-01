// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrollconfig.h"
#include "glyphselectors.h"
#include "ui_autoscroll_config.h"

#include <KConfigDialogManager>
#include <KSharedConfig>

#include <QMetaProperty>
#include <QTemporaryDir>
#include <QTest>
#include <QWidget>

using namespace AutoScroll;

class GlyphSelectorsTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void styleSelectorUsesStableIds();
  void sizeSelectorUsesPresets();
  void previewStyleCanBeUpdated();
  void visualFeedbackControlsAppearanceGroup();
  void appearanceControlsUseOneCenteredRow();
  void kconfigDialogManagerBindsProperties();
};

void GlyphSelectorsTest::styleSelectorUsesStableIds() {
  GlyphStyleComboBox selector;
  QCOMPARE(selector.count(), 7);
  QCOMPARE(QString::fromLatin1(selector.metaObject()->userProperty().name()),
           QStringLiteral("glyphStyle"));

  const QStringList expected{
      QStringLiteral("breeze-dark"), QStringLiteral("breeze"),
      QStringLiteral("classic"),     QStringLiteral("feather"),
      QStringLiteral("orbit"),       QStringLiteral("circuit"),
      QStringLiteral("pulse")};
  QStringList actual;
  for (int index = 0; index < selector.count(); ++index) {
    actual.append(selector.itemData(index).toString());
  }
  QCOMPARE(actual, expected);

  selector.setGlyphStyle(QStringLiteral("pulse"));
  QCOMPARE(selector.glyphStyle(), QStringLiteral("pulse"));
  QVERIFY(!selector.itemIcon(selector.currentIndex()).isNull());

  selector.setGlyphStyle(QStringLiteral("not-installed"));
  QCOMPARE(selector.glyphStyle(), QStringLiteral("breeze-dark"));
}

void GlyphSelectorsTest::sizeSelectorUsesPresets() {
  GlyphSizeComboBox selector;
  QCOMPARE(selector.count(), 8);
  QCOMPARE(QString::fromLatin1(selector.metaObject()->userProperty().name()),
           QStringLiteral("glyphSize"));

  const QList<int> expected{16, 24, 32, 40, 48, 56, 64, 72};
  QList<int> actual;
  for (int index = 0; index < selector.count(); ++index) {
    actual.append(selector.itemData(index).toInt());
  }
  QCOMPARE(actual, expected);

  selector.setGlyphSize(64);
  QCOMPARE(selector.glyphSize(), 64);
  selector.setGlyphSize(52);
  QCOMPARE(selector.glyphSize(), 48);
}

void GlyphSelectorsTest::previewStyleCanBeUpdated() {
  GlyphSizeComboBox selector;
  QCOMPARE(selector.previewGlyphStyle(), QStringLiteral("breeze-dark"));
  selector.setGlyphStyle(QStringLiteral("circuit"));
  QCOMPARE(selector.previewGlyphStyle(), QStringLiteral("circuit"));
  selector.setGlyphStyle(QStringLiteral("not-installed"));
  QCOMPARE(selector.previewGlyphStyle(), QStringLiteral("breeze-dark"));
}

void GlyphSelectorsTest::visualFeedbackControlsAppearanceGroup() {
  QWidget widget;
  Ui::AutoScrollConfigForm ui;
  ui.setupUi(&widget);

  ui.kcfg_VisualFeedback->setChecked(true);
  QVERIFY(ui.appearanceGroupBox->isEnabled());
  ui.kcfg_VisualFeedback->setChecked(false);
  QVERIFY(!ui.appearanceGroupBox->isEnabled());
  ui.kcfg_VisualFeedback->setChecked(true);
  QVERIFY(ui.appearanceGroupBox->isEnabled());
}

void GlyphSelectorsTest::appearanceControlsUseOneCenteredRow() {
  QWidget widget;
  Ui::AutoScrollConfigForm ui;
  ui.setupUi(&widget);

  QCOMPARE(ui.appearanceLayout->indexOf(ui.glyphStyleLabel), 0);
  QCOMPARE(ui.appearanceLayout->indexOf(ui.kcfg_GlyphStyle), 1);
  QCOMPARE(ui.appearanceLayout->indexOf(ui.kcfg_GlyphSize), 2);
  QCOMPARE(ui.appearanceLayout->itemAt(0)->alignment(), Qt::AlignVCenter);
  QCOMPARE(ui.appearanceLayout->itemAt(1)->alignment(), Qt::AlignVCenter);
  QCOMPARE(ui.appearanceLayout->itemAt(2)->alignment(), Qt::AlignVCenter);
  QCOMPARE(ui.glyphStyleLabel->buddy(), ui.kcfg_GlyphStyle);
  QCOMPARE(ui.kcfg_GlyphStyle->minimumSize(), ui.kcfg_GlyphSize->minimumSize());
  QCOMPARE(ui.kcfg_GlyphStyle->maximumSize(), ui.kcfg_GlyphSize->maximumSize());
  QCOMPARE(ui.kcfg_GlyphStyle->size(), ui.kcfg_GlyphSize->size());
  QCOMPARE(ui.kcfg_GlyphStyle->size(), QSize(200, 52));
  QCOMPARE(
      widget.findChildren<QLabel *>(QStringLiteral("glyphSizeLabel")).size(),
      0);
}

void GlyphSelectorsTest::kconfigDialogManagerBindsProperties() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const KSharedConfig::Ptr config = KSharedConfig::openConfig(
      directory.filePath(QStringLiteral("kwinrc")), KConfig::SimpleConfig);
  AutoScrollConfig settings(config);
  settings.setGlyphStyle(QStringLiteral("circuit"));
  settings.setGlyphSize(64);
  settings.setVisualFeedback(false);
  QVERIFY(settings.save());
  config->sync();

  QWidget widget;
  Ui::AutoScrollConfigForm ui;
  ui.setupUi(&widget);
  KConfigDialogManager manager(&widget, &settings);
  manager.updateWidgets();

  QCOMPARE(ui.kcfg_GlyphStyle->glyphStyle(), QStringLiteral("circuit"));
  QCOMPARE(ui.kcfg_GlyphSize->glyphSize(), 64);
  QVERIFY(!ui.appearanceGroupBox->isEnabled());

  ui.kcfg_GlyphStyle->setGlyphStyle(QStringLiteral("pulse"));
  ui.kcfg_GlyphSize->setGlyphSize(72);
  ui.kcfg_VisualFeedback->setChecked(true);
  manager.updateSettings();
  QCOMPARE(settings.glyphStyle(), QStringLiteral("pulse"));
  QCOMPARE(settings.glyphSize(), 72);
  QVERIFY(settings.visualFeedback());

  manager.updateWidgetsDefault();
  QCOMPARE(ui.kcfg_GlyphStyle->glyphStyle(), QStringLiteral("breeze-dark"));
  QCOMPARE(ui.kcfg_GlyphSize->glyphSize(), 40);
  QVERIFY(ui.appearanceGroupBox->isEnabled());
}

QTEST_MAIN(GlyphSelectorsTest)

#include "glyphselectors_test.moc"
