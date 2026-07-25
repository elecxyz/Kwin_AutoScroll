// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrolleffect.h"

#include <effect/effect.h>

namespace AutoScroll {

class AutoScrollEffectFactory final : public KWin::EffectPluginFactory {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID EffectPluginFactory_iid FILE "metadata.json")
  Q_INTERFACES(KPluginFactory)

public:
  KWin::Effect *createEffect() const override { return new AutoScrollEffect(); }
};

} // namespace AutoScroll

#include "main.moc"
