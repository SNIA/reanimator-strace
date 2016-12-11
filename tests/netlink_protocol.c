/*
 * Check decoding of netlink protocol.
 *
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
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

#ifdef HAVE_SYS_XATTR_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/xattr.h>
# include <netinet/in.h>
# include <linux/netlink.h>
# include <linux/sock_diag.h>
# include <linux/netlink_diag.h>

# if !defined NETLINK_SOCK_DIAG && defined NETLINK_INET_DIAG
#  define NETLINK_SOCK_DIAG NETLINK_INET_DIAG
# endif

static void
send_query(const int fd)
{
	static const struct req {
		struct nlmsghdr nlh;
		const char magic[4];
	} c_req = {
		.nlh = {
			.nlmsg_len = sizeof(struct req),
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.magic = "abcd"
	};
	struct req *const req = tail_memdup(&c_req, sizeof(c_req));
	long rc;
	const char *errstr;

	/* zero address */
	rc = sendto(fd, NULL, sizeof(*req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, NULL, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, (unsigned) sizeof(*req), sprintrc(rc));

	/* zero length */
	rc = sendto(fd, req, 0, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, \"\", 0, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, sprintrc(rc));

	/* zero address and length */
	rc = sendto(fd, NULL, 0, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, NULL, 0, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, sprintrc(rc));

	/* unfetchable struct nlmsghdr */
	const void *const efault = tail_alloc(sizeof(struct nlmsghdr) - 1);
	rc = sendto(fd, efault, sizeof(struct nlmsghdr), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, %p, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, efault, (unsigned) sizeof(struct nlmsghdr), sprintrc(rc));

	/* whole message length < sizeof(struct nlmsghdr) */
	rc = sendto(fd, req->magic, sizeof(req->magic), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, \"abcd\", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, (unsigned) sizeof(req->magic), sprintrc(rc));

	/* a single message with some data */
	rc = sendto(fd, req, sizeof(*req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
	       ", seq=0, pid=0}, \"abcd\"}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req->nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*req), sprintrc(rc));

	/* a single message without data */
	req->nlh.nlmsg_len = sizeof(req->nlh);
	rc = sendto(fd, &req->nlh, sizeof(req->nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
	       ", seq=0, pid=0}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req->nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(req->nlh), sprintrc(rc));

	/* nlmsg_len > whole message length */
	req->nlh.nlmsg_len = sizeof(*req) + 8;
	rc = sendto(fd, req, sizeof(*req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
	       ", seq=0, pid=0}, \"abcd\"}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req->nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*req), sprintrc(rc));

	/* nlmsg_len < sizeof(struct nlmsghdr) */
	req->nlh.nlmsg_len = 8;
	rc = sendto(fd, req, sizeof(*req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
	       ", seq=0, pid=0}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req->nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*req), sprintrc(rc));

	/* a sequence of two nlmsg objects */
	struct reqs {
		struct req req1;
		char padding[NLMSG_ALIGN(sizeof(struct req)) - sizeof(struct req)];
		struct req req2;
	} *const reqs = tail_alloc(sizeof(*reqs));
	memcpy(&reqs->req1, &c_req, sizeof(c_req));
	memcpy(&reqs->req2, &c_req, sizeof(c_req));

	rc = sendto(fd, reqs, sizeof(*reqs), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
	       ", seq=0, pid=0}, \"abcd\"}, {{len=%u, type=NLMSG_NOOP"
	       ", flags=NLM_F_REQUEST|0x%x, seq=0, pid=0}, \"abcd\"}]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, reqs->req1.nlh.nlmsg_len, NLM_F_DUMP,
	       reqs->req2.nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*reqs), sprintrc(rc));

	/* unfetchable second struct nlmsghdr */
	void *const efault2 = tail_memdup(&reqs->req1, sizeof(reqs->req1));
	rc = sendto(fd, efault2, sizeof(*reqs), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
	       ", seq=0, pid=0}, \"abcd\"}, %p], %u, MSG_DONTWAIT, NULL, 0)"
	       " = %s\n",
	       fd, reqs->req1.nlh.nlmsg_len, NLM_F_DUMP,
	       &((struct reqs *) efault2)->req2, (unsigned) sizeof(*reqs),
	       sprintrc(rc));

	/* message length is not enough for the second struct nlmsghdr */
	rc = sendto(fd, reqs, sizeof(*reqs) - sizeof(req->nlh), MSG_DONTWAIT,
		    NULL, 0);
	errstr = sprintrc(rc);
	printf("sendto(%d, [{{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
	       ", seq=0, pid=0}, \"abcd\"}, \"",
	       fd, reqs->req1.nlh.nlmsg_len, NLM_F_DUMP);
	print_quoted_memory((void *) &reqs->req2.nlh,
			    sizeof(reqs->req2) - sizeof(req->nlh));
	printf("\"], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       (unsigned) (sizeof(*reqs) - sizeof(req->nlh)), errstr);

	/* second nlmsg_len < sizeof(struct nlmsghdr) */
	reqs->req2.nlh.nlmsg_len = 4;
	rc = sendto(fd, reqs, sizeof(*reqs), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
	       ", seq=0, pid=0}, \"abcd\"}, {{len=%u, type=NLMSG_NOOP"
	       ", flags=NLM_F_REQUEST|0x%x, seq=0, pid=0}}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, reqs->req1.nlh.nlmsg_len, NLM_F_DUMP,
	       reqs->req2.nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*reqs), sprintrc(rc));

	/* abbreviated output */
# define DEFAULT_STRLEN 32
# define ABBREV_LEN (DEFAULT_STRLEN + 1)
	const unsigned int msg_len = sizeof(struct nlmsghdr) * ABBREV_LEN;
	struct nlmsghdr *const msgs = tail_alloc(msg_len);
	unsigned int i;
	for (i = 0; i < ABBREV_LEN; ++i) {
		msgs[i].nlmsg_len = sizeof(*msgs);
		msgs[i].nlmsg_type = NLMSG_NOOP;
		msgs[i].nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
		msgs[i].nlmsg_seq = i;
		msgs[i].nlmsg_pid = 0;
	}

	rc = sendto(fd, msgs, msg_len, MSG_DONTWAIT, NULL, 0);
	errstr = sprintrc(rc);
	printf("sendto(%d, [", fd);
	for (i = 0; i < DEFAULT_STRLEN; ++i) {
		if (i)
			printf(", ");
		printf("{{len=%u, type=NLMSG_NOOP, flags=NLM_F_REQUEST|0x%x"
		       ", seq=%u, pid=0}}",
		       msgs[i].nlmsg_len, NLM_F_DUMP, msgs[i].nlmsg_seq);
	}
	printf(", ...], %u, MSG_DONTWAIT, NULL, 0) = %s\n", msg_len, errstr);
}

int main(void)
{
	struct sockaddr_nl addr;
	socklen_t len = sizeof(addr);
	int fd;

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;

	if ((fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG)) == -1)
		perror_msg_and_skip("socket AF_NETLINK");

	printf("socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG) = %d\n",
	       fd);
	if (bind(fd, (struct sockaddr *) &addr, len))
		perror_msg_and_skip("bind");
	printf("bind(%d, {sa_family=AF_NETLINK, nl_pid=0, nl_groups=00000000}"
	       ", %u) = 0\n", fd, len);

	char *path;
	if (asprintf(&path, "/proc/self/fd/%u", fd) < 0)
		perror_msg_and_fail("asprintf");
	char buf[256];
	if (getxattr(path, "system.sockprotoname", buf, sizeof(buf) - 1) < 0)
		perror_msg_and_skip("getxattr");
	free(path);

	send_query(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_XATTR_H")

#endif
