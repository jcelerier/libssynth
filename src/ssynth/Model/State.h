#pragma once

#include <ssynth/Matrix4.h>

#include <QString>

#include <map>

namespace ssynth
{
namespace Model
{

class Rule; // Forward

// A slight trimmed version of a State.
struct PreviousState
{
  Math::Matrix4f matrix; // Transformation matrix (4x4 homogenous representation)
  Math::Vector3f hsv;    // Hue, Saturation, Value colorspace state
  float alpha;           // Transparency
};

/// A state represent the current rendering projection matrix and other rendering settings.
struct State
{
  State();
  State(const State& rhs);
  ~State();

  State& operator=(const State& rhs);

  void setPreviousState(Math::Matrix4f matrix, Math::Vector3f hsv, float alpha);

  Math::Matrix4f matrix; // Transformation matrix (4x4 homogenous representation)
  Math::Vector3f hsv;    // Hue, Saturation, Value colorspace state
  float alpha;           // Transparency
  std::map<const Rule*, int>
      maxDepths; // Rules may have a max. recursion depth before they are retired.
                 // We need to keep track of this in the state.
  PreviousState* previous;
  int seed;
};

}
}
