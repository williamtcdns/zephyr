# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(xaf)

target_sources(app PRIVATE src/main.c)

zephyr_get_include_directories_for_lang_as_string(       C includes)
zephyr_get_system_include_directories_for_lang_as_string(C system_includes)
zephyr_get_compile_definitions_for_lang_as_string(       C definitions)
zephyr_get_compile_options_for_lang_as_string(           C options)

set(external_project_cflags
  "${includes} ${definitions} ${options} ${system_includes} -std=gnu99"
)

include(ExternalProject)

set(xaf_build_dir ${CMAKE_CURRENT_BINARY_DIR}/xaf)

set(xaf_lib_obj ${xaf_build_dir}/test/build/xa_af_gain_rend_test_core0)

ExternalProject_Add(
  xaf_project
  DEPENDS zephyr_interface
  PREFIX     ${xaf_build_dir}
  SOURCE_DIR ${xaf_build_dir}
  CONFIGURE_COMMAND git clone https://github.com/williamtcdns/multicore-xaf.git ${xaf_build_dir}/repo
  COMMAND           git -C ${xaf_build_dir}/repo checkout main
  COMMAND           cp -al ${xaf_build_dir}/repo/xa_af_hostless/. ${xaf_build_dir}/
  COMMAND           rm -rf ${xaf_build_dir}/repo
  BUILD_COMMAND xt-make -C build/ clean all install XA_RTOS=zephyr ZEPHYR_CFLAGS=${external_project_cflags}
  COMMAND       xt-make -C test/build/ -f makefile_testbench_sample clean af_gain_renderer XA_RTOS=zephyr ZEPHYR_CFLAGS=${external_project_cflags}
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS ${xaf_lib_obj}
  BUILD_IN_SOURCE 1
  LOG_CONFIGURE 1
  LOG_INSTALL 1
  LOG_BUILD 1
  LOG_MERGED_STDOUTERR 1
  LOG_OUTPUT_ON_FAILURE 1
)

add_library(xaf_lib STATIC IMPORTED GLOBAL)
add_dependencies(
  xaf_lib
  xaf_project
)
set_target_properties(xaf_lib PROPERTIES IMPORTED_LOCATION ${xaf_lib_obj})

target_link_libraries(app PUBLIC xaf_lib -lc -lsim ${CMAKE_CURRENT_BINARY_DIR}/zephyr/kernel/libkernel.a)
