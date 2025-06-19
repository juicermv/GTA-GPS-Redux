#include "DistCache.h"

namespace util
{
    DistCache::DistCache()
    {
        cache.reserve(MAX_ITEMS);
        cache2D.reserve(MAX_ITEMS);
    }

    float DistCache::GetDist(const CVector &v1, const CVector &v2)
    {
        const CVectorPair the_pair = {v1, v2};
        auto it = cache.find(the_pair);
        if (it != cache.end())
        {
            return it->second;
        }

        if (cache.size() >= MAX_ITEMS)
        {
            cache.clear();
        }

        float out = DistanceBetweenPoints(v1, v2);
        cache.emplace(the_pair, out);
        return out;
    }

    float DistCache::GetDist2D(const CVector2D &v1, const CVector2D &v2)
    {
        const CVector2DPair the_pair = {v1, v2};
        auto it = cache2D.find(the_pair);
        if (it != cache2D.end())
        {
            return it->second;
        }

        if (cache2D.size() >= MAX_ITEMS)
        {
            cache2D.clear();
        }

        float out = DistanceBetweenPoints(v1, v2);
        cache2D.emplace(the_pair, out);
        return out;
    }
}