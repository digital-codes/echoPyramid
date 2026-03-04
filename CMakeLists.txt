idf_component_register(
    SRCS
        "src/I2CBus.cpp"
        "src/SI5351.cpp"
        "src/ES7210.cpp"
        "src/ES8311.cpp"
        "src/AW87559.cpp"
        "src/STM32Ctrl.cpp"
        "src/M5EchoPyramid.cpp"
    INCLUDE_DIRS "src"
    REQUIRES
        driver
        esp_driver_i2c
        esp_driver_i2s
)