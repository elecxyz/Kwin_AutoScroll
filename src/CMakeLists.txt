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

add_library(autoscroll_visuals STATIC
    glyphstyle.cpp
    glyphstyle.h
)
set_target_properties(autoscroll_visuals PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(autoscroll_visuals PUBLIC Qt6::Core Qt6::Gui Qt6::Svg)
target_include_directories(autoscroll_visuals PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

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
set_property(TARGET autoscroll APPEND PROPERTY AUTOGEN_TARGET_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json"
)

target_compile_definitions(autoscroll PRIVATE
    -DTRANSLATION_DOMAIN=\"kwin_autoscroll\"
)

if(KWin_VERSION VERSION_LESS "6.7.0")
    target_compile_definitions(autoscroll PRIVATE
        AUTOSCROLL_USE_LEGACY_IMAGE_ITEM_FACTORY
    )
    message(STATUS
        "KWin ${KWin_VERSION}: creating image items through the scene renderer")
else()
    message(STATUS
        "KWin ${KWin_VERSION}: creating unified image items directly")
endif()

target_link_libraries(autoscroll PRIVATE
    autoscroll_engine
    autoscroll_visuals
    KWin::kwin
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
)
