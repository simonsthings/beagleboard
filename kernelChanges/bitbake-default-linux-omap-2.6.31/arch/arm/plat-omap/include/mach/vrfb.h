/*
 * VRFB Rotation Engine
 *
 * Copyright (C) 2009 Nokia Corporation
 * Author: Tomi Valkeinen <tomi.valkeinen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __OMAP_VRFB_H__
#define __OMAP_VRFB_H__

#define OMAP_VRFB_LINE_LEN 2048

struct vrfb {
	u8 context;
	void __iomem *vaddr[4];
	unsigned long paddr[4];
	u16 xoffset;
	u16 yoffset;
	u8 bytespp;
};

extern int omap_vrfb_request_ctx(struct vrfb *vrfb);
extern void omap_vrfb_release_ctx(struct vrfb *vrfb);
extern void omap_vrfb_suspend_ctx(struct vrfb *vrfb);
extern void omap_vrfb_resume_ctx(struct vrfb *vrfb);
extern void omap_vrfb_adjust_size(u16 *width, u16 *height,
		u8 bytespp);
extern void omap_vrfb_setup(struct vrfb *vrfb, unsigned long paddr,
		u16 width, u16 height,
		unsigned bytespp, bool yuv_mode);
extern void omap_vrfb_restore_context(void);

#endif /* __VRFB_H */
