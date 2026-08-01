// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "glyphstyle.h"

#include <QFile>
#include <QSet>
#include <QTest>

using namespace AutoScroll;

namespace {
bool hasVisiblePixel(const QImage &image) {
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      if (qAlpha(image.pixel(x, y)) != 0) {
        return true;
      }
    }
  }
  return false;
}
} // namespace

class GlyphStyleTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void registryIsComplete();
  void normalizesConfiguration();
  void rendersEveryStyleAndSize();
  void rendersEveryDirectionRotation();
};

void GlyphStyleTest::registryIsComplete() {
  QSet<QString> ids;
  QCOMPARE(glyphStyles().size(), 7);
  for (const GlyphStyle &style : glyphStyles()) {
    const QString id = QString::fromLatin1(style.id);
    QVERIFY2(!ids.contains(id), qPrintable(id));
    ids.insert(id);
    QVERIFY2(QFile::exists(QString::fromLatin1(style.anchorResource)),
             style.anchorResource);
    QVERIFY2(QFile::exists(QString::fromLatin1(style.directionResource)),
             style.directionResource);
  }
}

void GlyphStyleTest::normalizesConfiguration() {
  QCOMPARE(normalizedGlyphStyleId(u"breeze"), QStringLiteral("breeze"));
  QCOMPARE(normalizedGlyphStyleId(u"breeze-dark"),
           QStringLiteral("breeze-dark"));
  QCOMPARE(normalizedGlyphStyleId(u"pulse"), QStringLiteral("pulse"));
  QCOMPARE(normalizedGlyphStyleId(u"not-installed"),
           QStringLiteral("breeze-dark"));
  QCOMPARE(normalizedGlyphSize(-1), 16);
  QCOMPARE(normalizedGlyphSize(20), 16);
  QCOMPARE(normalizedGlyphSize(28), 24);
  QCOMPARE(normalizedGlyphSize(52), 48);
  QCOMPARE(normalizedGlyphSize(500), 72);
}

void GlyphStyleTest::rendersEveryStyleAndSize() {
  for (const GlyphStyle &style : glyphStyles()) {
    const QString id = QString::fromLatin1(style.id);
    for (const int size : glyphSizePresets()) {
      for (const qreal scale : {1.0, 2.0}) {
        const QImage anchor =
            renderGlyph(id, GlyphElement::Anchor, size, scale);
        const QImage direction =
            renderGlyph(id, GlyphElement::Direction, size * 0.8, scale);
        QVERIFY2(!anchor.isNull(), qPrintable(id));
        QVERIFY2(!direction.isNull(), qPrintable(id));
        QVERIFY2(hasVisiblePixel(anchor), qPrintable(id));
        QVERIFY2(hasVisiblePixel(direction), qPrintable(id));
        QCOMPARE(anchor.devicePixelRatio(), scale);
        QCOMPARE(direction.devicePixelRatio(), scale);
      }
    }
  }
}

void GlyphStyleTest::rendersEveryDirectionRotation() {
  for (int rotation = 0; rotation < 360; rotation += 45) {
    const QImage image = renderGlyph(u"breeze-dark", GlyphElement::Direction,
                                     32.0, 1.0, rotation);
    QVERIFY(hasVisiblePixel(image));
  }
}

QTEST_MAIN(GlyphStyleTest)

#include "glyphstyle_test.moc"
