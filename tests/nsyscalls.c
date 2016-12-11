/*
 * Check decoding of out-of-range syscalls.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include "kernel_types.h"
#include "syscall.h"

#define TD 0
#define TF 0
#define TI 0
#define TN 0
#define TP 0
#define TS 0
#define TM 0
#define NF 0
#define MA 0
#define SI 0
#define SE 0
#define SEN(arg) 0,0

static const struct_sysent syscallent[] = {
#include "syscallent.h"
};

#include <asm/unistd.h>

#if defined __X32_SYSCALL_BIT && defined __NR_read \
 && (__X32_SYSCALL_BIT & __NR_read) != 0
# define SYSCALL_BIT __X32_SYSCALL_BIT
#else
# define SYSCALL_BIT 0
#endif

static void
test_syscall(const unsigned long nr)
{
	static const kernel_ulong_t a[] = {
		(kernel_ulong_t) 0xface0fedbadc0dedULL,
		(kernel_ulong_t) 0xface1fedbadc1dedULL,
		(kernel_ulong_t) 0xface2fedbadc2dedULL,
		(kernel_ulong_t) 0xface3fedbadc3dedULL,
		(kernel_ulong_t) 0xface4fedbadc4dedULL,
		(kernel_ulong_t) 0xface5fedbadc5dedULL
	};

	long rc = syscall(nr | SYSCALL_BIT,
			  a[0], a[1], a[2], a[3], a[4], a[5]);
#ifdef LINUX_MIPSO32
	printf("syscall(%#lx, %#lx, %#lx, %#lx, %#lx, %#lx, %#lx)"
	       " = %ld ENOSYS (%m)\n", nr | SYSCALL_BIT,
	       a[0], a[1], a[2], a[3], a[4], a[5], rc);
#else
	printf("syscall_%lu(%#llx, %#llx, %#llx, %#llx, %#llx, %#llx)"
	       " = %ld (errno %d)\n", nr,
	       (unsigned long long) a[0],
	       (unsigned long long) a[1],
	       (unsigned long long) a[2],
	       (unsigned long long) a[3],
	       (unsigned long long) a[4],
	       (unsigned long long) a[5],
	       rc, errno);
#endif
}

int
main(void)
{
	test_syscall(ARRAY_SIZE(syscallent));

#ifdef SYS_socket_subcall
	test_syscall(SYS_socket_subcall + 1);
#endif

#ifdef SYS_ipc_subcall
	test_syscall(SYS_ipc_subcall + 1);
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
