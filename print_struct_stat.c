/*
 * Copyright (c) 1999-2003 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 David S. Miller <davem@nuts.davemloft.net>
 * Copyright (c) 2003-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007 Jan Kratochvil <jan.kratochvil@redhat.com>
 * Copyright (c) 2009 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2009-2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include "stat.h"

void
print_struct_stat(struct tcb *tcp, const struct strace_stat *const st)
{
	tprints("{");
	if (!abbrev(tcp)) {
		tprintf("st_dev=makedev(%u, %u), st_ino=%llu, st_mode=",
			(unsigned int) major(st->dev),
			(unsigned int) minor(st->dev),
			st->ino);
		print_symbolic_mode_t(st->mode);
		tprintf(", st_nlink=%llu, st_uid=%llu, st_gid=%llu",
			st->nlink, st->uid, st->gid);
		tprintf(", st_blksize=%llu", st->blksize);
		tprintf(", st_blocks=%llu", st->blocks);
	} else {
		tprints("st_mode=");
		print_symbolic_mode_t(st->mode);
	}

	switch (st->mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		tprintf(", st_rdev=makedev(%u, %u)",
			(unsigned int) major(st->rdev),
			(unsigned int) minor(st->rdev));
		break;
	default:
		tprintf(", st_size=%llu", st->size);
		break;
	}

	if (!abbrev(tcp)) {
		tprints(", st_atime=");
		tprints(sprinttime(st->atime));
		if (st->atime_nsec)
			tprintf(".%09llu", st->atime_nsec);
		tprints(", st_mtime=");
		tprints(sprinttime(st->mtime));
		if (st->mtime_nsec)
			tprintf(".%09llu", st->mtime_nsec);
		tprints(", st_ctime=");
		tprints(sprinttime(st->ctime));
		if (st->ctime_nsec)
			tprintf(".%09llu", st->ctime_nsec);
	} else {
		tprints(", ...");
	}
	tprints("}");
}
