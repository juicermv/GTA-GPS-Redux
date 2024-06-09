#pragma once
#include <CVector.h>
#include <CVector2D.h>
#include <algorithm>

#include <map>

class DistCache
{
  private:
	struct CVectorPair
	{
		CVector v1;
		CVector v2;

		bool operator<(const CVectorPair &a_pair) const noexcept
		{
			CVector sum1 = v1 + v2;
			CVector sum2 = a_pair.v1 + a_pair.v2;

			return sum1.Magnitude() < sum2.Magnitude();
		};
	};

	std::map<CVectorPair, float> cache{};

  public:
	inline float GetDist(const CVector &v1, const CVector &v2)
	{
		const CVectorPair the_pair = {v1, v2};
		if (cache.find(the_pair) != cache.end())
		{
			return cache[the_pair];
		}
		else
		{
			while (cache.size() >= 10)
			{
				cache.erase(cache.begin());
			}

			float out = DistanceBetweenPoints(v1, v2);
			cache[the_pair] = out;
			return out;
		}
	}

	inline float GetDist2D(const CVector &v1, const CVector &v2)
	{
		const CVector v1_2D = CVector(v1.x, 0, v1.z);
		const CVector v2_2D = CVector(v2.x, 0, v2.z);
		return GetDist(v1_2D, v2_2D);
	}

	inline float GetDist2D(const CVector2D &v1, const CVector2D &v2)
	{
		const CVector v1_2D = CVector(v1.x, 0, v1.y);
		const CVector v2_2D = CVector(v2.x, 0, v2.y);
		return GetDist(v1_2D, v2_2D);
	}
} distCache;