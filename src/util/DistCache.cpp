#include "DistCache.h"

namespace util
{
    float DistCache::GetDist(const CVector &v1, const CVector &v2)
    {
        const CVectorPair the_pair = {v1, v2};
        if (cache.find(the_pair) != cache.end())
        {
            return cache[the_pair];
        }
        else
        {
            while (cache.size() >= MAX_ITEMS)
            {
                cache.clear();
            }

            float out = DistanceBetweenPoints(v1, v2);
            cache[the_pair] = out;
            return out;
        }
    }

    float DistCache::GetDist2D(const CVector2D &v1, const CVector2D &v2)
    {
        const CVector2DPair the_pair = {v1, v2};
        if (cache2D.find(the_pair) != cache2D.end())
        {
            return cache2D[the_pair];
        }
        else
        {
            while (cache2D.size() >= MAX_ITEMS)
            {
                cache2D.clear();
            }

            float out = DistanceBetweenPoints(v1, v2);
            cache2D[the_pair] = out;
            return out;
        }
    }
}