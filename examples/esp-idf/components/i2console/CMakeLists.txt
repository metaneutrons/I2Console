idf_component_register(
    SRCS "i2console.c"
    INCLUDE_DIRS "include"
    # esp_driver_i2c is PUBLIC because include/i2console.h exposes
    # i2c_master_bus_handle_t in its own interface.
    REQUIRES esp_driver_i2c
)
