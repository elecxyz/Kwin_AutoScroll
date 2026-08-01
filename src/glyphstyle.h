// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QString>
#include <QStringView>

#include <array>

namespace AutoScroll {

enum class GlyphElement {
  Anchor,
  Direction,
};

struct GlyphStyle {
  const char *id;
  const char *anchorResource;
  const char *directionResource;
};

const std::array<GlyphStyle, 7> &glyphStyles();
const GlyphStyle &glyphStyle(QStringView id);
QString normalizedGlyphStyleId(QStringView id);

const std::array<int, 8> &glyphSizePresets();
int normalizedGlyphSize(int size);

QImage renderGlyph(QStringView styleId, GlyphElement element, qreal logicalSize,
                   qreal scale, qreal rotation = 0.0);

} // namespace AutoScroll
