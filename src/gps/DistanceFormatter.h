#pragma once
#include <SIMDString.h>

namespace gps {
class DistanceFormatter {
public:
    static SIMDString<64> FloatToString(float value, unsigned char precision = 2);
    static SIMDString<64> FormatDistance(float dist, bool useMetric);
};
}
