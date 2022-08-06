#include <ssynth/RandomStreams.h>

namespace ssynth::Model
{
Math::RandomNumberGenerator* RandomStreams::geometry = new Math::RandomNumberGenerator();
Math::RandomNumberGenerator* RandomStreams::color = new Math::RandomNumberGenerator();

}
