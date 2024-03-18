#pragma once
#include <CVector.h>
#include <algorithm>
#include <map>

class DistCache {
 private:
  struct CVectorPair {
    CVector v1;
    CVector v2;

    bool operator<(const CVectorPair& a_pair) const noexcept {
      CVector sum1 = v1 + v2;
      CVector sum2 = a_pair.v1 + a_pair.v2;

      return sum1.Magnitude() < sum2.Magnitude();
    };
  };

  std::map<CVectorPair, float> cache{};

 public:
  inline float GetDist(const CVector& v1, const CVector& v2) {
    CVectorPair the_pair = {v1, v2};
    if (cache.find(the_pair) != cache.end()) {
      return cache[the_pair];
    } else {
      if (cache.size() >= 10) {
        while (cache.size() >= 10) {
          cache.erase(cache.begin());
        }
      }

      float out = DistanceBetweenPoints(v1, v2);
      cache[the_pair] = out;
      return out;
    }
  }
} distCache;