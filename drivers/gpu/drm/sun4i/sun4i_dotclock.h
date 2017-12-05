/*
 * Copyright (C) 2015 Free Electrons
 * Copyright (C) 2015 NextThing Co
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef _SUN4I_DOTCLOCK_H_
#define _SUN4I_DOTCLOCK_H_

/* The TCON has the ability to further divide the pixel clock rate. In LVDS
 * NS/JEIDA mode, 7 bits of pixel data are sent per pixel clock.
 * Alternatively for other connectors other rates per pixel clock could be valid
 * and can range from DCLK_DIV_MIN to DCLK_DIV_MAX.
 */
#define DCLK_DIV_MIN			6
#define DCLK_DIV_MAX			127
#define DCLK_NS_JEIDA_LVDS_DIV		7

struct sun4i_tcon;

int sun4i_dclk_create(struct device *dev, struct sun4i_tcon *tcon);
int sun4i_dclk_free(struct sun4i_tcon *tcon);

#endif /* _SUN4I_DOTCLOCK_H_ */
