# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

add_executable(scrollengine_test scrollengine_test.cpp)
target_link_libraries(scrollengine_test PRIVATE autoscroll_engine Qt6::Test)
add_test(NAME scrollengine_test COMMAND scrollengine_test)

add_executable(sessionstate_test sessionstate_test.cpp)
target_link_libraries(sessionstate_test PRIVATE autoscroll_engine Qt6::Test)
add_test(NAME sessionstate_test COMMAND sessionstate_test)

set(config_test_sources config_test.cpp)
kconfig_add_kcfg_files(config_test_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/../config/autoscrollconfig.kcfgc"
)
add_executable(config_test ${config_test_sources})
target_link_libraries(config_test PRIVATE KF6::ConfigGui Qt6::Test)
add_test(NAME config_test COMMAND config_test)

add_executable(glyphstyle_test
    glyphstyle_test.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/resources.qrc"
)
target_link_libraries(glyphstyle_test PRIVATE autoscroll_visuals Qt6::Test)
add_test(NAME glyphstyle_test COMMAND glyphstyle_test)

set(glyphselectors_test_sources
    glyphselectors_test.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../kcm/autoscroll_config.ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/resources.qrc"
)
kconfig_add_kcfg_files(glyphselectors_test_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/../config/autoscrollconfig.kcfgc"
)
add_executable(glyphselectors_test ${glyphselectors_test_sources})
set_target_properties(glyphselectors_test PROPERTIES
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../kcm"
)
target_include_directories(glyphselectors_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../kcm"
)
target_link_libraries(glyphselectors_test PRIVATE
    autoscroll_kcm_widgets
    KF6::ConfigGui
    KF6::ConfigWidgets
    Qt6::Test
    Qt6::Widgets
)
add_test(NAME glyphselectors_test COMMAND glyphselectors_test)

add_executable(pluginmetadata_test pluginmetadata_test.cpp)
target_link_libraries(pluginmetadata_test PRIVATE KWin::kwin Qt6::Core Qt6::Test)
target_compile_definitions(pluginmetadata_test PRIVATE
    AUTOSCROLL_EFFECT_PATH=\"$<TARGET_FILE:autoscroll>\"
    AUTOSCROLL_KCM_PATH=\"$<TARGET_FILE:kwin_autoscroll_config>\"
    AUTOSCROLL_PROJECT_VERSION=\"${PROJECT_VERSION}\"
)
add_dependencies(pluginmetadata_test autoscroll kwin_autoscroll_config)
add_test(NAME pluginmetadata_test COMMAND pluginmetadata_test)

set_tests_properties(
    scrollengine_test
    sessionstate_test
    config_test
    glyphstyle_test
    glyphselectors_test
    pluginmetadata_test
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
