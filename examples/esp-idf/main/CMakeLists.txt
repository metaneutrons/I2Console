idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    # i2console for the component API, esp_driver_i2c because this file
    # creates the bus itself.
    REQUIRES i2console esp_driver_i2c
)
