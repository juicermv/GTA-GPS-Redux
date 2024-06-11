#pragma once
#include <CVector.h>
#include <CVector2D.h>
#include <algorithm>

#include <unordered_map>

#define MAX_ITEMS 10

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

	bool operator==(const CVectorPair &comp) const
	{
		return (comp.v1 + comp.v2).Magnitude() == (v1 + v2).Magnitude();
	}
};

struct CVector2DPair
{
	CVector2D v1;
	CVector2D v2;

	bool operator<(const CVector2DPair &a_pair) const noexcept
	{
		CVector2D sum1 = CVector2D(v1.x + v2.x, v1.y + v1.y);
		CVector2D sum2 = CVector2D(a_pair.v1.x + a_pair.v2.x, a_pair.v2.y + a_pair.v2.y);

		return sum1.Magnitude() < sum2.Magnitude();
	};

	bool operator==(const CVector2DPair &comp) const
	{
		return CVector2D(comp.v1.x + comp.v2.x, comp.v1.y + comp.v2.y).Magnitude() ==
			   CVector2D(v1.x + v2.x, v1.y + v2.y).Magnitude();
	}
};

namespace std
{
template <> class hash<CVectorPair>
{
  public:
	std::size_t operator()(const CVectorPair &pair) const
	{
		return (size_t)((size_t)pair.v1.MagnitudeSqr2D() ^
						(size_t)((pair.v1 + pair.v2).Magnitude()) - (size_t)pair.v2.MagnitudeSqr2D());
	}
};

template <> class hash<CVector2DPair>
{
  public:
	std::size_t operator()(const CVector2DPair &pair) const
	{
		return (size_t)((size_t)pair.v1.MagnitudeSqr() ^
						(size_t)CVector2D(pair.v1.x + pair.v2.x, pair.v1.y + pair.v2.y).Magnitude()) -
			   (size_t)pair.v2.MagnitudeSqr();
	}
};

} // namespace std

class DistCache
{
  private:
	std::unordered_map<CVectorPair, float> cache{};
	std::unordered_map<CVector2DPair, float> cache2D{};

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
			while (cache.size() >= MAX_ITEMS)
			{
				cache.erase(cache.begin());
			}

			float out = DistanceBetweenPoints(v1, v2);
			cache[the_pair] = out;
			return out;
		}
	}

	inline float GetDist2D(const CVector2D &v1, const CVector2D &v2)
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
				cache2D.erase(cache2D.begin());
			}

			float out = DistanceBetweenPoints(v1, v2);
			cache2D[the_pair] = out;
			return out;
		}
	}
} distCache;