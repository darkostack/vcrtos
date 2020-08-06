set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../source/core/assert_failure.c
)

set(unittest-test-sources
    source/core/cib/test_cib.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPROJECT_CONFIG_FILE='\"vcrtos-unittest-config.h\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_CONFIG_FILE='\"vcrtos-unittest-config.h\"'")
