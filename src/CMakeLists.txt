# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set(autoscroll_engine_sources
    scrollengine.cpp
    scrollengine.h
    sessionstate.cpp
    sessionstate.h
)

add_library(autoscroll_engine STATIC ${autoscroll_engine_sources})
set_target_properties(autoscroll_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(autoscroll_engine PUBLIC Qt6::Core Qt6::Gui)
target_include_directories(autoscroll_engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set(autoscroll_effect_sources
    autoscrolleffect.cpp
    autoscrolleffect.h
    autoscrollinputdevice.cpp
    autoscrollinputdevice.h
    autoscrollvisual.cpp
    autoscrollvisual.h
    main.cpp
    resources.qrc
)

kconfig_add_kcfg_files(autoscroll_effect_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/../config/autoscrollconfig.kcfgc"
)

kcoreaddons_add_plugin(autoscroll
    SOURCES ${autoscroll_effect_sources}
    INSTALL_NAMESPACE "kwin/effects/plugins"
)

target_compile_definitions(autoscroll PRIVATE
    -DTRANSLATION_DOMAIN=\"kwin_autoscroll\"
)

target_link_libraries(autoscroll PRIVATE
    autoscroll_engine
    KWin::kwin
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
)
