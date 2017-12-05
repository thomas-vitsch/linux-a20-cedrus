/*
 * Copyright (C) 2016 Free Electrons
 * Copyright (C) 2016 NextThing Co
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <drm/drmP.h>

#include "sun4i_tcon.h"
#include "sun4i_dotclock.h"


#define NS_JEIDA_LVDS_BITS_PER_PIX_CLK			7


struct sun4i_dclk {
	struct clk_hw		hw;
	struct regmap		*regmap;

	struct sun4i_tcon	*tcon;
};

static inline struct sun4i_dclk *hw_to_dclk(struct clk_hw *hw)
{
	return container_of(hw, struct sun4i_dclk, hw);
}

static void sun4i_dclk_disable(struct clk_hw *hw)
{
	struct sun4i_dclk *dclk = hw_to_dclk(hw);

	regmap_update_bits(dclk->regmap, SUN4I_TCON0_DCLK_REG,
			   BIT(SUN4I_TCON0_DCLK_GATE_BIT), 0);
}

static int sun4i_dclk_enable(struct clk_hw *hw)
{
	struct sun4i_dclk *dclk = hw_to_dclk(hw);

	return regmap_update_bits(dclk->regmap, SUN4I_TCON0_DCLK_REG,
				  BIT(SUN4I_TCON0_DCLK_GATE_BIT),
				  BIT(SUN4I_TCON0_DCLK_GATE_BIT));
}

static int sun4i_dclk_is_enabled(struct clk_hw *hw)
{
	struct sun4i_dclk *dclk = hw_to_dclk(hw);
	u32 val;

	regmap_read(dclk->regmap, SUN4I_TCON0_DCLK_REG, &val);

	return val & BIT(SUN4I_TCON0_DCLK_GATE_BIT);
}

static unsigned long sun4i_dclk_recalc_rate(struct clk_hw *hw,
    unsigned long parent_rate)
{
	struct sun4i_dclk *dclk = hw_to_dclk(hw);
	u32 val;

	regmap_read(dclk->regmap, SUN4I_TCON0_DCLK_REG, &val);

	val >>= SUN4I_TCON0_DCLK_DIV_SHIFT;
	val &= (1 << SUN4I_TCON0_DCLK_DIV_WIDTH) - 1;

	if (!val)
		val = 1;

	DRM_DEBUG_DRIVER("%s:%d return parent_rate(%lu) / div(%u) = %lu\n",
	    __FUNCTION__, __LINE__, parent_rate, val,
	    parent_rate / val);

	return parent_rate / val;
}

static long sun4i_dclk_round_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long *parent_rate)
{
	unsigned long best_parent;
	struct sun4i_dclk *dclk;
	int i, min_div, max_div;
	unsigned long rounded;
	unsigned long ideal;
	u8 best_div;

	dclk = hw_to_dclk(hw);
	best_parent = 0;
	best_div = 1;

	DRM_DEBUG_DRIVER("connector type is %d\n",
	    dclk->tcon->connector->connector_type);

	/*
	 * When generating LVDS output, dclk must always be 7* the LVDS
	 * signaling clock rate.
	 */
	if (dclk->tcon->connector->connector_type == DRM_MODE_CONNECTOR_LVDS) {
		min_div = NS_JEIDA_LVDS_BITS_PER_PIX_CLK;
		max_div = NS_JEIDA_LVDS_BITS_PER_PIX_CLK;
	} else {
		min_div = DCLK_DIV_MIN;
		max_div = DCLK_DIV_MAX;
	}

	for (i = min_div; i <= max_div; i++) {
		ideal = rate * i;

		rounded = clk_hw_round_rate(clk_hw_get_parent(hw),
		    ideal);

		if (rounded == ideal) {
			best_parent = rounded;
			best_div = i;
			goto out;
		}

		if (abs(rate - rounded / i) <
		    abs(rate - best_parent / best_div)) {
			best_parent = rounded;
			best_div = i;
		}
	}

out:
	DRM_DEBUG_DRIVER("%s:%d return best_parent(%lu) / best_div (%u) = %lu\n",
	    __FUNCTION__, __LINE__, best_parent, best_div,
	    best_parent / best_div);

	*parent_rate = best_parent;

	return best_parent / best_div;
}

static int sun4i_dclk_set_rate(struct clk_hw *hw, unsigned long rate,
			       unsigned long parent_rate)
{
	struct sun4i_dclk *dclk = hw_to_dclk(hw);
	u8 div = parent_rate / rate;

	printk("%s:%d rate = %ld, parent_rate = %ld\n", __FUNCTION__, __LINE__,
	    rate, parent_rate);

	return regmap_update_bits(dclk->regmap, SUN4I_TCON0_DCLK_REG,
				  GENMASK(6, 0), div);
}

static int sun4i_dclk_get_phase(struct clk_hw *hw)
{
	struct sun4i_dclk *dclk = hw_to_dclk(hw);
	u32 val;

	regmap_read(dclk->regmap, SUN4I_TCON0_IO_POL_REG, &val);

	val >>= 28;
	val &= 3;

	return val * 120;
}

static int sun4i_dclk_set_phase(struct clk_hw *hw, int degrees)
{
	struct sun4i_dclk *dclk = hw_to_dclk(hw);

	regmap_update_bits(dclk->regmap, SUN4I_TCON0_IO_POL_REG,
			   GENMASK(29, 28),
			   degrees / 120);

	return 0;
}

static const struct clk_ops sun4i_dclk_ops = {
	.disable	= sun4i_dclk_disable,
	.enable		= sun4i_dclk_enable,
	.is_enabled	= sun4i_dclk_is_enabled,

	.recalc_rate	= sun4i_dclk_recalc_rate,
	.round_rate	= sun4i_dclk_round_rate,
	.set_rate	= sun4i_dclk_set_rate,

	.get_phase	= sun4i_dclk_get_phase,
	.set_phase	= sun4i_dclk_set_phase,
};

int sun4i_dclk_create(struct device *dev, struct sun4i_tcon *tcon)
{
	const char *clk_name, *parent_name;
	struct clk_init_data init;
	struct sun4i_dclk *dclk;
	int ret;

	parent_name = __clk_get_name(tcon->sclk0);
	ret = of_property_read_string_index(dev->of_node,
					    "clock-output-names", 0,
					    &clk_name);
	if (ret)
		return ret;

	dclk = devm_kzalloc(dev, sizeof(*dclk), GFP_KERNEL);
	if (!dclk)
		return -ENOMEM;

	init.name = clk_name;
	init.ops = &sun4i_dclk_ops;
	init.parent_names = &parent_name;
	init.num_parents = 1;
	init.flags = CLK_SET_RATE_PARENT;

	dclk->regmap = tcon->regs;
	dclk->hw.init = &init;

	tcon->dclk = clk_register(dev, &dclk->hw);
	if (IS_ERR(tcon->dclk))
		return PTR_ERR(tcon->dclk);

	/*
	 * DAAN: Make sure we have a reference back to the tcon controller we
	 * belong to.
	 */
	dclk->tcon = tcon;

	return 0;
}
EXPORT_SYMBOL(sun4i_dclk_create);

int sun4i_dclk_free(struct sun4i_tcon *tcon)
{
	clk_unregister(tcon->dclk);
	return 0;
}
EXPORT_SYMBOL(sun4i_dclk_free);
