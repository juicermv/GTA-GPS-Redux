#pragma once
#include <unordered_map>

#include "CVector2D.h"
#include "CVector.h"

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
	template <>
	class hash<CVectorPair>
	{
	public:
		std::size_t operator()(const CVectorPair &pair) const
		{
			return (size_t)((size_t)pair.v1.MagnitudeSqr2D() ^
							(size_t)((pair.v1 + pair.v2).Magnitude()) - (size_t)pair.v2.MagnitudeSqr2D());
		}
	};

	template <>
	class hash<CVector2DPair>
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

namespace util
{
	class DistCache
	{
	private:
		std::unordered_map<CVectorPair, float> cache{};
		std::unordered_map<CVector2DPair, float> cache2D{};

	public:
		float GetDist(const CVector &v1, const CVector &v2);
		float GetDist2D(const CVector2D &v1, const CVector2D &v2);

		/*
		inline float GetDist2D(const CVector &v1, const CVector &v2)
		{
			return GetDist2D(CVector2D(v1.x, v1.z), CVector2D(v2.x, v2.z));
		}
		*/
	};
}