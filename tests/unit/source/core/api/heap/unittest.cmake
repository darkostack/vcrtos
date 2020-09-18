set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../source/utils/heap.cpp
    ../../source/core/api/heap_api.cpp
    ../../source/core/assert_failure.c
)

set(unittest-test-sources
    source/core/api/heap/test_heap_api.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVCRTOS_PROJECT_CONFIG_FILE='\"vcrtos-unittest-config.h\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVCRTOS_PROJECT_CONFIG_FILE='\"vcrtos-unittest-config.h\"'")
