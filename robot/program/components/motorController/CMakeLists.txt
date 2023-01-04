idf_component_register(SRCS "motorController.cpp"
                    INCLUDE_DIRS "." 
                    PRIV_REQUIRES
                    REQUIRES PIDController)
