/*
 * Copyright (c) 2016 Fei Jie <feij.fnst@cn.fujitsu.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getrusage

# include <stdio.h>
# include <stdint.h>
# include <sys/resource.h>
# include <unistd.h>

int
main(void)
{
	struct rusage *const usage = tail_alloc(sizeof(struct rusage));
	int rc = syscall(__NR_getrusage, RUSAGE_SELF, usage);
	printf("getrusage(RUSAGE_SELF, {ru_utime={tv_sec=%jd, tv_usec=%jd}"
	       ", ru_stime={tv_sec=%jd, tv_usec=%jd}, ru_maxrss=%lu"
	       ", ru_ixrss=%lu, ru_idrss=%lu, ru_isrss=%lu, ru_minflt=%lu"
	       ", ru_majflt=%lu, ru_nswap=%lu, ru_inblock=%lu"
	       ", ru_oublock=%lu, ru_msgsnd=%lu, ru_msgrcv=%lu"
	       ", ru_nsignals=%lu, ru_nvcsw=%lu, ru_nivcsw=%lu}) = %d\n",
	       (intmax_t) usage->ru_utime.tv_sec,
	       (intmax_t) usage->ru_utime.tv_usec,
	       (intmax_t) usage->ru_stime.tv_sec,
	       (intmax_t) usage->ru_stime.tv_usec,
	       usage->ru_maxrss, usage->ru_ixrss, usage->ru_idrss,
	       usage->ru_isrss, usage->ru_minflt, usage->ru_majflt,
	       usage->ru_nswap, usage->ru_inblock, usage->ru_oublock,
	       usage->ru_msgsnd, usage->ru_msgrcv, usage->ru_nsignals,
	       usage->ru_nvcsw, usage->ru_nivcsw, rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getrusage")

#endif
