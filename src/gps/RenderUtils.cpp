#include "gps/RenderUtils.h"
#include <CRadar.h>
#include <CSprite2d.h>
#include <plugin.h>

namespace gps {

CRGBA RenderUtils::SetupColor(short color, bool friendly, const util::Config& cfg) {
    CRGBA clr;
    if (cfg.ENABLE_CUSTOM_CLRS) {
        switch (color) {
        case 0: clr = cfg.CC_RED; break;
        case 1: clr = cfg.CC_GREEN; break;
        case 2: clr = cfg.CC_BLUE; break;
        case 3: clr = cfg.CC_WHITE; break;
        case 4: clr = cfg.CC_YELLOW; break;
        case 5: clr = cfg.CC_PURPLE; break;
        case 6: clr = cfg.CC_CYAN; break;
        case 7:
            clr = friendly ? cfg.CC_BLUE : cfg.CC_RED;
            break;
        case 8:
            clr = cfg.CC_YELLOW;
            break;
        }
    } else {
        clr = CRadar::GetRadarTraceColour(color, 1, friendly);
    }

    if (color < 1 || color > 8) {
        clr = cfg.GPS_LINE_CLR;
    }

    return clr;
}

void RenderUtils::Setup2dVertex(RwIm2DVertex& vertex, double x, double y, CRGBA& clr) {
    vertex.x = x;
    vertex.y = y;
    vertex.u = vertex.v = 0.0f;
    vertex.z = CSprite2d::NearScreenZ + 0.0001f;
    vertex.rhw = CSprite2d::RecipNearClip;
    vertex.emissiveColor = RWRGBALONG(clr.r, clr.g, clr.b, clr.a);
}

} // namespace gps
