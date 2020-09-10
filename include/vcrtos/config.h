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

#ifndef VCRTOS_CONFIG_H
#define VCRTOS_CONFIG_H

#define VCRTOS_CONFIG_H_IN

#ifdef VCRTOS_PROJECT_CONFIG_FILE
#include VCRTOS_PROJECT_CONFIG_FILE
#endif

#include <vcrtos/default-config.h>

#undef VCRTOS_CONFIG_H_IN

#endif // VCRTOS_CONFIG_H
