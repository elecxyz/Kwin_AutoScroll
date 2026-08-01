// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QComboBox>
#include <QString>

namespace AutoScroll {

class GlyphStyleComboBox : public QComboBox {
  Q_OBJECT
  Q_PROPERTY(QString glyphStyle READ glyphStyle WRITE setGlyphStyle NOTIFY
                 glyphStyleChanged USER true)

public:
  explicit GlyphStyleComboBox(QWidget *parent = nullptr);

  QString glyphStyle() const;
  void setGlyphStyle(const QString &styleId);

Q_SIGNALS:
  void glyphStyleChanged(const QString &styleId);
};

class GlyphSizeComboBox : public QComboBox {
  Q_OBJECT
  Q_PROPERTY(int glyphSize READ glyphSize WRITE setGlyphSize NOTIFY
                 glyphSizeChanged USER true)

public:
  explicit GlyphSizeComboBox(QWidget *parent = nullptr);

  int glyphSize() const;
  void setGlyphSize(int size);
  void setGlyphStyle(const QString &styleId);
  QString previewGlyphStyle() const;

Q_SIGNALS:
  void glyphSizeChanged(int size);

private:
  QString m_glyphStyle = QStringLiteral("breeze-dark");
};

} // namespace AutoScroll
