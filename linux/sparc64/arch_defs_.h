/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define HAVE_ARCH_GETRVAL2 1
#define HAVE_ARCH_UID16_SYSCALLS 1
#define HAVE_ARCH_SA_RESTORER 1
#define SUPPORTED_PERSONALITIES 2
#define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_SPARC64, 0 }
#define PERSONALITY1_AUDIT_ARCH { AUDIT_ARCH_SPARC,   0 }
#define HAVE_ARCH_DEDICATED_ERR_REG 1
