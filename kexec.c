/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include "xlat/kexec_load_flags.h"
#include "xlat/kexec_arch_values.h"

#ifndef KEXEC_ARCH_MASK
# define KEXEC_ARCH_MASK 0xffff0000
#endif
#ifndef KEXEC_SEGMENT_MAX
# define KEXEC_SEGMENT_MAX 16
#endif

static bool
print_seg(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const unsigned long *seg;
	unsigned long seg_buf[4];

        if (elem_size < sizeof(seg_buf)) {
		unsigned int i;

		for (i = 0; i < ARRAY_SIZE(seg_buf); ++i)
			seg_buf[i] = ((unsigned int *) elem_buf)[i];
		seg = seg_buf;
	} else {
		seg = elem_buf;
	}

	tprints("{buf=");
	printaddr(seg[0]);
	tprintf(", bufsz=%lu, mem=", seg[1]);
	printaddr(seg[2]);
	tprintf(", memsz=%lu}", seg[3]);

	return true;
}

static void
print_kexec_segments(struct tcb *tcp, const unsigned long addr,
		     const unsigned long len)
{
	if (len > KEXEC_SEGMENT_MAX) {
		printaddr(addr);
		return;
	}

	unsigned long seg[4];
	const size_t sizeof_seg = ARRAY_SIZE(seg) * current_wordsize;

	print_array(tcp, addr, len, seg, sizeof_seg,
		    umoven_or_printaddr, print_seg, 0);
}

SYS_FUNC(kexec_load)
{
	/* entry, nr_segments */
	printaddr(widen_to_ulong(tcp->u_arg[0]));
	tprintf(", %lu, ", widen_to_ulong(tcp->u_arg[1]));

	/* segments */
	print_kexec_segments(tcp, widen_to_ulong(tcp->u_arg[2]),
			     widen_to_ulong(tcp->u_arg[1]));
	tprints(", ");

	/* flags */
	unsigned long n = widen_to_ulong(tcp->u_arg[3]);
	printxval_long(kexec_arch_values, n & KEXEC_ARCH_MASK, "KEXEC_ARCH_???");
	n &= ~(unsigned long) KEXEC_ARCH_MASK;
	if (n) {
		tprints("|");
		printflags_long(kexec_load_flags, n, "KEXEC_???");
	}

	return RVAL_DECODED;
}

#include "xlat/kexec_file_load_flags.h"

SYS_FUNC(kexec_file_load)
{
	/* kernel_fd */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* initrd_fd */
	printfd(tcp, tcp->u_arg[1]);
	tprints(", ");
	/* cmdline_len */
	tprintf("%lu, ", tcp->u_arg[2]);
	/* cmdline */
	printstr(tcp, tcp->u_arg[3], tcp->u_arg[2]);
	tprints(", ");
	/* flags */
	printflags_long(kexec_file_load_flags, tcp->u_arg[4], "KEXEC_FILE_???");

	return RVAL_DECODED;
}
