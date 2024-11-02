# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(include)

zephyr_library_sources_ifdef(CONFIG_SMP smp.c)
zephyr_library_sources_ifdef(CONFIG_SMP mpu_table.c)

if(CONFIG_SMP)
execute_process(
	COMMAND sh "-c" "xt-clang -dM -E -x c -imacros ../arch/include/xtensa/config/core-isa.h /dev/null | fgrep XCHAL_SUBSYS_NUM_CORES | cut -d ' ' -f 3"
	OUTPUT_VARIABLE CONFIG_MP_MAX_NUM_CPUS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND sed "-i" "/ CONFIG_MP_MAX_NUM_CPUS /c\#define CONFIG_MP_MAX_NUM_CPUS ${CONFIG_MP_MAX_NUM_CPUS}" "${PROJECT_BINARY_DIR}/include/generated/zephyr/autoconf.h"
)
endif()

set(SOC_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/include/dsp.ld CACHE INTERNAL "")
