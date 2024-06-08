#include "Config.h"
#include "DistCache.h"
#include <CRGBA.h>

CRGBA SetupColor(short color, bool friendly, Config cfg)
{
	CRGBA clr;
	if (cfg.ENABLE_CUSTOM_CLRS)
	{
		switch (color)
		{
		case 0: // RED
			clr = cfg.CC_RED;
			break;
		case 1: // GREEN
			clr = cfg.CC_GREEN;
			break;
		case 2: // BLUE
			clr = cfg.CC_BLUE;
			break;
		case 3: // WHITE
			clr = cfg.CC_WHITE;
			break;
		case 4: // YELLOW
			clr = cfg.CC_YELLOW;
			break;
		case 5: // PURPLE
			clr = cfg.CC_PURPLE;
			break;
		case 6: // CYAN
			clr = cfg.CC_CYAN;
			break;
		case 7: // Depends on whether blip is friendly.
			if (friendly)
			{
				// BLUE
				clr = cfg.CC_BLUE;
			}
			else
			{
				// RED
				clr = cfg.CC_RED;
			}
			break;
		case 8: // DESTINATION
			clr = cfg.CC_YELLOW;
			break;
		}
	}
	else
		clr = CRadar::GetRadarTraceColour(color, 1, friendly);

	if (color < 1 || color > 8)
	{
		clr = cfg.GPS_LINE_CLR;
	}

	return clr;
}

constexpr void Setup2dVertex(RwIm2DVertex &vertex, double x, double y, CRGBA &clr)
{
	vertex.x = x;
	vertex.y = y;
	vertex.u = vertex.v = 0.0f;
	vertex.z = CSprite2d::NearScreenZ + 0.0001f;
	vertex.rhw = CSprite2d::RecipNearClip;

	vertex.emissiveColor = RWRGBALONG(clr.r, clr.g, clr.b, clr.a);
}

// Meters to yards.
constexpr float mtoyard(float m)
{
	return m * 1.094f;
}

std::string Float2String(float in, unsigned char precision = 2)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << in;
	return stream.str();
}

std::string makeDist(float dist, bool units)
{
	// 1 Unit of distance = 1 meter.
	switch (units)
	{
	case 0:
		if (dist > 999)
		{
			return Float2String(dist / 1000, 1) + " KM";
		}
		else
		{
			return Float2String(dist, 0) + " m";
		}
		break;
	case 1:
		dist = mtoyard(dist);
		if (dist > 599)
		{
			return Float2String(dist / 1760, 1) + " Mi";
		}
		else
		{
			return Float2String(dist, 0) + " yrds";
		}
		break;
	}
}

/*
const char *VectorToString(std::vector<tRadarTrace> &vec, CPed *player)
{
	char *result = "";
	for (unsigned short i = 0; i < (unsigned short)vec.size() - 1; i++)
	{
		char buffer[3] = "";
		sprintf(buffer, "%d", (int)vec.at(i).m_nRadarSprite);
		strcat(result, buffer);
		strcat(result, ", ");

		char f_buffer[10] = "";
		sprintf(f_buffer, "%f", distCache.GetDist(player->GetPosition(), vec.at(i).m_vecPos));
		strcat(result, f_buffer);
		strcat(buffer, "\n\t");
	}
	return result;
}
*/