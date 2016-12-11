/*
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

/*
 * Based on test by Dr. David Alan Gilbert <dave@treblig.org>
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

static fd_set set[0x1000000 / sizeof(fd_set)];

int main(void)
{
	int fds[2];
	struct {
		struct timeval tv;
		int pad[2];
	} tm_in = {
		.tv = { .tv_sec = 0xc0de1, .tv_usec = 0xc0de2 },
		.pad = { 0xdeadbeef, 0xbadc0ded }
	}, tm = tm_in;

	if (pipe(fds))
		perror_msg_and_fail("pipe");

	/*
	 * Start with a nice simple select.
	 */
	FD_ZERO(set);
	FD_SET(fds[0], set);
	FD_SET(fds[1], set);
	int rc = syscall(TEST_SYSCALL_NR, fds[1] + 1, set, set, set, NULL);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	assert(rc == 1);
	printf("%s(%d, [%d %d], [%d %d], [%d %d], NULL) = 1 ()\n",
	       TEST_SYSCALL_STR, fds[1] + 1, fds[0], fds[1],
	       fds[0], fds[1], fds[0], fds[1]);

	/*
	 * Another simple one, with a timeout.
	 */
	FD_SET(1, set);
	FD_SET(2, set);
	FD_SET(fds[0], set);
	FD_SET(fds[1], set);
	assert(syscall(TEST_SYSCALL_NR, fds[1] + 1, NULL, set, NULL, &tm.tv) == 3);
	printf("%s(%d, NULL, [1 2 %d %d], NULL, {tv_sec=%lld, tv_usec=%lld})"
	       " = 3 (out [1 2 %d], left {tv_sec=%lld, tv_usec=%lld})\n",
	       TEST_SYSCALL_STR, fds[1] + 1, fds[0], fds[1],
	       (long long) tm_in.tv.tv_sec, (long long) tm_in.tv.tv_usec,
	       fds[1],
	       (long long) tm.tv.tv_sec, (long long) tm.tv.tv_usec);

	/*
	 * Now the crash case that trinity found, negative nfds
	 * but with a pointer to a large chunk of valid memory.
	 */
	FD_ZERO(set);
	FD_SET(fds[1],set);
	assert(syscall(TEST_SYSCALL_NR, -1, NULL, set, NULL, NULL) == -1);
	printf("%s(-1, NULL, %p, NULL, NULL) = -1 EINVAL (%m)\n",
	       TEST_SYSCALL_STR, set);

	/*
	 * Another variant, with nfds exceeding FD_SETSIZE limit.
	 */
	FD_ZERO(set);
	FD_SET(fds[0],set);
	tm.tv.tv_sec = 0;
	tm.tv.tv_usec = 123;
	assert(syscall(TEST_SYSCALL_NR, FD_SETSIZE + 1, set, set + 1, NULL, &tm.tv) == 0);
	printf("%s(%d, [%d], [], NULL, {tv_sec=0, tv_usec=123}) = 0 (Timeout)\n",
	       TEST_SYSCALL_STR, FD_SETSIZE + 1, fds[0]);

	puts("+++ exited with 0 +++");
	return 0;
}
