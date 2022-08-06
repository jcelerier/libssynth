#pragma once

#include <ssynth/Random.h>

namespace ssynth
{
namespace Model
{

/// These two independent random number generator streams are used in Structure Synth
class RandomStreams
{
public:
  static Math::RandomNumberGenerator* Geometry() { return geometry; }
  static Math::RandomNumberGenerator* Color() { return color; }
  static void SetSeed(int seed)
  {
    geometry->setSeed(seed);
    color->setSeed(seed);
  }
  // static void UseOldRandomGenerators(bool useOld) { geometry->useStdLib(useOld); color->useStdLib(useOld); }
private:
  static Math::RandomNumberGenerator* geometry;
  static Math::RandomNumberGenerator* color;
};

}
}
