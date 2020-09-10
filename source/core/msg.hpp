/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

#ifndef CORE_MSG_HPP
#define CORE_MSG_HPP

#include <stdint.h>

#include <vcrtos/assert.h>
#include <vcrtos/config.h>
#include <vcrtos/kernel.h>
#include <vcrtos/msg.h>

#include "core/list.hpp"

namespace vc {

class Thread;
class Instance;
class MsgBusEntry;
class MsgBus;

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t instance_raw[];
#endif

class Msg : public msg_t
{
public:
    Msg(void) {}

    explicit Msg(Instance &instance)
    {
        init(instance);
    }

    void init(Instance &instances)
    {
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
        instance = static_cast<void *>(&instances);
#else
        (void)instances;
#endif
        sender_pid = KERNEL_PID_UNDEF;
        type = 0;
        content.ptr = NULL;
        content.value = 0;
    }

    int queued_msg(Thread *target);

    int send(kernel_pid_t target_pid);

    int try_send(kernel_pid_t target_pid);

    int send_to_self_queue(void);

    int send_in_isr(kernel_pid_t target_pid);

    int is_sent_by_isr(void) { return sender_pid == KERNEL_PID_ISR; }

    int receive(void) { return receive(1); }

    int try_receive(void) { return receive(0); }

    int send_receive(Msg *reply, kernel_pid_t target_pid);

    int reply(Msg *reply);

    int reply_in_isr(Msg *reply);

private:
    int send(kernel_pid_t target_pid, int blocking, unsigned state);

    int receive(int blocking);

    template <typename Type> inline Type &get(void) const;

#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &get_instance(void) const { return *static_cast<Instance *>(instance); }
#else
    Instance &get_instance(void) const { return *reinterpret_cast<Instance *>(&instance_raw); }
#endif
};

} // namespace vc

#endif /* CORE_MSG_HPP */
