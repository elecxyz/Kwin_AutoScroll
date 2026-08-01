# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

add_library(autoscroll_kcm_widgets STATIC
    glyphselectors.cpp
    glyphselectors.h
)
set_target_properties(autoscroll_kcm_widgets PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(autoscroll_kcm_widgets PRIVATE
    -DTRANSLATION_DOMAIN=\"kwin_autoscroll\"
)
target_include_directories(autoscroll_kcm_widgets PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(autoscroll_kcm_widgets PUBLIC
    autoscroll_visuals
    KF6::I18n
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

set(autoscroll_kcm_sources
    autoscroll_config.cpp
    autoscroll_config.h
    autoscroll_config.ui
    metadata.json
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/resources.qrc"
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
    autoscroll_kcm_widgets
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
