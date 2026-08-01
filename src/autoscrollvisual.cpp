// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrollvisual.h"

#include "glyphstyle.h"

#include <scene/imageitem.h>
#include <scene/item.h>
#ifdef AUTOSCROLL_USE_LEGACY_IMAGE_ITEM_FACTORY
#include <scene/itemrenderer.h>
#include <scene/scene.h>
#endif

namespace AutoScroll {
namespace {
constexpr qreal DirectionSizeRatio = 0.8;

std::unique_ptr<KWin::ImageItem> createImageItem(KWin::Item *parent) {
#ifdef AUTOSCROLL_USE_LEGACY_IMAGE_ITEM_FACTORY
  return parent->scene()->renderer()->createImageItem(parent);
#else
  return std::make_unique<KWin::ImageItem>(parent);
#endif
}

} // namespace

AutoScrollVisual::AutoScrollVisual(KWin::Item *overlayItem)
    : m_overlayItem(overlayItem) {}

AutoScrollVisual::~AutoScrollVisual() = default;

void AutoScrollVisual::setAppearance(const QString &styleId, int glyphSize) {
  const QString normalizedStyle = normalizedGlyphStyleId(styleId);
  const qreal normalizedSize = normalizedGlyphSize(glyphSize);
  if (m_styleId == normalizedStyle &&
      qFuzzyCompare(m_anchorSize, normalizedSize)) {
    return;
  }

  m_styleId = normalizedStyle;
  m_anchorSize = normalizedSize;
  const qreal directionSize = m_anchorSize * DirectionSizeRatio;

  if (m_anchorItem) {
    m_anchorItem->setImage(renderAnchor(m_scale));
    m_anchorItem->setSize(QSizeF(m_anchorSize, m_anchorSize));
    m_anchorItem->setPosition(m_anchorPosition -
                              QPointF(m_anchorSize / 2.0, m_anchorSize / 2.0));
  }
  if (m_cursorItem) {
    m_cursorItem->setSize(QSizeF(directionSize, directionSize));
    m_cursorItem->setPosition(
        m_cursorPosition - QPointF(directionSize / 2.0, directionSize / 2.0));
    if (m_direction != Direction::Center) {
      m_cursorItem->setImage(renderDirection(m_direction, m_scale));
    }
  }
}

void AutoScrollVisual::show(const QPointF &anchorPosition,
                            const QPointF &cursorPosition, qreal scale) {
  hide();
  m_anchorPosition = anchorPosition;
  m_cursorPosition = cursorPosition;
  m_scale = std::max(scale, 1.0);
  m_direction = Direction::Center;

  m_anchorItem = createImageItem(m_overlayItem);
  m_anchorItem->setImage(renderAnchor(m_scale));
  m_anchorItem->setSize(QSizeF(m_anchorSize, m_anchorSize));
  m_anchorItem->setPosition(m_anchorPosition -
                            QPointF(m_anchorSize / 2.0, m_anchorSize / 2.0));
  m_anchorItem->setZ(1000);

  const qreal directionSize = m_anchorSize * DirectionSizeRatio;
  m_cursorItem = createImageItem(m_overlayItem);
  m_cursorItem->setSize(QSizeF(directionSize, directionSize));
  m_cursorItem->setZ(1001);
  m_cursorItem->setVisible(false);
}

void AutoScrollVisual::update(const QPointF &cursorPosition,
                              Direction direction, qreal scale) {
  if (!m_anchorItem || !m_cursorItem) {
    return;
  }

  const qreal boundedScale = std::max(scale, 1.0);
  const bool imageChanged =
      direction != m_direction || !qFuzzyCompare(boundedScale, m_scale);
  m_cursorPosition = cursorPosition;
  m_direction = direction;
  m_scale = boundedScale;

  if (!qFuzzyCompare(m_anchorItem->image().devicePixelRatio(), m_scale)) {
    m_anchorItem->setImage(renderAnchor(m_scale));
  }

  if (m_direction == Direction::Center) {
    m_cursorItem->setVisible(false);
    return;
  }

  if (imageChanged) {
    m_cursorItem->setImage(renderDirection(m_direction, m_scale));
  }
  const qreal directionSize = m_anchorSize * DirectionSizeRatio;
  m_cursorItem->setPosition(m_cursorPosition -
                            QPointF(directionSize / 2.0, directionSize / 2.0));
  m_cursorItem->setVisible(true);
}

void AutoScrollVisual::hide() {
  m_cursorItem.reset();
  m_anchorItem.reset();
}

bool AutoScrollVisual::isVisible() const { return m_anchorItem != nullptr; }

QImage AutoScrollVisual::renderAnchor(qreal scale) const {
  return renderGlyph(m_styleId, GlyphElement::Anchor, m_anchorSize, scale);
}

QImage AutoScrollVisual::renderDirection(Direction direction,
                                         qreal scale) const {
  return renderGlyph(m_styleId, GlyphElement::Direction,
                     m_anchorSize * DirectionSizeRatio, scale,
                     rotationForDirection(direction));
}

qreal AutoScrollVisual::rotationForDirection(Direction direction) {
  switch (direction) {
  case Direction::North:
    return 0.0;
  case Direction::NorthEast:
    return 45.0;
  case Direction::East:
    return 90.0;
  case Direction::SouthEast:
    return 135.0;
  case Direction::South:
    return 180.0;
  case Direction::SouthWest:
    return 225.0;
  case Direction::West:
    return 270.0;
  case Direction::NorthWest:
    return 315.0;
  case Direction::Center:
    return 0.0;
  }
  return 0.0;
}

} // namespace AutoScroll
