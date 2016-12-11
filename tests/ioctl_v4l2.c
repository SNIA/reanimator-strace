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

#include "tests.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>

static const unsigned int magic = 0xdeadbeef;

static void
init_magic(void *addr, const unsigned int size)
{
	unsigned int *p = addr;
	const unsigned int *end = addr + size;

	for (; p < end; ++p)
		*(unsigned int *) p = magic;
}

int
main(void )
{
	const unsigned int size = get_page_size();
	void *const page = tail_alloc(size);
	init_magic(page, size);

	const union u_pixel_format {
		unsigned int pixelformat;
		unsigned char cc[sizeof(int)];
	} u = {
#if WORDS_BIGENDIAN
		.cc = {
			(unsigned char) (magic >> 24),
			(unsigned char) (magic >> 16),
			(unsigned char) (magic >> 8),
			(unsigned char) magic
		}
#else
		.pixelformat = magic
#endif
	};

	/* VIDIOC_QUERYCAP */
	ioctl(-1, VIDIOC_QUERYCAP, 0);
	printf("ioctl(-1, VIDIOC_QUERYCAP, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_QUERYCAP, page);
	printf("ioctl(-1, VIDIOC_QUERYCAP, %p) = -1 EBADF (%m)\n", page);

	/* VIDIOC_ENUM_FMT */
	ioctl(-1, VIDIOC_ENUM_FMT, 0);
	printf("ioctl(-1, VIDIOC_ENUM_FMT, NULL) = -1 EBADF (%m)\n");

	struct v4l2_fmtdesc *const p_fmtdesc = tail_alloc(sizeof(*p_fmtdesc));
	p_fmtdesc->index = magic;
	p_fmtdesc->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(-1, VIDIOC_ENUM_FMT, p_fmtdesc);
	printf("ioctl(-1, VIDIOC_ENUM_FMT, {index=%u"
	       ", type=V4L2_BUF_TYPE_VIDEO_CAPTURE}) = -1 EBADF (%m)\n", magic);

	/* VIDIOC_G_FMT */
	ioctl(-1, VIDIOC_G_FMT, 0);
	printf("ioctl(-1, VIDIOC_G_FMT, NULL) = -1 EBADF (%m)\n");

	struct v4l2_format *const p_format =
		tail_alloc(sizeof(*p_format));
	p_format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ioctl(-1, VIDIOC_G_FMT, p_format);
	printf("ioctl(-1, VIDIOC_G_FMT"
	       ", {type=V4L2_BUF_TYPE_VIDEO_CAPTURE}) = -1 EBADF (%m)\n");

	/* VIDIOC_S_FMT */
	ioctl(-1, VIDIOC_S_FMT, 0);
	printf("ioctl(-1, VIDIOC_S_FMT, NULL) = -1 EBADF (%m)\n");

	p_format->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	p_format->fmt.pix.width = 0xdad1beaf;
	p_format->fmt.pix.height = 0xdad2beaf;
	p_format->fmt.pix.pixelformat = 0xdeadbeef;
	p_format->fmt.pix.field = V4L2_FIELD_NONE;
	p_format->fmt.pix.bytesperline = 0xdad3beaf;
	p_format->fmt.pix.sizeimage = 0xdad4beaf;
	p_format->fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;

	ioctl(-1, VIDIOC_S_FMT, p_format);
	printf("ioctl(-1, VIDIOC_S_FMT, {type=V4L2_BUF_TYPE_VIDEO_OUTPUT"
	       ", fmt.pix={width=%u, height=%u, pixelformat="
	       "v4l2_fourcc('\\x%x', '\\x%x', '\\x%x', '\\x%x')"
	       ", field=V4L2_FIELD_NONE, bytesperline=%u, sizeimage=%u"
	       ", colorspace=V4L2_COLORSPACE_JPEG}}) = -1 EBADF (%m)\n",
	       p_format->fmt.pix.width, p_format->fmt.pix.height,
	       u.cc[0], u.cc[1], u.cc[2], u.cc[3],
	       p_format->fmt.pix.bytesperline, p_format->fmt.pix.sizeimage);

	/* VIDIOC_TRY_FMT */
	ioctl(-1, VIDIOC_TRY_FMT, 0);
	printf("ioctl(-1, VIDIOC_TRY_FMT, NULL) = -1 EBADF (%m)\n");

#if HAVE_DECL_V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
	memset(p_format, -1, sizeof(*p_format));
	p_format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	p_format->fmt.pix_mp.width = 0xdad1beaf;
	p_format->fmt.pix_mp.height = 0xdad2beaf;
	p_format->fmt.pix_mp.pixelformat = 0xdeadbeef;
	p_format->fmt.pix_mp.field = V4L2_FIELD_NONE;
	p_format->fmt.pix_mp.colorspace = V4L2_COLORSPACE_JPEG;
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(p_format->fmt.pix_mp.plane_fmt); ++i) {
		p_format->fmt.pix_mp.plane_fmt[i].sizeimage = 0xbadc0de0 | i;
		p_format->fmt.pix_mp.plane_fmt[i].bytesperline = 0xdadbeaf0 | i;
	}
	ioctl(-1, VIDIOC_TRY_FMT, p_format);
	printf("ioctl(-1, VIDIOC_TRY_FMT"
	       ", {type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE"
	       ", fmt.pix_mp={width=%u, height=%u, pixelformat="
	       "v4l2_fourcc('\\x%x', '\\x%x', '\\x%x', '\\x%x')"
	       ", field=V4L2_FIELD_NONE, colorspace=V4L2_COLORSPACE_JPEG"
	       ", plane_fmt=[",
	       p_format->fmt.pix_mp.width, p_format->fmt.pix_mp.height,
	       u.cc[0], u.cc[1], u.cc[2], u.cc[3]);
	for (i = 0; i < ARRAY_SIZE(p_format->fmt.pix_mp.plane_fmt); ++i) {
		if (i)
			printf(", ");
		printf("{sizeimage=%u, bytesperline=%u}",
		       p_format->fmt.pix_mp.plane_fmt[i].sizeimage,
		       p_format->fmt.pix_mp.plane_fmt[i].bytesperline);
	}
	printf("], num_planes=%u}}) = -1 EBADF (%m)\n",
	       p_format->fmt.pix_mp.num_planes);
#else
	ioctl(-1, VIDIOC_TRY_FMT, page);
	printf("ioctl(-1, VIDIOC_TRY_FMT, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", magic);
#endif

	/* VIDIOC_REQBUFS */
	ioctl(-1, VIDIOC_REQBUFS, 0);
	printf("ioctl(-1, VIDIOC_REQBUFS, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_REQBUFS,
	      page + size - sizeof(struct v4l2_requestbuffers));
	printf("ioctl(-1, VIDIOC_REQBUFS, {count=%u, type=%#x"
	       " /* V4L2_BUF_TYPE_??? */, memory=%#x /* V4L2_MEMORY_??? */})"
	       " = -1 EBADF (%m)\n", magic, magic, magic);

	/* VIDIOC_QUERYBUF */
	ioctl(-1, VIDIOC_QUERYBUF, 0);
	printf("ioctl(-1, VIDIOC_QUERYBUF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_QUERYBUF, page + size - sizeof(struct v4l2_buffer));
	printf("ioctl(-1, VIDIOC_QUERYBUF, {type=%#x /* V4L2_BUF_TYPE_??? */"
	       ", index=%u}) = -1 EBADF (%m)\n", magic, magic);

	/* VIDIOC_QBUF */
	ioctl(-1, VIDIOC_QBUF, 0);
	printf("ioctl(-1, VIDIOC_QBUF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_QBUF, page + size - sizeof(struct v4l2_buffer));
	printf("ioctl(-1, VIDIOC_QBUF, {type=%#x /* V4L2_BUF_TYPE_??? */"
	       ", index=%u}) = -1 EBADF (%m)\n", magic, magic);

	/* VIDIOC_DQBUF */
	ioctl(-1, VIDIOC_DQBUF, 0);
	printf("ioctl(-1, VIDIOC_DQBUF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_DQBUF, page + size - sizeof(struct v4l2_buffer));
	printf("ioctl(-1, VIDIOC_DQBUF, {type=%#x"
	       " /* V4L2_BUF_TYPE_??? */}) = -1 EBADF (%m)\n", magic);

	/* VIDIOC_G_FBUF */
	ioctl(-1, VIDIOC_G_FBUF, 0);
	printf("ioctl(-1, VIDIOC_G_FBUF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_FBUF, page);
	printf("ioctl(-1, VIDIOC_G_FBUF, %p) = -1 EBADF (%m)\n", page);

	/* VIDIOC_S_FBUF */
	ioctl(-1, VIDIOC_S_FBUF, 0);
	printf("ioctl(-1, VIDIOC_S_FBUF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_FBUF, page + size - sizeof(struct v4l2_framebuffer));
	printf("ioctl(-1, VIDIOC_S_FBUF, {capability=%#x"
	       ", flags=%#x, base=%#lx}) = -1 EBADF (%m)\n",
	       magic, magic, *(unsigned long *) page);

	/* VIDIOC_STREAMON */
	ioctl(-1, VIDIOC_STREAMON, 0);
	printf("ioctl(-1, VIDIOC_STREAMON, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_STREAMON, page + size - sizeof(int));
	printf("ioctl(-1, VIDIOC_STREAMON, [%#x /* V4L2_BUF_TYPE_??? */])"
	       " = -1 EBADF (%m)\n", magic);

	/* VIDIOC_STREAMOFF */
	ioctl(-1, VIDIOC_STREAMOFF, 0);
	printf("ioctl(-1, VIDIOC_STREAMOFF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_STREAMOFF, page + size - sizeof(int));
	printf("ioctl(-1, VIDIOC_STREAMOFF, [%#x /* V4L2_BUF_TYPE_??? */])"
	       " = -1 EBADF (%m)\n", magic);

	/* VIDIOC_G_PARM */
	ioctl(-1, VIDIOC_G_PARM, 0);
	printf("ioctl(-1, VIDIOC_G_PARM, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_PARM, page + size - sizeof(struct v4l2_streamparm));
	printf("ioctl(-1, VIDIOC_G_PARM, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", magic);

	/* VIDIOC_S_PARM */
	ioctl(-1, VIDIOC_S_PARM, 0);
	printf("ioctl(-1, VIDIOC_S_PARM, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_PARM, page);
	printf("ioctl(-1, VIDIOC_S_PARM, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", magic);

	struct v4l2_streamparm *const p_streamparm =
		tail_alloc(sizeof(*p_streamparm));
	p_streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	p_streamparm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	p_streamparm->parm.capture.capturemode = V4L2_MODE_HIGHQUALITY;
	p_streamparm->parm.capture.timeperframe.numerator = 0xdeadbeef;
	p_streamparm->parm.capture.timeperframe.denominator = 0xbadc0ded;
	ioctl(-1, VIDIOC_S_PARM, p_streamparm);
	printf("ioctl(-1, VIDIOC_S_PARM, {type=V4L2_BUF_TYPE_VIDEO_CAPTURE"
	       ", parm.capture={capability=V4L2_CAP_TIMEPERFRAME"
	       ", capturemode=V4L2_MODE_HIGHQUALITY, timeperframe=%u/%u"
	       ", extendedmode=%u, readbuffers=%u}}) = -1 EBADF (%m)\n",
	       p_streamparm->parm.capture.timeperframe.numerator,
	       p_streamparm->parm.capture.timeperframe.denominator, -1U, -1U);

	p_streamparm->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	p_streamparm->parm.output.outputmode = 0;
	ioctl(-1, VIDIOC_S_PARM, p_streamparm);
	printf("ioctl(-1, VIDIOC_S_PARM, {type=V4L2_BUF_TYPE_VIDEO_OUTPUT"
	       ", parm.output={capability=V4L2_CAP_TIMEPERFRAME"
	       ", outputmode=0, timeperframe=%u/%u"
	       ", extendedmode=%u, writebuffers=%u}}) = -1 EBADF (%m)\n",
	       p_streamparm->parm.output.timeperframe.numerator,
	       p_streamparm->parm.output.timeperframe.denominator, -1U, -1U);

	/* VIDIOC_G_STD */
	ioctl(-1, VIDIOC_G_STD, 0);
	printf("ioctl(-1, VIDIOC_G_STD, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_STD, page);
	printf("ioctl(-1, VIDIOC_G_STD, %p) = -1 EBADF (%m)\n", page);

	/* VIDIOC_S_STD */
	ioctl(-1, VIDIOC_S_STD, 0);
	printf("ioctl(-1, VIDIOC_S_STD, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_STD, page + size - sizeof(long long));
	printf("ioctl(-1, VIDIOC_S_STD, [%#llx]) = -1 EBADF (%m)\n",
	       *(unsigned long long *) page);

	/* VIDIOC_ENUMSTD */
	ioctl(-1, VIDIOC_ENUMSTD, 0);
	printf("ioctl(-1, VIDIOC_ENUMSTD, NULL) = -1 EBADF (%m)\n");

	struct v4l2_standard *const p_standard =
		tail_alloc(sizeof(*p_standard));
	p_standard->index = magic;
	ioctl(-1, VIDIOC_ENUMSTD, p_standard);
	printf("ioctl(-1, VIDIOC_ENUMSTD, {index=%u}) = -1 EBADF (%m)\n",
	       magic);

	/* VIDIOC_ENUMINPUT */
	ioctl(-1, VIDIOC_ENUMINPUT, 0);
	printf("ioctl(-1, VIDIOC_ENUMINPUT, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_ENUMINPUT, page);
	printf("ioctl(-1, VIDIOC_ENUMINPUT, {index=%u}) = -1 EBADF (%m)\n",
	       magic);

	/* VIDIOC_G_CTRL */
	ioctl(-1, VIDIOC_G_CTRL, 0);
	printf("ioctl(-1, VIDIOC_G_CTRL, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_CTRL, page + size - sizeof(struct v4l2_control));
	printf("ioctl(-1, VIDIOC_G_CTRL, {id=%#x /* V4L2_CID_??? */})"
	       " = -1 EBADF (%m)\n", magic);

	/* VIDIOC_S_CTRL */
	ioctl(-1, VIDIOC_S_CTRL, 0);
	printf("ioctl(-1, VIDIOC_S_CTRL, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_CTRL, page + size - sizeof(struct v4l2_control));
	printf("ioctl(-1, VIDIOC_S_CTRL, {id=%#x /* V4L2_CID_??? */"
	       ", value=%d}) = -1 EBADF (%m)\n", magic, magic);

	/* VIDIOC_QUERYCTRL */
	ioctl(-1, VIDIOC_QUERYCTRL, 0);
	printf("ioctl(-1, VIDIOC_QUERYCTRL, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_QUERYCTRL,
	      page + size - sizeof(struct v4l2_queryctrl));
# ifdef V4L2_CTRL_FLAG_NEXT_CTRL
	printf("ioctl(-1, VIDIOC_QUERYCTRL, {id=V4L2_CTRL_FLAG_NEXT_CTRL"
	       "|%#x /* V4L2_CID_??? */}) = -1 EBADF (%m)\n",
	       magic & ~V4L2_CTRL_FLAG_NEXT_CTRL);
# else
	printf("ioctl(-1, VIDIOC_QUERYCTRL, {id=%#x /* V4L2_CID_??? */})"
	       " = -1 EBADF (%m)\n", magic);
# endif

	struct v4l2_queryctrl *const p_queryctrl =
		tail_alloc(sizeof(*p_queryctrl));
	p_queryctrl->id = V4L2_CID_SATURATION;
	ioctl(-1, VIDIOC_QUERYCTRL, p_queryctrl);
	printf("ioctl(-1, VIDIOC_QUERYCTRL, {id=V4L2_CID_SATURATION})"
	       " = -1 EBADF (%m)\n");

	/* VIDIOC_G_INPUT */
	ioctl(-1, VIDIOC_G_INPUT, 0);
	printf("ioctl(-1, VIDIOC_G_INPUT, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_INPUT, page);
	printf("ioctl(-1, VIDIOC_G_INPUT, %p) = -1 EBADF (%m)\n", page);

	/* VIDIOC_S_INPUT */
	ioctl(-1, VIDIOC_S_INPUT, 0);
	printf("ioctl(-1, VIDIOC_S_INPUT, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_INPUT, page + size - sizeof(int));
	printf("ioctl(-1, VIDIOC_S_INPUT, [%u]) = -1 EBADF (%m)\n", magic);

	/* VIDIOC_CROPCAP */
	ioctl(-1, VIDIOC_CROPCAP, 0);
	printf("ioctl(-1, VIDIOC_CROPCAP, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_CROPCAP, page + size - sizeof(struct v4l2_cropcap));
	printf("ioctl(-1, VIDIOC_CROPCAP, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", magic);

	/* VIDIOC_G_CROP */
	ioctl(-1, VIDIOC_G_CROP, 0);
	printf("ioctl(-1, VIDIOC_G_CROP, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_CROP, page + size - sizeof(struct v4l2_crop));
	printf("ioctl(-1, VIDIOC_G_CROP, {type=%#x /* V4L2_BUF_TYPE_??? */})"
	       " = -1 EBADF (%m)\n", magic);

	/* VIDIOC_S_CROP */
	ioctl(-1, VIDIOC_S_CROP, 0);
	printf("ioctl(-1, VIDIOC_S_CROP, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_S_CROP, page + size - sizeof(struct v4l2_crop));
	printf("ioctl(-1, VIDIOC_S_CROP, {type=%#x /* V4L2_BUF_TYPE_??? */"
	       ", c={left=%d, top=%d, width=%u, height=%u}}) = -1 EBADF (%m)\n",
	       magic, magic, magic, magic, magic);

#ifdef VIDIOC_S_EXT_CTRLS
	/* VIDIOC_S_EXT_CTRLS */
	ioctl(-1, VIDIOC_S_EXT_CTRLS, 0);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS, NULL) = -1 EBADF (%m)\n");

	struct v4l2_ext_controls *const p_ext_controls =
		tail_alloc(sizeof(*p_ext_controls));
	p_ext_controls->ctrl_class = V4L2_CTRL_CLASS_USER;
	p_ext_controls->count = 0;
	p_ext_controls->controls = (void *) -2UL;
	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS, {ctrl_class=V4L2_CTRL_CLASS_USER"
	       ", count=%u}) = -1 EBADF (%m)\n", p_ext_controls->count);

	p_ext_controls->ctrl_class = V4L2_CTRL_CLASS_MPEG;
	p_ext_controls->count = magic;
	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS, {ctrl_class=V4L2_CTRL_CLASS_MPEG"
	       ", count=%u, controls=%p}) = -1 EBADF (%m)\n",
	       p_ext_controls->count, p_ext_controls->controls);

# if HAVE_DECL_V4L2_CTRL_TYPE_STRING
	p_ext_controls->count = 2;
	p_ext_controls->controls =
		tail_alloc(sizeof(*p_ext_controls->controls) * p_ext_controls->count);
	p_ext_controls->controls[0].id = V4L2_CID_BRIGHTNESS;
	p_ext_controls->controls[0].size = 0;
	p_ext_controls->controls[0].value64 = 0xfacefeeddeadbeefULL;
	p_ext_controls->controls[1].id = V4L2_CID_CONTRAST;
	p_ext_controls->controls[1].size = 2;
	p_ext_controls->controls[1].string =
		tail_alloc(p_ext_controls->controls[1].size);

	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS"
	       ", {ctrl_class=V4L2_CTRL_CLASS_MPEG, count=%u, controls="
	       "[{id=V4L2_CID_BRIGHTNESS, size=0, value=%d, value64=%lld}"
	       ", {id=V4L2_CID_CONTRAST, size=2, string=\"\\377\\377\"}"
	       "] => controls="
	       "[{id=V4L2_CID_BRIGHTNESS, size=0, value=%d, value64=%lld}"
	       ", {id=V4L2_CID_CONTRAST, size=2, string=\"\\377\\377\"}"
	       "], error_idx=%u}) = -1 EBADF (%m)\n",
	       p_ext_controls->count,
	       p_ext_controls->controls[0].value,
	       (long long) p_ext_controls->controls[0].value64,
	       p_ext_controls->controls[0].value,
	       (long long) p_ext_controls->controls[0].value64,
	       p_ext_controls->error_idx);

	++p_ext_controls->count;
	ioctl(-1, VIDIOC_S_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_S_EXT_CTRLS"
	       ", {ctrl_class=V4L2_CTRL_CLASS_MPEG, count=%u, controls="
	       "[{id=V4L2_CID_BRIGHTNESS, size=0, value=%d, value64=%lld}"
	       ", {id=V4L2_CID_CONTRAST, size=2, string=\"\\377\\377\"}"
	       ", %p]}) = -1 EBADF (%m)\n",
	       p_ext_controls->count,
	       p_ext_controls->controls[0].value,
	       (long long) p_ext_controls->controls[0].value64,
	       p_ext_controls->controls + 2);
# endif /* HAVE_DECL_V4L2_CTRL_TYPE_STRING */

	/* VIDIOC_TRY_EXT_CTRLS */
	ioctl(-1, VIDIOC_TRY_EXT_CTRLS, 0);
	printf("ioctl(-1, VIDIOC_TRY_EXT_CTRLS, NULL) = -1 EBADF (%m)\n");

	p_ext_controls->ctrl_class = V4L2_CTRL_CLASS_USER;
	p_ext_controls->count = magic;
	p_ext_controls->controls = (void *) -2UL;
	ioctl(-1, VIDIOC_TRY_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_TRY_EXT_CTRLS"
	       ", {ctrl_class=V4L2_CTRL_CLASS_USER, count=%u, controls=%p})"
	       " = -1 EBADF (%m)\n",
	       p_ext_controls->count, p_ext_controls->controls);

	/* VIDIOC_G_EXT_CTRLS */
	ioctl(-1, VIDIOC_G_EXT_CTRLS, 0);
	printf("ioctl(-1, VIDIOC_G_EXT_CTRLS, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_G_EXT_CTRLS, p_ext_controls);
	printf("ioctl(-1, VIDIOC_G_EXT_CTRLS"
	       ", {ctrl_class=V4L2_CTRL_CLASS_USER, count=%u, controls=%p"
	       ", error_idx=%u}) = -1 EBADF (%m)\n",
	       p_ext_controls->count, p_ext_controls->controls,
	       p_ext_controls->error_idx);
#endif /* VIDIOC_S_EXT_CTRLS */

#ifdef VIDIOC_ENUM_FRAMESIZES
	ioctl(-1, VIDIOC_ENUM_FRAMESIZES, 0);
	printf("ioctl(-1, VIDIOC_ENUM_FRAMESIZES, NULL) = -1 EBADF (%m)\n");

	struct v4l2_frmsizeenum *const p_frmsizeenum =
		tail_alloc(sizeof(*p_frmsizeenum));
	p_frmsizeenum->index = magic;
	const union u_pixel_format u_frmsizeenum = {
		.cc = { 'A', '\'', '\\', '\xfa' }
	};
#if WORDS_BIGENDIAN
	p_frmsizeenum->pixel_format =
		(unsigned) u_frmsizeenum.cc[0] << 24 |
		(unsigned) u_frmsizeenum.cc[1] << 16 |
		(unsigned) u_frmsizeenum.cc[2] << 8 |
		(unsigned) u_frmsizeenum.cc[3];
#else
	p_frmsizeenum->pixel_format = u_frmsizeenum.pixelformat;
#endif

	ioctl(-1, VIDIOC_ENUM_FRAMESIZES, p_frmsizeenum);
	printf("ioctl(-1, VIDIOC_ENUM_FRAMESIZES, {index=%u"
	       ", pixel_format=v4l2_fourcc('%c', '\\%c', '\\%c', '\\x%x')})"
	       " = -1 EBADF (%m)\n", p_frmsizeenum->index,
	       u_frmsizeenum.cc[0], u_frmsizeenum.cc[1],
	       u_frmsizeenum.cc[2], u_frmsizeenum.cc[3]);
#endif /* VIDIOC_ENUM_FRAMESIZES */

#ifdef VIDIOC_ENUM_FRAMEINTERVALS
	ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, 0);
	printf("ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, page);
	printf("ioctl(-1, VIDIOC_ENUM_FRAMEINTERVALS, {index=%u"
	       ", pixel_format=v4l2_fourcc('\\x%x', '\\x%x', '\\x%x', '\\x%x')"
	       ", width=%u, height=%u}) = -1 EBADF (%m)\n",
	       magic, u.cc[0], u.cc[1], u.cc[2], u.cc[3], magic, magic);
#endif /* VIDIOC_ENUM_FRAMEINTERVALS */

#ifdef VIDIOC_CREATE_BUFS
	ioctl(-1, VIDIOC_CREATE_BUFS, 0);
	printf("ioctl(-1, VIDIOC_CREATE_BUFS, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, VIDIOC_CREATE_BUFS,
	      page + size - sizeof(struct v4l2_create_buffers));
	printf("ioctl(-1, VIDIOC_CREATE_BUFS, {count=%u, memory=%#x"
	       " /* V4L2_MEMORY_??? */, format={type=%#x"
	       " /* V4L2_BUF_TYPE_??? */}}) = -1 EBADF (%m)\n",
	       magic, magic, magic);
#endif /* VIDIOC_CREATE_BUFS */

	puts("+++ exited with 0 +++");
	return 0;
}
