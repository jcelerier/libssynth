#pragma once

#include <cmath>
#include <ssynth/Vector3.h>

#include <QString>

#include <vector>

namespace ssynth
{
namespace Misc
{

class ColorUtils
{
public:
  static Math::Vector3f HSVtoRGB(Math::Vector3f hsv);
};

}
}
