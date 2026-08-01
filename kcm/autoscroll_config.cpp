// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscroll_config.h"

#include "autoscrollconfig.h"

#include <KPluginFactory>

#include <KSharedConfig>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

K_PLUGIN_CLASS_WITH_JSON(AutoScroll::AutoScrollEffectConfig, "metadata.json")

namespace AutoScroll {

AutoScrollEffectConfig::AutoScrollEffectConfig(QObject *parent,
                                               const KPluginMetaData &data)
    : KCModule(parent, data) {
  m_ui.setupUi(widget());
  m_config = std::make_unique<AutoScrollConfig>(
      KSharedConfig::openConfig(QStringLiteral("kwinrc")));
  m_config->read();
  addConfig(m_config.get(), widget());

  connect(m_ui.kcfg_GlyphStyle, &GlyphStyleComboBox::glyphStyleChanged,
          m_ui.kcfg_GlyphSize, &GlyphSizeComboBox::setGlyphStyle);
  m_ui.kcfg_GlyphSize->setGlyphStyle(m_ui.kcfg_GlyphStyle->glyphStyle());
  m_ui.appearanceGroupBox->setEnabled(m_ui.kcfg_VisualFeedback->isChecked());
}

AutoScrollEffectConfig::~AutoScrollEffectConfig() = default;

void AutoScrollEffectConfig::save() {
  KCModule::save();

  QDBusMessage message = QDBusMessage::createMethodCall(
      QStringLiteral("org.kde.KWin"), QStringLiteral("/Effects"),
      QStringLiteral("org.kde.kwin.Effects"),
      QStringLiteral("reconfigureEffect"));
  message << QStringLiteral("autoscroll");
  QDBusConnection::sessionBus().asyncCall(message);
}

} // namespace AutoScroll

#include "autoscroll_config.moc"
