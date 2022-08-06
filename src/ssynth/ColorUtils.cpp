#include <ssynth/ColorUtils.h>
#include <ssynth/Logging.h>

namespace ssynth::Misc
{

auto ColorUtils::HSVtoRGB(Math::Vector3f hsv) -> Math::Vector3f
{
  using namespace Math;
  using namespace Logging;
  /// Implementation based on: http://en.wikipedia.org/wiki/HSV_color_space
  if (hsv[0] >= 360)
    hsv[0] -= 360;
  int Hi = (int)(hsv[0] / 60) % 6;
  double f = (hsv[0] / 60) - Hi;
  double p = hsv[2] * (1 - hsv[1]);
  double q = hsv[2] * (1 - f * hsv[1]);
  double t = hsv[2] * (1 - (1 - f) * hsv[1]);
  if (Hi == 0)
    return {hsv[2], t, p};
  if (Hi == 1)
    return {q, hsv[2], p};
  if (Hi == 2)
    return {p, hsv[2], t};
  if (Hi == 3)
    return {p, q, hsv[2]};
  if (Hi == 4)
    return {t, p, hsv[2]};
  if (Hi == 5)
    return {hsv[2], p, q};
  WARNING("ColorUtils::HSVtoRGB failed");
  return {0, 0, 0};
}

}
