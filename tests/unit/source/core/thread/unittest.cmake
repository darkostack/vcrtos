set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../source/core/instance.cpp
    ../../source/core/thread.cpp
    ../../source/core/mutex.cpp
    stubs/assert_api_stub.c
    stubs/cpu_stub.c
    stubs/thread_stub.c
    stubs/thread_arch_stub.c
)

set(unittest-test-sources
    source/core/thread/test_thread.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPROJECT_CONFIG_FILE='\"vcrtos-unittest-config.h\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_CONFIG_FILE='\"vcrtos-unittest-config.h\"'")
