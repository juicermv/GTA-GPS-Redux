#pragma once
#include <cmath>
#include "Config.h"

namespace util
{
	// Deterministically derives a distinct, readable color for a blip
	// color index outside vanilla's 0-8 range, so a third-party mod's own
	// blip colors still look reasonable even if nobody ever configures
	// them. The 6 vanilla-assigned colors (red/yellow/green/cyan/blue/
	// purple) sit exactly 60 degrees apart on the hue wheel, so hues are
	// anchored at the midpoints between them (30, 90, ..., 330) via
	// index % 6 - a generated color is then never near a named/vanilla
	// one. Saturation is varied too (via a different irrational constant,
	// keyed off how many times that hue bucket has repeated) so indices
	// that land in the same bucket are still distinct from each other.
	inline CRGBA GeneratedColorForIndex(unsigned int index)
	{
		constexpr double GAP_HUES[6] = {30.0, 90.0, 150.0, 210.0, 270.0, 330.0};
		double hueDeg = GAP_HUES[index % 6];

		constexpr double SQRT2_MINUS_1 = 0.4142135623730951;
		double saturation = 0.55 + 0.4 * std::fmod((index / 6 + 1) * SQRT2_MINUS_1, 1.0);
		constexpr double value = 0.95;

		double hue = hueDeg / 60.0;
		int hi = static_cast<int>(hue);
		double f = hue - hi;
		double p = value * (1.0 - saturation);
		double q = value * (1.0 - saturation * f);
		double t = value * (1.0 - saturation * (1.0 - f));

		double r, g, b;
		switch (hi % 6)
		{
		case 0: r = value; g = t; b = p; break;
		case 1: r = q; g = value; b = p; break;
		case 2: r = p; g = value; b = t; break;
		case 3: r = p; g = q; b = value; break;
		case 4: r = t; g = p; b = value; break;
		default: r = value; g = p; b = q; break;
		}

		return CRGBA(static_cast<unsigned char>(r * 255.0), static_cast<unsigned char>(g * 255.0),
					static_cast<unsigned char>(b * 255.0), 255);
	}

	inline CRGBA SetupColor(short color, bool friendly, const struct Config& cfg)
	{
		if (color < 0)
			return cfg.GPS_LINE_CLR;

		if (color > 8)
		{
			// Outside vanilla's 0-8 range entirely - most likely a mod's
			// own blip color. An explicit "colorN=" override (only
			// possible when custom colors are enabled) always wins;
			// otherwise fall back to a generated distinct color, so this
			// works out of the box even if custom colors were never
			// configured at all.
			if (cfg.ENABLE_CUSTOM_CLRS)
			{
				auto it = cfg.CUSTOM_COLORS.find(static_cast<unsigned int>(color));
				if (it != cfg.CUSTOM_COLORS.end())
					return it->second;
			}
			return GeneratedColorForIndex(static_cast<unsigned int>(color));
		}

		if (cfg.ENABLE_CUSTOM_CLRS)
		{
			unsigned int lookupColor = static_cast<unsigned int>(color);
			if (color == 7) // THREAT: blue if friendly, red otherwise
				lookupColor = friendly ? 2u : 0u;
			else if (color == 8 && !cfg.CUSTOM_COLORS.contains(8)) // DESTINATION defaults to yellow
				lookupColor = 4u;

			auto it = cfg.CUSTOM_COLORS.find(lookupColor);
			if (it != cfg.CUSTOM_COLORS.end())
				return it->second;

			return cfg.GPS_LINE_CLR;
		}

		return CRadar::GetRadarTraceColour(color, 1, friendly);
	}

	constexpr void Setup2dVertex(RwIm2DVertex &vertex, const double x, const double y, const CRGBA &clr)
	{
		vertex.x = x;
		vertex.y = y;
		vertex.u = vertex.v = 0.0f;
		vertex.z = CSprite2d::NearScreenZ + 0.0001f;
		vertex.rhw = CSprite2d::RecipNearClip;

		vertex.emissiveColor = RWRGBALONG(clr.r, clr.g, clr.b, clr.a);
	}
} // namespace util