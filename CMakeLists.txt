cmake_minimum_required(VERSION 2.8)
project(dpi)

set(CMAKE_C_FLAGS "-fPIC -ggdb -Wall")
set(CMAKE_C_FLAGS_RELEASE "-Os")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wstrict-prototypes -Wall -Wundef -Wno-long-long -Wsign-compare -Wcomment -pedantic -finline-functions -fno-strict-aliasing -g")

set(MODULE_SRC dpi_module.c)
set(MODULE_KO lkm/dpi_module.ko)

set(LIBRARY_INCLUDE library/include)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lkm)

add_custom_command(OUTPUT ${MODULE_KO}
                   COMMAND bash ../prepare_kbuild.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
                   COMMAND make W=1 -C /lib/modules/`uname -r`/build M=`pwd`/lkm
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(dpi_module
                  DEPENDS ${MODULE_KO}
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

execute_process(COMMAND uname -r OUTPUT_VARIABLE KERNEL OUTPUT_STRIP_TRAILING_WHITESPACE)

add_library(${PROJECT_NAME} SHARED library/src/dpi.c)
add_library(${PROJECT_NAME}_static STATIC library/src/dpi.c)

include_directories(${PROJECT_NAME} ${LIBRARY_INCLUDE})
include_directories(${PROJECT_NAME}_static ${LIBRARY_INCLUDE})

add_dependencies(${PROJECT_NAME} dpi_module)

install(FILES ${LIBRARY_INCLUDE}/dpi.h DESTINATION include)
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(CODE "execute_process(COMMAND make W=1 -C /lib/modules/${KERNEL}/build M=${CMAKE_BINARY_DIR}/lkm modules_install)")
install(CODE "execute_process(COMMAND depmod)")

