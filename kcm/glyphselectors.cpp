// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "glyphselectors.h"

#include "glyphstyle.h"

#include <KLocalizedString>

#include <QAbstractItemView>
#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyledItemDelegate>

namespace AutoScroll {
namespace {
constexpr int SizeRole = Qt::UserRole;
constexpr int AppearanceControlWidth = 200;
constexpr int AppearanceControlHeight = 52;

void setAppearanceControlSize(QComboBox *comboBox) {
  comboBox->setFixedSize(AppearanceControlWidth, AppearanceControlHeight);
}

QString styleName(QStringView styleId) {
  if (styleId == u"breeze-dark") {
    return i18nc("@item:inlistbox autoscroll glyph style", "Breeze Dark");
  }
  if (styleId == u"breeze") {
    return i18nc("@item:inlistbox autoscroll glyph style", "Breeze");
  }
  if (styleId == u"classic") {
    return i18nc("@item:inlistbox autoscroll glyph style", "Classic");
  }
  if (styleId == u"feather") {
    return i18nc("@item:inlistbox autoscroll glyph style", "Feather");
  }
  if (styleId == u"orbit") {
    return i18nc("@item:inlistbox autoscroll glyph style", "Orbit");
  }
  if (styleId == u"circuit") {
    return i18nc("@item:inlistbox autoscroll glyph style", "Circuit");
  }
  if (styleId == u"pulse") {
    return i18nc("@item:inlistbox autoscroll glyph style", "Pulse");
  }
  return i18nc("@item:inlistbox autoscroll glyph style", "Breeze Dark");
}

QIcon stylePreview(QStringView styleId) {
  constexpr int Width = 76;
  constexpr int Height = 40;
  QImage preview(QSize(Width, Height), QImage::Format_ARGB32_Premultiplied);
  preview.fill(Qt::transparent);

  const QImage anchor = renderGlyph(styleId, GlyphElement::Anchor, 34.0, 1.0);
  const QImage direction =
      renderGlyph(styleId, GlyphElement::Direction, 27.2, 1.0);

  QPainter painter(&preview);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.drawImage(QRectF(3.0, 3.0, 34.0, 34.0), anchor);
  painter.drawImage(QRectF(45.0, 6.4, 27.2, 27.2), direction);
  return QIcon(QPixmap::fromImage(preview));
}

class GlyphSizeDelegate final : public QStyledItemDelegate {
public:
  explicit GlyphSizeDelegate(QObject *parent = nullptr)
      : QStyledItemDelegate(parent) {}

  void setGlyphStyle(const QString &styleId) {
    m_glyphStyle = normalizedGlyphStyleId(styleId);
  }

  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override {
    const int size = index.data(SizeRole).toInt();
    const QSize base = QStyledItemDelegate::sizeHint(option, index);
    return QSize(std::max(base.width(), 172),
                 std::max(base.height(), size + 12));
  }

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    QStyleOptionViewItem backgroundOption(option);
    initStyleOption(&backgroundOption, index);
    backgroundOption.text.clear();
    backgroundOption.icon = {};
    const QStyle *style =
        option.widget ? option.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &backgroundOption, painter,
                       option.widget);

    const int size = index.data(SizeRole).toInt();
    const qreal scale =
        painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    const QImage glyph =
        renderGlyph(m_glyphStyle, GlyphElement::Anchor, size, scale);
    const QRectF glyphRect(option.rect.left() + 8.0,
                           option.rect.center().y() - size / 2.0, size, size);
    painter->drawImage(glyphRect, glyph);

    const QRect textRect(option.rect.left() + 92, option.rect.top(),
                         std::max(0, option.rect.width() - 100),
                         option.rect.height());
    painter->setPen(option.state.testFlag(QStyle::State_Selected)
                        ? option.palette.highlightedText().color()
                        : option.palette.text().color());
    painter->drawText(
        textRect, Qt::AlignVCenter | Qt::AlignLeft,
        i18nc("@item:inlistbox autoscroll glyph size", "Size: %1", size));
  }

private:
  QString m_glyphStyle = QStringLiteral("breeze-dark");
};
} // namespace

GlyphStyleComboBox::GlyphStyleComboBox(QWidget *parent) : QComboBox(parent) {
  setAppearanceControlSize(this);
  setIconSize(QSize(76, 40));
  for (const GlyphStyle &style : glyphStyles()) {
    const QString id = QString::fromLatin1(style.id);
    addItem(stylePreview(id), styleName(id), id);
  }

  connect(this, &QComboBox::currentIndexChanged, this,
          [this](int) { Q_EMIT glyphStyleChanged(glyphStyle()); });
}

QString GlyphStyleComboBox::glyphStyle() const {
  return normalizedGlyphStyleId(currentData().toString());
}

void GlyphStyleComboBox::setGlyphStyle(const QString &styleId) {
  const QString normalized = normalizedGlyphStyleId(styleId);
  const int index = findData(normalized);
  setCurrentIndex(index < 0 ? 0 : index);
}

GlyphSizeComboBox::GlyphSizeComboBox(QWidget *parent) : QComboBox(parent) {
  setAppearanceControlSize(this);
  auto *delegate = new GlyphSizeDelegate(this);
  setItemDelegate(delegate);
  view()->setMinimumWidth(188);

  for (const int size : glyphSizePresets()) {
    addItem(i18nc("@item:inlistbox autoscroll glyph size", "Size: %1", size),
            size);
  }

  connect(this, &QComboBox::currentIndexChanged, this,
          [this](int) { Q_EMIT glyphSizeChanged(glyphSize()); });
}

int GlyphSizeComboBox::glyphSize() const {
  return normalizedGlyphSize(currentData().toInt());
}

void GlyphSizeComboBox::setGlyphSize(int size) {
  const int normalized = normalizedGlyphSize(size);
  const int index = findData(normalized);
  setCurrentIndex(index < 0 ? findData(40) : index);
}

void GlyphSizeComboBox::setGlyphStyle(const QString &styleId) {
  m_glyphStyle = normalizedGlyphStyleId(styleId);
  if (auto *delegate = dynamic_cast<GlyphSizeDelegate *>(itemDelegate())) {
    delegate->setGlyphStyle(m_glyphStyle);
  }
  view()->viewport()->update();
}

QString GlyphSizeComboBox::previewGlyphStyle() const { return m_glyphStyle; }

} // namespace AutoScroll
