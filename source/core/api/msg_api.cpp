#include <vcrtos/msg.h>

#include "core/instance.hpp"
#include "core/msg.hpp"

using namespace vc;

void msg_init(instance_t *instance, msg_t *msg)
{
    Instance &instances = *static_cast<Instance *>(instance);
    Msg &m = *static_cast<Msg *>(msg);
    m.init(instances);
}

int msg_receive(msg_t *msg)
{
    Msg &m = *static_cast<Msg *>(msg);
    return m.receive();
}

int msg_send(msg_t *msg, kernel_pid_t pid)
{
    Msg &m = *static_cast<Msg *>(msg);
    return m.send(pid);
}

int msg_try_send(msg_t *msg, kernel_pid_t pid)
{
    Msg &m = *static_cast<Msg *>(msg);
    return m.try_send(pid);
}

int msg_send_receive(msg_t *msg, msg_t *reply, kernel_pid_t pid)
{
    Msg &m = *static_cast<Msg *>(msg);
    return m.send_receive(static_cast<Msg *>(reply), pid);
}

int msg_reply(msg_t *msg, msg_t *reply)
{
    Msg &m = *static_cast<Msg *>(msg);
    return m.reply(static_cast<Msg *>(reply));
}

void msg_active_thread_queue_print(instance_t *instance)
{
    (void) instance;
    // TODO:
}
