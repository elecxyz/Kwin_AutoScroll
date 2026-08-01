// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "scrollengine.h"

#include <QImage>
#include <QPointF>
#include <QString>

#include <memory>

namespace KWin {
class ImageItem;
class Item;
} // namespace KWin

namespace AutoScroll {

class AutoScrollVisual {
public:
  explicit AutoScrollVisual(KWin::Item *overlayItem);
  ~AutoScrollVisual();

  void setAppearance(const QString &styleId, int glyphSize);
  void show(const QPointF &anchorPosition, const QPointF &cursorPosition,
            qreal scale);
  void update(const QPointF &cursorPosition, Direction direction, qreal scale);
  void hide();
  bool isVisible() const;

private:
  QImage renderAnchor(qreal scale) const;
  QImage renderDirection(Direction direction, qreal scale) const;
  static qreal rotationForDirection(Direction direction);

  KWin::Item *m_overlayItem;
  std::unique_ptr<KWin::ImageItem> m_anchorItem;
  std::unique_ptr<KWin::ImageItem> m_cursorItem;
  QPointF m_anchorPosition;
  QPointF m_cursorPosition;
  Direction m_direction = Direction::Center;
  QString m_styleId = QStringLiteral("breeze-dark");
  qreal m_anchorSize = 40.0;
  qreal m_scale = 1.0;
};

} // namespace AutoScroll
