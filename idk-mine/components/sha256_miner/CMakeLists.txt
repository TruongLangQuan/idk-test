idf_component_register(
    SRCS "sha256_engine.c" "miner_core.c"
    INCLUDE_DIRS "include"
    REQUIRES lwip mbedtls utils stratum_client
)
