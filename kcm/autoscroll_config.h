// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KCModule>

#include "ui_autoscroll_config.h"

#include <memory>

namespace AutoScroll {

class AutoScrollConfig;

class AutoScrollEffectConfig : public KCModule {
  Q_OBJECT

public:
  explicit AutoScrollEffectConfig(QObject *parent, const KPluginMetaData &data);
  ~AutoScrollEffectConfig() override;
  void save() override;

private:
  Ui::AutoScrollConfigForm m_ui;
  std::unique_ptr<AutoScrollConfig> m_config;
};

} // namespace AutoScroll
