set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../source/core/instance.cpp
    ../../source/core/thread.cpp
    ../../source/core/mutex.cpp
    ../../source/core/assert_failure.c
    stubs/cpu_stub.c
    stubs/thread_stub.c
    stubs/thread_arch_stub.c
)

set(unittest-test-sources
    source/core/instance/test_instance.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVCRTOS_PROJECT_CONFIG_FILE='\"vcrtos-unittest-config.h\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVCRTOS_PROJECT_CONFIG_FILE='\"vcrtos-unittest-config.h\"'")
