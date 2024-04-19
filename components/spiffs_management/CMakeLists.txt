idf_component_register(SRCS "spiffs_management.c"
                       INCLUDE_DIRS "." 
                       REQUIRES spiffs file_system json
                    )