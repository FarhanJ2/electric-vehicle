add_library(pico_mpu_9250
    mpu9250.c
    mpuObject.cpp
)

target_include_directories(pico_mpu_9250 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(pico_mpu_9250
    pico_stdlib
    hardware_spi
)
