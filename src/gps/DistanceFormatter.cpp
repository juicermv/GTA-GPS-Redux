#include "gps/DistanceFormatter.h"
#include <iomanip>
#include <sstream>

namespace gps {
namespace {
    constexpr float MetersToYards(float m) { return m * 1.094f; }
}

SIMDString<64> DistanceFormatter::FloatToString(float in, unsigned char precision) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision) << in;
    return SIMDString<64>(stream.str());
}

SIMDString<64> DistanceFormatter::FormatDistance(float dist, bool useMetric) {
    switch (useMetric) {
        case 0:
            if (dist > 999) {
                return FloatToString(dist / 1000, 1) + " KM";
            } else {
                return FloatToString(dist, 0) + " m";
            }
        default:
            dist = MetersToYards(dist);
            if (dist > 599) {
                return FloatToString(dist / 1760, 1) + " Mi";
            } else {
                return FloatToString(dist, 0) + " yrds";
            }
    }
}
} // namespace gps
