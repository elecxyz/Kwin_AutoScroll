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

add_executable(pluginmetadata_test pluginmetadata_test.cpp)
target_link_libraries(pluginmetadata_test PRIVATE KWin::kwin Qt6::Core Qt6::Test)
target_compile_definitions(pluginmetadata_test PRIVATE
    AUTOSCROLL_EFFECT_PATH=\"$<TARGET_FILE:autoscroll>\"
    AUTOSCROLL_KCM_PATH=\"$<TARGET_FILE:kwin_autoscroll_config>\"
)
add_dependencies(pluginmetadata_test autoscroll kwin_autoscroll_config)
add_test(NAME pluginmetadata_test COMMAND pluginmetadata_test)
