// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "glyphstyle.h"

#include <QPainter>
#include <QSvgRenderer>

#include <algorithm>
#include <cmath>
#include <limits>

namespace AutoScroll {
namespace {
constexpr std::array<GlyphStyle, 7> Styles{{
    {"breeze-dark", ":/autoscroll/styles/breeze-dark/anchor.svg",
     ":/autoscroll/styles/breeze-dark/direction.svg"},
    {"breeze", ":/autoscroll/styles/breeze/anchor.svg",
     ":/autoscroll/styles/breeze/direction.svg"},
    {"classic", ":/autoscroll/styles/classic/anchor.svg",
     ":/autoscroll/styles/classic/direction.svg"},
    {"feather", ":/autoscroll/styles/feather/anchor.svg",
     ":/autoscroll/styles/feather/direction.svg"},
    {"orbit", ":/autoscroll/styles/orbit/anchor.svg",
     ":/autoscroll/styles/orbit/direction.svg"},
    {"circuit", ":/autoscroll/styles/circuit/anchor.svg",
     ":/autoscroll/styles/circuit/direction.svg"},
    {"pulse", ":/autoscroll/styles/pulse/anchor.svg",
     ":/autoscroll/styles/pulse/direction.svg"},
}};

constexpr std::array<int, 8> SizePresets{{16, 24, 32, 40, 48, 56, 64, 72}};
} // namespace

const std::array<GlyphStyle, 7> &glyphStyles() { return Styles; }

const GlyphStyle &glyphStyle(QStringView id) {
  const auto found = std::find_if(Styles.cbegin(), Styles.cend(),
                                  [id](const GlyphStyle &style) {
                                    return id == QLatin1StringView(style.id);
                                  });
  return found == Styles.cend() ? Styles.front() : *found;
}

QString normalizedGlyphStyleId(QStringView id) {
  return QString::fromLatin1(glyphStyle(id).id);
}

const std::array<int, 8> &glyphSizePresets() { return SizePresets; }

int normalizedGlyphSize(int size) {
  const auto closest = std::min_element(
      SizePresets.cbegin(), SizePresets.cend(), [size](int left, int right) {
        const auto leftDistance = std::abs(static_cast<long long>(size) - left);
        const auto rightDistance =
            std::abs(static_cast<long long>(size) - right);
        if (leftDistance == rightDistance) {
          return left < right;
        }
        return leftDistance < rightDistance;
      });
  return closest == SizePresets.cend() ? 40 : *closest;
}

QImage renderGlyph(QStringView styleId, GlyphElement element, qreal logicalSize,
                   qreal scale, qreal rotation) {
  const GlyphStyle &style = glyphStyle(styleId);
  const QString resource = QString::fromLatin1(element == GlyphElement::Anchor
                                                   ? style.anchorResource
                                                   : style.directionResource);
  const qreal boundedSize = std::max(logicalSize, 1.0);
  const qreal boundedScale = std::max(scale, 1.0);
  const int pixelSize =
      std::max(1, static_cast<int>(std::ceil(boundedSize * boundedScale)));

  QImage image(QSize(pixelSize, pixelSize),
               QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);

  QSvgRenderer renderer(resource);
  if (!renderer.isValid()) {
    image.setDevicePixelRatio(boundedScale);
    return image;
  }

  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  painter.translate(pixelSize / 2.0, pixelSize / 2.0);
  painter.rotate(rotation);
  painter.translate(-pixelSize / 2.0, -pixelSize / 2.0);
  renderer.render(&painter, QRectF(0, 0, pixelSize, pixelSize));
  painter.end();

  image.setDevicePixelRatio(boundedScale);
  return image;
}

} // namespace AutoScroll
