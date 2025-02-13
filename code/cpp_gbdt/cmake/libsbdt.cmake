include_guard(GLOBAL)


################################################################
# TARGET: libsbdt (make fast_shared)
################################################################

set(TARGET sbdt)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    ${SOURCE_DIR}/**.cpp
)

file(GLOB_RECURSE PUBLIC_HEADER_FILES CONFIGURE_DEPENDS
    ${INCLUDE_DIR}/**.h
    ${INCLUDE_DIR}/**.hpp
)

add_library(${TARGET} SHARED ${SOURCE_FILES})

target_include_directories(${TARGET}
    PUBLIC ${INCLUDE_DIR}
    PUBLIC ${INCLUDE_DIR}/gbdt
)

set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADER_FILES}")


#################################################################
# LIBRARY LINKING
################################################################

find_library(LIB_SVML
    NAME libsvml.so libsvml.a
    HINTS /usr/local/lib /usr/lib
)

find_library(LIB_FMT
    NAME libfmt.so libfmt.a
    HINTS /usr/local/lib /usr/lib
)

target_link_libraries(${TARGET}
    PUBLIC ${LIB_SVML}
    PUBLIC ${LIB_FMT}
)


################################################################
# libsbdt BUILD FLAGS
################################################################

target_compile_options(${TARGET} PRIVATE
    ${COMMON_COMPILE_OPTIONS}
    -c
    -Wall
    -Wextra
    -std=gnu++11
    -pipe
    -Ofast
    -ffast-math
    -march=native
    -mveclibabi=svml
    -pthread
    -flto
    -fPIC
)

target_link_options(${TARGET} PRIVATE
    -Wl,--no-as-needed
    -pthread
)

target_compile_definitions(${TARGET} PRIVATE
    DDELTA=5e-8
    DMAX_ALPHA=1000
)


################################################################
# libsbdt BUILD FLAGS
################################################################

install(TARGETS ${TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sbdt)
