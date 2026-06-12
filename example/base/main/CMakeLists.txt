idf_component_register(SRCS "main.c"
                       PRIV_REQUIRES spi_flash
                       REQUIRES nvs_flash esp_lcd driver esp_timer yari)
