add_library(mp92plus INTERFACE)

target_include_directories(mp92plus
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(mp92plus
    INTERFACE
    pico_stdlib
    hardware_spi
    hardware_i2c
)

target_sources(mp92plus
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/mp92plus.c
    ${CMAKE_CURRENT_LIST_DIR}/include/mp92plus.h
    ${CMAKE_CURRENT_LIST_DIR}/include/regmap.h
)
