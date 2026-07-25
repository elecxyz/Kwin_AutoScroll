# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set(autoscroll_kcm_sources
    autoscroll_config.cpp
    autoscroll_config.h
    autoscroll_config.ui
)

kconfig_add_kcfg_files(autoscroll_kcm_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/../config/autoscrollconfig.kcfgc"
)

kcoreaddons_add_plugin(kwin_autoscroll_config
    SOURCES ${autoscroll_kcm_sources}
    INSTALL_NAMESPACE "kwin/effects/configs"
)

target_compile_definitions(kwin_autoscroll_config PRIVATE
    -DTRANSLATION_DOMAIN=\"kwin_autoscroll\"
)

target_link_libraries(kwin_autoscroll_config PRIVATE
    KF6::ConfigGui
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtils
    KF6::WidgetsAddons
    Qt6::Core
    Qt6::DBus
    Qt6::Widgets
)
