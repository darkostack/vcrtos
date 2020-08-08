#include <vcrtos/instance.h>

#include "core/instance.hpp"

using namespace vc;

#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

instance_t *instance_init(void *buffer, size_t *size)
{
    Instance *instance;
    instance = Instance::init(buffer, size);
    return instance;
}

#else

instance_t *instance_init_single(void)
{
    return &Instance::init_single();
}


instance_t *instance_get(void)
{
    return &Instance::get();
}

#endif // #if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

bool instance_is_initialized(instance_t *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    return instances.is_initialized();
}

const char *instance_get_version_string(void)
{
    static const char _version[] = VCRTOS_CONFIG_PACKAGE_NAME "-" VCRTOS_CONFIG_PACKAGE_VERSION
#if defined(__DATE__)
                                 "; " __DATE__ " " __TIME__
#endif
        ; // Trailing semicolon to end statement.
    return _version;
}
