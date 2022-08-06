#pragma once

#include <cmath>
#include <ssynth/Vector3.h>

#include <QString>
#include <QStringList>

#include <random>
#include <vector>

namespace ssynth
{
namespace Math
{

/// A simple class for generating random numbers
/// It is possible to have multiple independent streams, if the underlying RNG is the Mersenne Twister.
/// If set to useStdLib, the CStdLib 'rand' and 'srand' functions are used - these are not independent - not even with multiple instances of this class.
class RandomNumberGenerator
{
public:
  RandomNumberGenerator() { setSeed(0); };
  ~RandomNumberGenerator(){};

  // This is only useful for backward compatibility.
  // The Mersenne Twister is much better since it allows multiple independent streams.
  // void useStdLib(bool useOldLibrary) {
  //     setSeed(lastSeed);
  // };
  // bool isUsingStdLib() { return (rng == 0); }

  std::vector<int> getRandomIndices(int count);

  // Returns a vector, where the elements are ranked randomly.
  template <typename T>
  std::vector<T> randomize(const std::vector<T>& list)
  {
    std::vector<int> indices = getRandomIndices(list.size());
    std::vector<T> copy(list.size());
    for (int i = 0; i < list.size(); i++)
      copy[i] = list[indices[i]];
    return copy;
  }

  // Returns a double in the interval [0;1]
  double getDouble() { return std::uniform_real_distribution<double>(0., 1.)(rng); };

  // Normal distributed number with mean zero.
  double getNormal(double variance)
  {
    /// FIXME: normal_distribution takes standard deviation, not variance
    return std::normal_distribution<double>{0., variance}(rng);
  }

  double getDouble(double min, double max) { return getDouble() * (max - min) + min; }

  // Returns an integer between 0 and max (both inclusive).
  int getInt(int max) { return std::uniform_int_distribution<int>(0, max)(rng); }

  int getInt() { return rng(); }

  void setSeed(int seed)
  {
    lastSeed = seed;
    rng.seed(seed);
  };

private:
  int lastSeed;
  std::mt19937 rng;
};

}
}
