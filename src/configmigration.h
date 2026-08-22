// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KConfigGroup>
#include <KSharedConfig>

namespace AutoScroll {

inline void migrateActivationBehavior(const KSharedConfig::Ptr &config) {
  KConfigGroup group(config, QStringLiteral("Effect-autoscroll"));
  const QString legacyKey = QStringLiteral("HoldToScroll");
  if (group.hasKey(QStringLiteral("ActivationBehavior"))) {
    if (group.hasKey(legacyKey)) {
      group.deleteEntry(legacyKey);
      config->sync();
    }
    return;
  }
  if (!group.hasKey(legacyKey)) {
    return;
  }

  constexpr int ToggleBehavior = 0;
  constexpr int HoldBehavior = 1;
  const bool legacyHoldToScroll = group.readEntry(legacyKey, false);
  group.writeEntry(QStringLiteral("ActivationBehavior"),
                   legacyHoldToScroll ? HoldBehavior : ToggleBehavior);
  group.deleteEntry(legacyKey);
  config->sync();
}

} // namespace AutoScroll
