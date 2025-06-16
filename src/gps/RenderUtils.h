#pragma once
#include <CRGBA.h>
#include <RenderWare.h>
#include "util/Config.h"

namespace gps {
class RenderUtils {
public:
    static CRGBA SetupColor(short color, bool friendly, const util::Config& cfg);
    static void Setup2dVertex(RwIm2DVertex& vertex, double x, double y, CRGBA& clr);
};
}
