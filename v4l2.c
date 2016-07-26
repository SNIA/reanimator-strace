/*
 * Copyright (c) 2014 Philippe De Muyter <phdm@macqel.be>
 * Copyright (c) 2014 William Manley <will@williammanley.net>
 * Copyright (c) 2011 Peter Zotov <whitequark@whitequark.org>
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

#include DEF_MPERS_TYPE(struct_v4l2_buffer)
#include DEF_MPERS_TYPE(struct_v4l2_create_buffers)
#include DEF_MPERS_TYPE(struct_v4l2_ext_control)
#include DEF_MPERS_TYPE(struct_v4l2_ext_controls)
#include DEF_MPERS_TYPE(struct_v4l2_format)
#include DEF_MPERS_TYPE(struct_v4l2_framebuffer)
#include DEF_MPERS_TYPE(struct_v4l2_input)
#include DEF_MPERS_TYPE(struct_v4l2_standard)

#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>

typedef struct v4l2_buffer struct_v4l2_buffer;
typedef struct v4l2_create_buffers struct_v4l2_create_buffers;
typedef struct v4l2_ext_control struct_v4l2_ext_control;
typedef struct v4l2_ext_controls struct_v4l2_ext_controls;
typedef struct v4l2_format struct_v4l2_format;
typedef struct v4l2_framebuffer struct_v4l2_framebuffer;
typedef struct v4l2_input struct_v4l2_input;
typedef struct v4l2_standard struct_v4l2_standard;

#include MPERS_DEFS

/* some historical constants */
#ifndef V4L2_CID_HCENTER
#define V4L2_CID_HCENTER (V4L2_CID_BASE+22)
#endif
#ifndef V4L2_CID_VCENTER
#define V4L2_CID_VCENTER (V4L2_CID_BASE+23)
#endif
#ifndef V4L2_CID_BAND_STOP_FILTER
#define V4L2_CID_BAND_STOP_FILTER (V4L2_CID_BASE+33)
#endif

#define FMT_FRACT "%u/%u"
#define ARGS_FRACT(x) ((x).numerator), ((x).denominator)

#define FMT_RECT "{left=%d, top=%d, width=%u, height=%u}"
#define ARGS_RECT(x) (x).left, (x).top, (x).width, (x).height

static void
print_pixelformat(uint32_t fourcc)
{
	const union {
		uint32_t pixelformat;
		unsigned char cc[sizeof(uint32_t)];
	} u = { .pixelformat = htole32(fourcc) };
	unsigned int i;

	tprints("v4l2_fourcc(");
	for (i = 0; i < sizeof(u.cc); ++i) {
		unsigned char c = u.cc[i];

		if (i)
			tprints(", ");
		if (c == '\'' || c == '\\') {
			char sym[] = {
				'\'',
				'\\',
				c,
				'\'',
				'\0'
			};
			tprints(sym);
		} else if (c >= ' ' && c <= 0x7e) {
			char sym[] = {
				'\'',
				c,
				'\'',
				'\0'
			};
			tprints(sym);
		} else {
			char hex[] = {
				'\'',
				'\\',
				'x',
				"0123456789abcdef"[c >> 4],
				"0123456789abcdef"[c & 0xf],
				'\'',
				'\0'
			};
			tprints(hex);
		}
	}
	tprints(")");
}

#include "xlat/v4l2_device_capabilities_flags.h"

static int
print_v4l2_capability(struct tcb *tcp, const long arg)
{
	struct v4l2_capability caps;

	if (entering(tcp))
		return 0;
	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &caps))
		return 1;
	tprints("{driver=");
	print_quoted_string((const char *) caps.driver,
			    sizeof(caps.driver), QUOTE_0_TERMINATED);
	tprints(", card=");
	print_quoted_string((const char *) caps.card,
			    sizeof(caps.card), QUOTE_0_TERMINATED);
	tprints(", bus_info=");
	print_quoted_string((const char *) caps.bus_info,
			    sizeof(caps.bus_info), QUOTE_0_TERMINATED);
	tprintf(", version=%u.%u.%u, capabilities=",
		(caps.version >> 16) & 0xFF,
		(caps.version >> 8) & 0xFF,
		caps.version & 0xFF);
	printflags(v4l2_device_capabilities_flags, caps.capabilities,
		   "V4L2_CAP_???");
#ifdef V4L2_CAP_DEVICE_CAPS
	tprints(", device_caps=");
	printflags(v4l2_device_capabilities_flags, caps.device_caps,
		   "V4L2_CAP_???");
#endif
	tprints("}");
	return 1;
}

#include "xlat/v4l2_buf_types.h"
#include "xlat/v4l2_format_description_flags.h"

static int
print_v4l2_fmtdesc(struct tcb *tcp, const long arg)
{
	struct v4l2_fmtdesc f;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_DECODED | 1;
		tprintf("{index=%u, type=", f.index);
		printxval(v4l2_buf_types, f.type, "V4L2_BUF_TYPE_???");
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &f)) {
		tprints(", flags=");
		printflags(v4l2_format_description_flags, f.flags,
			   "V4L2_FMT_FLAG_???");
		tprints(", description=");
		print_quoted_string((const char *) f.description,
				    sizeof(f.description),
				    QUOTE_0_TERMINATED);
		tprints(", pixelformat=");
		print_pixelformat(f.pixelformat);
	}
	tprints("}");
	return 1;
}

#include "xlat/v4l2_fields.h"
#include "xlat/v4l2_colorspaces.h"

static void
print_v4l2_format_fmt(const char *prefix, const struct_v4l2_format *f)
{
	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		tprints(prefix);
		tprintf("fmt.pix={width=%u, height=%u, pixelformat=",
			f->fmt.pix.width, f->fmt.pix.height);
		print_pixelformat(f->fmt.pix.pixelformat);
		tprints(", field=");
		printxval(v4l2_fields, f->fmt.pix.field, "V4L2_FIELD_???");
		tprintf(", bytesperline=%u, sizeimage=%u, colorspace=",
			f->fmt.pix.bytesperline, f->fmt.pix.sizeimage);
		printxval(v4l2_colorspaces, f->fmt.pix.colorspace,
			  "V4L2_COLORSPACE_???");
		tprints("}");
		break;
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: {
		unsigned int i, max;

		tprints(prefix);
		tprintf("fmt.pix_mp={width=%u, height=%u, pixelformat=",
			f->fmt.pix_mp.width, f->fmt.pix_mp.height);
		print_pixelformat(f->fmt.pix_mp.pixelformat);
		tprints(", field=");
		printxval(v4l2_fields, f->fmt.pix_mp.field, "V4L2_FIELD_???");
		tprints(", colorspace=");
		printxval(v4l2_colorspaces, f->fmt.pix_mp.colorspace,
			  "V4L2_COLORSPACE_???");
		tprints(", plane_fmt=[");
		max = f->fmt.pix_mp.num_planes;
		if (max > VIDEO_MAX_PLANES)
			max = VIDEO_MAX_PLANES;
		for (i = 0; i < max; i++) {
			if (i > 0)
				tprints(", ");
			tprintf("{sizeimage=%u, bytesperline=%u}",
				f->fmt.pix_mp.plane_fmt[i].sizeimage,
				f->fmt.pix_mp.plane_fmt[i].bytesperline);
		}
		tprintf("], num_planes=%u}", (unsigned) f->fmt.pix_mp.num_planes);
		break;
	}
#endif

	/* TODO: Complete this switch statement */
#if 0
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
#endif
		tprints(prefix);
		tprints("fmt.win={???}");
		break;

	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		tprints(prefix);
		tprints("fmt.vbi={???}");
		break;

	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		tprints(prefix);
		tprints("fmt.sliced={???}");
		break;

#if HAVE_DECL_V4L2_BUF_TYPE_SDR_CAPTURE
	case V4L2_BUF_TYPE_SDR_CAPTURE:
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		tprints(prefix);
		tprints("fmt.sdr={???}");
		break;
#endif
#endif
	}
}

static int
print_v4l2_format(struct tcb *tcp, const long arg, const bool is_get)
{
	struct_v4l2_format f;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_DECODED | 1;
		tprints("{type=");
		printxval(v4l2_buf_types, f.type, "V4L2_BUF_TYPE_???");
		if (is_get)
			return 0;
		print_v4l2_format_fmt(", ", &f);
	} else {
		if (!syserror(tcp) && !umove(tcp, arg, &f)) {
			const char *delim = is_get ? ", " : " => ";
			print_v4l2_format_fmt(delim, &f);
		}
		tprints("}");
	}
	return 1;
}

#include "xlat/v4l2_memories.h"

static int
print_v4l2_requestbuffers(struct tcb *tcp, const long arg)
{
	struct v4l2_requestbuffers reqbufs;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &reqbufs))
			return RVAL_DECODED | 1;
		tprintf("{count=%u, type=", reqbufs.count);
		printxval(v4l2_buf_types, reqbufs.type, "V4L2_BUF_TYPE_???");
		tprints(", memory=");
		printxval(v4l2_memories, reqbufs.memory, "V4L2_MEMORY_???");
		tprints("}");
		return 0;
	} else {
		static char outstr[sizeof("{count=}") + sizeof(int) * 3];

		if (syserror(tcp) || umove(tcp, arg, &reqbufs) < 0)
			return 1;
		sprintf(outstr, "{count=%u}", reqbufs.count);
		tcp->auxstr = outstr;
		return 1 + RVAL_STR;
	}
}

#include "xlat/v4l2_buf_flags.h"

static int
print_v4l2_buffer(struct tcb *tcp, const unsigned int code, const long arg)
{
	struct_v4l2_buffer b;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &b))
			return RVAL_DECODED | 1;
		tprints("{type=");
		printxval(v4l2_buf_types, b.type, "V4L2_BUF_TYPE_???");
		if (code != VIDIOC_DQBUF)
			tprintf(", index=%u", b.index);
	} else {
		if (!syserror(tcp) && umove(tcp, arg, &b) == 0) {
			if (code == VIDIOC_DQBUF)
				tprintf(", index=%u", b.index);
			tprints(", memory=");
			printxval(v4l2_memories, b.memory, "V4L2_MEMORY_???");

			if (b.memory == V4L2_MEMORY_MMAP) {
				tprintf(", m.offset=%#x", b.m.offset);
			} else if (b.memory == V4L2_MEMORY_USERPTR) {
				tprintf(", m.userptr=%#lx",
					(unsigned long) b.m.userptr);
			}

			tprintf(", length=%u, bytesused=%u, flags=",
				b.length, b.bytesused);
			printflags(v4l2_buf_flags, b.flags, "V4L2_BUF_FLAG_???");
			if (code == VIDIOC_DQBUF)
				tprintf(", timestamp = {%ju.%06ju}",
					(uintmax_t)b.timestamp.tv_sec,
					(uintmax_t)b.timestamp.tv_usec);
			tprints(", ...");
		}
		tprints("}");
	}
	return 1;
}

static int
print_v4l2_framebuffer(struct tcb *tcp, const long arg)
{
	struct_v4l2_framebuffer b;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &b)) {
		tprintf("{capability=%#x, flags=%#x, base=%#lx}",
			b.capability, b.flags, (unsigned long) b.base);
	}

	return RVAL_DECODED | 1;
}

static int
print_v4l2_buf_type(struct tcb *tcp, const long arg)
{
	int type;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &type)) {
		tprints("[");
		printxval(v4l2_buf_types, type, "V4L2_BUF_TYPE_???");
		tprints("]");
	}
	return RVAL_DECODED | 1;
}

#include "xlat/v4l2_streaming_capabilities.h"
#include "xlat/v4l2_capture_modes.h"

static int
print_v4l2_streamparm(struct tcb *tcp, const long arg, const bool is_get)
{
	struct v4l2_streamparm s;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_DECODED | 1;
		tprints("{type=");
		printxval(v4l2_buf_types, s.type, "V4L2_BUF_TYPE_???");
		switch (s.type) {
			case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			case V4L2_BUF_TYPE_VIDEO_OUTPUT:
				if (is_get)
					return 0;
				tprints(", ");
				break;
			default:
				tprints("}");
				return RVAL_DECODED | 1;
		}
	} else {
		if (syserror(tcp) || umove(tcp, arg, &s) < 0) {
			tprints("}");
			return 1;
		}
		tprints(is_get ? ", " : " => ");
	}

	if (s.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		tprints("parm.capture={capability=");
		printflags(v4l2_streaming_capabilities,
			   s.parm.capture.capability, "V4L2_CAP_???");

		tprints(", capturemode=");
		printflags(v4l2_capture_modes,
			   s.parm.capture.capturemode, "V4L2_MODE_???");

		tprintf(", timeperframe=" FMT_FRACT,
			ARGS_FRACT(s.parm.capture.timeperframe));

		tprintf(", extendedmode=%u, readbuffers=%u}",
			s.parm.capture.extendedmode,
			s.parm.capture.readbuffers);
	} else {
		tprints("parm.output={capability=");
		printflags(v4l2_streaming_capabilities,
			   s.parm.output.capability, "V4L2_CAP_???");

		tprintf(", outputmode=%u", s.parm.output.outputmode);

		tprintf(", timeperframe=" FMT_FRACT,
			ARGS_FRACT(s.parm.output.timeperframe));

		tprintf(", extendedmode=%u, writebuffers=%u}",
			s.parm.output.extendedmode,
			s.parm.output.writebuffers);
	}
	if (exiting(tcp))
		tprints("}");
	return 1;
}

static int
print_v4l2_standard(struct tcb *tcp, const long arg)
{
	struct_v4l2_standard s;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_DECODED | 1;
		tprintf("{index=%u", s.index);
	} else {
		if (!syserror(tcp) && !umove(tcp, arg, &s)) {
			tprints(", name=");
			print_quoted_string((const char *) s.name,
					    sizeof(s.name),
					    QUOTE_0_TERMINATED);
			tprintf(", frameperiod=" FMT_FRACT,
				ARGS_FRACT(s.frameperiod));
			tprintf(", framelines=%d", s.framelines);
		}
		tprints("}");
	}
	return 1;
}

#include "xlat/v4l2_input_types.h"

static int
print_v4l2_input(struct tcb *tcp, const long arg)
{
	struct_v4l2_input i;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &i))
			return RVAL_DECODED | 1;
		tprintf("{index=%u", i.index);
	} else {
		if (!syserror(tcp) && !umove(tcp, arg, &i)) {
			tprints(", name=");
			print_quoted_string((const char *) i.name,
					    sizeof(i.name),
					    QUOTE_0_TERMINATED);
			tprints(", type=");
			printxval(v4l2_input_types, i.type,
				  "V4L2_INPUT_TYPE_???");
		}
		tprints("}");
	}
	return 1;
}

#include "xlat/v4l2_control_ids.h"

static int
print_v4l2_control(struct tcb *tcp, const long arg, const bool is_get)
{
	struct v4l2_control c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_DECODED | 1;
		tprints("{id=");
		printxval(v4l2_control_ids, c.id, "V4L2_CID_???");
		if (!is_get)
			tprintf(", value=%d", c.value);
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &c)) {
		tprints(is_get ? ", " : " => ");
		tprintf("value=%d", c.value);
	}

	tprints("}");
	return 1;
}

#include "xlat/v4l2_control_types.h"
#include "xlat/v4l2_control_flags.h"

static int
print_v4l2_queryctrl(struct tcb *tcp, const long arg)
{
	struct v4l2_queryctrl c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_DECODED | 1;
		tprints("{id=");
	} else {
		if (syserror(tcp) || umove(tcp, arg, &c) < 0) {
			tprints("}");
			return 1;
		}
		if (tcp->auxstr)
			tprints(" => ");
	}

	if (entering(tcp) || tcp->auxstr) {
#ifdef V4L2_CTRL_FLAG_NEXT_CTRL
		tcp->auxstr = (c.id & V4L2_CTRL_FLAG_NEXT_CTRL) ? "" : NULL;
		if (tcp->auxstr) {
			tprints("V4L2_CTRL_FLAG_NEXT_CTRL|");
			c.id &= ~V4L2_CTRL_FLAG_NEXT_CTRL;
		}
#endif
		printxval(v4l2_control_ids, c.id, "V4L2_CID_???");
	}

	if (exiting(tcp)) {
		tprints(", type=");
		printxval(v4l2_control_types, c.type, "V4L2_CTRL_TYPE_???");
		tprints(", name=");
		print_quoted_string((const char *) c.name,
				    sizeof(c.name),
				    QUOTE_0_TERMINATED);
		tprintf(", minimum=%d, maximum=%d, step=%d"
			", default_value=%d, flags=",
			c.minimum, c.maximum, c.step, c.default_value);
		printflags(v4l2_control_flags, c.flags, "V4L2_CTRL_FLAG_???");
		tprints("}");
	}
	return 1;
}

static int
print_v4l2_cropcap(struct tcb *tcp, const long arg)
{
	struct v4l2_cropcap c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_DECODED | 1;
		tprints("{type=");
		printxval(v4l2_buf_types, c.type, "V4L2_BUF_TYPE_???");
		return 0;
	}
	if (!syserror(tcp) && !umove(tcp, arg, &c)) {
		tprintf(", bounds=" FMT_RECT
			", defrect=" FMT_RECT
			", pixelaspect=" FMT_FRACT,
			ARGS_RECT(c.bounds),
			ARGS_RECT(c.defrect),
			ARGS_FRACT(c.pixelaspect));
	}
	tprints("}");
	return 1;
}

static int
print_v4l2_crop(struct tcb *tcp, const long arg, const bool is_get)
{
	struct v4l2_crop c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_DECODED | 1;
		tprints("{type=");
		printxval(v4l2_buf_types, c.type, "V4L2_BUF_TYPE_???");
		if (is_get)
			return 0;
		tprintf(", c=" FMT_RECT, ARGS_RECT(c.c));
	} else {
		if (!syserror(tcp) && !umove(tcp, arg, &c))
			tprintf(", c=" FMT_RECT, ARGS_RECT(c.c));
	}

	tprints("}");
	return RVAL_DECODED | 1;
}

#ifdef VIDIOC_S_EXT_CTRLS
static bool
print_v4l2_ext_control(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct_v4l2_ext_control *p = elem_buf;

	tprints("{id=");
	printxval(v4l2_control_ids, p->id, "V4L2_CID_???");
# if HAVE_DECL_V4L2_CTRL_TYPE_STRING
	tprintf(", size=%u", p->size);
	if (p->size > 0) {
		tprints(", string=");
		printstr(tcp, (long) p->string, p->size);
	} else
# endif
	tprintf(", value=%d, value64=%lld", p->value,
		(long long) p->value64);
	tprints("}");

	return true;
}

#include "xlat/v4l2_control_classes.h"

static int
umoven_or_printaddr_ignore_syserror(struct tcb *tcp, const long addr,
				    const unsigned int len, void *our_addr)
{
	if (!addr) {
		tprints("NULL");
		return -1;
	}
	if (umoven(tcp, addr, len, our_addr) < 0) {
		tprintf("%#lx", addr);
		return -1;
	}
	return 0;
}

static int
print_v4l2_ext_controls(struct tcb *tcp, const long arg, const bool is_get)
{
	struct_v4l2_ext_controls c;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_DECODED | 1;
		tprints("{ctrl_class=");
		printxval(v4l2_control_classes, c.ctrl_class,
			  "V4L2_CTRL_CLASS_???");
		tprintf(", count=%u", c.count);
		if (!c.count) {
			tprints("}");
			return RVAL_DECODED | 1;
		}
		if (is_get)
			return 0;
		tprints(", ");
	} else {
		if (umove(tcp, arg, &c) < 0) {
			tprints("}");
			return 1;
		}
		tprints(is_get ? ", " : " => ");
	}

	tprints("controls=");
	struct_v4l2_ext_control ctrl;
	bool fail = !print_array(tcp, (unsigned long) c.controls, c.count,
				 &ctrl, sizeof(ctrl),
				 umoven_or_printaddr_ignore_syserror,
				 print_v4l2_ext_control, 0);

	if (exiting(tcp) && syserror(tcp))
		tprintf(", error_idx=%u", c.error_idx);

	if (exiting(tcp) || fail) {
		tprints("}");
		return RVAL_DECODED | 1;
	}
	return 1;
}
#endif /* VIDIOC_S_EXT_CTRLS */

#ifdef VIDIOC_ENUM_FRAMESIZES
# include "xlat/v4l2_framesize_types.h"

static int
print_v4l2_frmsizeenum(struct tcb *tcp, const long arg)
{
	struct v4l2_frmsizeenum s;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &s))
			return RVAL_DECODED | 1;
		tprintf("{index=%u, pixel_format=", s.index);
		print_pixelformat(s.pixel_format);
		return 0;
	}

	if (!syserror(tcp) && !umove(tcp, arg, &s)) {
		tprints(", type=");
		printxval(v4l2_framesize_types, s.type, "V4L2_FRMSIZE_TYPE_???");
		switch (s.type) {
		case V4L2_FRMSIZE_TYPE_DISCRETE:
			tprintf(", discrete={width=%u, height=%u}",
				s.discrete.width, s.discrete.height);
			break;
		case V4L2_FRMSIZE_TYPE_STEPWISE:
			tprintf(", stepwise={min_width=%u, max_width=%u, "
				"step_width=%u, min_height=%u, max_height=%u, "
				"step_height=%u}",
				s.stepwise.min_width, s.stepwise.max_width,
				s.stepwise.step_width, s.stepwise.min_height,
				s.stepwise.max_height, s.stepwise.step_height);
			break;
		}
	}
	tprints("}");
	return 1;
}
#endif /* VIDIOC_ENUM_FRAMESIZES */

#ifdef VIDIOC_ENUM_FRAMEINTERVALS
# include "xlat/v4l2_frameinterval_types.h"

static int
print_v4l2_frmivalenum(struct tcb *tcp, const long arg)
{
	struct v4l2_frmivalenum f;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &f))
			return RVAL_DECODED | 1;
		tprintf("{index=%u, pixel_format=", f.index);
		print_pixelformat(f.pixel_format);
		tprintf(", width=%u, height=%u", f.width, f.height);
		return 0;
	}
	if (!syserror(tcp) && !umove(tcp, arg, &f)) {
		tprints(", type=");
		printxval(v4l2_frameinterval_types, f.type,
			  "V4L2_FRMIVAL_TYPE_???");
		switch (f.type) {
		case V4L2_FRMIVAL_TYPE_DISCRETE:
			tprintf(", discrete=" FMT_FRACT,
				ARGS_FRACT(f.discrete));
			break;
		case V4L2_FRMIVAL_TYPE_STEPWISE:
		case V4L2_FRMSIZE_TYPE_CONTINUOUS:
			tprintf(", stepwise={min=" FMT_FRACT ", max="
				FMT_FRACT ", step=" FMT_FRACT "}",
				ARGS_FRACT(f.stepwise.min),
				ARGS_FRACT(f.stepwise.max),
				ARGS_FRACT(f.stepwise.step));
			break;
		}
	}
	tprints("}");
	return 1;
}
#endif /* VIDIOC_ENUM_FRAMEINTERVALS */

#ifdef VIDIOC_CREATE_BUFS
static int
print_v4l2_create_buffers(struct tcb *tcp, const long arg)
{
	struct_v4l2_create_buffers b;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &b))
			return RVAL_DECODED | 1;
		tprintf("{count=%u, memory=", b.count);
		printxval(v4l2_memories, b.memory, "V4L2_MEMORY_???");
		tprints(", format={type=");
		printxval(v4l2_buf_types, b.format.type,
			  "V4L2_BUF_TYPE_???");
		print_v4l2_format_fmt(", ",
				      (struct_v4l2_format *) &b.format);
		tprints("}}");
		return 0;
	} else {
		static const char fmt[] = "{index=%u, count=%u}";
		static char outstr[sizeof(fmt) + sizeof(int) * 6];

		if (syserror(tcp) || umove(tcp, arg, &b) < 0)
			return 1;
		sprintf(outstr, fmt, b.index, b.count);
		tcp->auxstr = outstr;
		return 1 + RVAL_STR;
	}
}
#endif /* VIDIOC_CREATE_BUFS */

MPERS_PRINTER_DECL(int, v4l2_ioctl)(struct tcb *tcp, const unsigned int code, const long arg)
{
#ifdef ENABLE_DATASERIES
	if (!ds_module) {
#endif
	if (!verbose(tcp))
		return RVAL_DECODED;
#ifdef ENABLE_DATASERIES
	}
#endif

	switch (code) {
	case VIDIOC_QUERYCAP: /* R */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_capability));
#endif
		return print_v4l2_capability(tcp, arg);

	case VIDIOC_ENUM_FMT: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_fmtdesc));
#endif
		return print_v4l2_fmtdesc(tcp, arg);

	case VIDIOC_G_FMT: /* RW */
	case VIDIOC_S_FMT: /* RW */
	case VIDIOC_TRY_FMT: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct_v4l2_format));
#endif
		return print_v4l2_format(tcp, arg, code == VIDIOC_G_FMT);

	case VIDIOC_REQBUFS: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_requestbuffers));
#endif
		return print_v4l2_requestbuffers(tcp, arg);

	case VIDIOC_QUERYBUF: /* RW */
	case VIDIOC_QBUF: /* RW */
	case VIDIOC_DQBUF: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct_v4l2_buffer));
#endif
		return print_v4l2_buffer(tcp, code, arg);

	case VIDIOC_G_FBUF: /* R */
		if (entering(tcp))
			return 0;
		/* fall through */
	case VIDIOC_S_FBUF: /* W */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct_v4l2_framebuffer));
#endif
		return print_v4l2_framebuffer(tcp, arg);

	case VIDIOC_STREAMON: /* W */
	case VIDIOC_STREAMOFF: /* W */
#ifdef ENABLE_DATASERIES
		if (ds_module)
		  ds_set_ioctl_size(ds_module, sizeof(int));
#endif
		return print_v4l2_buf_type(tcp, arg);

	case VIDIOC_G_PARM: /* RW */
	case VIDIOC_S_PARM: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_streamparm));
#endif
		return print_v4l2_streamparm(tcp, arg, code == VIDIOC_G_PARM);

	case VIDIOC_G_STD: /* R */
		if (entering(tcp))
			return 0;
		/* fall through */
	case VIDIOC_S_STD: /* W */
		tprints(", ");
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(int64_t));
#endif
		printnum_int64(tcp, arg, "%#" PRIx64);
		return RVAL_DECODED | 1;

	case VIDIOC_ENUMSTD: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct_v4l2_standard));
#endif
		return print_v4l2_standard(tcp, arg);

	case VIDIOC_ENUMINPUT: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct_v4l2_input));
#endif
		return print_v4l2_input(tcp, arg);

	case VIDIOC_G_CTRL: /* RW */
	case VIDIOC_S_CTRL: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_control));
#endif
		return print_v4l2_control(tcp, arg, code == VIDIOC_G_CTRL);

	case VIDIOC_QUERYCTRL: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_queryctrl));
#endif
		return print_v4l2_queryctrl(tcp, arg);

	case VIDIOC_G_INPUT: /* R */
		if (entering(tcp))
			return 0;
		/* fall through */
	case VIDIOC_S_INPUT: /* RW */
		tprints(", ");
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(unsigned int));
#endif
		printnum_int(tcp, arg, "%u");
		return RVAL_DECODED | 1;

	case VIDIOC_CROPCAP: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_cropcap));
#endif
		return print_v4l2_cropcap(tcp, arg);

	case VIDIOC_G_CROP: /* RW */
	case VIDIOC_S_CROP: /* W */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(struct v4l2_crop));
#endif
		return print_v4l2_crop(tcp, arg, code == VIDIOC_G_CROP);

#ifdef VIDIOC_S_EXT_CTRLS
	case VIDIOC_S_EXT_CTRLS: /* RW */
	case VIDIOC_TRY_EXT_CTRLS: /* RW */
	case VIDIOC_G_EXT_CTRLS: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct_v4l2_ext_controls));
#endif
		return print_v4l2_ext_controls(tcp, arg,
					       code == VIDIOC_G_EXT_CTRLS);
#endif /* VIDIOC_S_EXT_CTRLS */

#ifdef VIDIOC_ENUM_FRAMESIZES
	case VIDIOC_ENUM_FRAMESIZES: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_frmsizeenum));
#endif
		return print_v4l2_frmsizeenum(tcp, arg);
#endif /* VIDIOC_ENUM_FRAMESIZES */

#ifdef VIDIOC_ENUM_FRAMEINTERVALS
	case VIDIOC_ENUM_FRAMEINTERVALS: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct v4l2_frmivalenum));
#endif
		return print_v4l2_frmivalenum(tcp, arg);
#endif /* VIDIOC_ENUM_FRAMEINTERVALS */

#ifdef VIDIOC_CREATE_BUFS
	case VIDIOC_CREATE_BUFS: /* RW */
#ifdef ENABLE_DATASERIES
		if (ds_module)
			ds_set_ioctl_size(ds_module, sizeof(
					     struct_v4l2_create_buffers));
#endif
		return print_v4l2_create_buffers(tcp, arg);
#endif /* VIDIOC_CREATE_BUFS */

	default:
		return RVAL_DECODED;
	}
}
