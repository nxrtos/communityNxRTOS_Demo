/*
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019 . or its affiliates.   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ARCH_BASETYPE  for  GCC/ARM_CM4F/
 */


#ifndef     ARCH_BASETYPE_H
#define    ARCH_BASETYPE_H

#include "stdint.h"
#include "stddef.h"

/// {{{  typedefs from arch4rtos for  GCC/ARM_CM4F/ {{{

typedef  int32_t   BaseType_t;
typedef  uint32_t  UBaseType_t;

typedef  uint32_t  AddressOpType_t;
typedef  uint32_t  StackType_t;
// refer  StackTypeEnum_t for special definition usage

typedef  uint32_t	TickType_t;
typedef  uint32_t	IrqPriorityBase_t;
typedef  int32_t	SysCriticalLevel_t;
// see  LOWEST_SYSCRITICALLEVEL  defined in
typedef  int32_t    ThreadPriorityType_t;

#ifndef  bool
typedef     uint8_t     bool;
#endif

#ifndef  __u8
typedef     uint8_t     __u8;
#endif

#ifndef	__u16
typedef     uint16_t    __u16;
#endif

#ifndef	__u32
typedef     uint32_t	__u32;
#endif

#ifndef  __u64
typedef     uint64_t    __u64;
#endif

/// }}}  typedefs from arch4rtos for  GCC/ARM_CM4F/ }}}

#endif /* ARCH_BASETYPE_H */

