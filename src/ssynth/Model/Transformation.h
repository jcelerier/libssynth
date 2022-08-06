#pragma once

#include <ssynth/ColorPool.h>
#include <ssynth/Matrix4.h>
#include <ssynth/Model/State.h>
#include <ssynth/Vector3.h>

#include <QColor>
#include <QString>

namespace ssynth
{
namespace Model
{

class Transformation
{
public:
  Transformation();
  ~Transformation();

  /// 'Applies' the transformation 'T' to this transformation.
  /// (For the matrix this corresponds to matrix multiplication).
  void append(const Transformation& T);
  State apply(const State& s, ColorPool* colorPool) const;

  // The predefined operators
  // Translations
  static Transformation createX(double offset);
  static Transformation createY(double offset);
  static Transformation createZ(double offset);

  // Rotations
  static Transformation createRX(double angle);
  static Transformation createRY(double angle);
  static Transformation createRZ(double angle);

  // Plane reflection
  static Transformation createPlaneReflection(Math::Vector3f normal);

  // Scaling
  static Transformation createScale(double x, double y, double z);

  // Free transformation
  static Transformation createMatrix(const std::vector<double>& vals);

  // Color stuff
  static Transformation createHSV(float h, float s, float v, float a);
  static Transformation createColor(const QString& color);
  static Transformation createBlend(const QString& color, double strength);

private:
  // Matrix and Color transformations here.
  Math::Matrix4f matrix;

  // For color alterations
  float deltaH;
  float scaleS;
  float scaleV;
  float scaleAlpha;
  bool absoluteColor;

  // For color blends.
  QColor blendColor;
  double strength;
};

}
}
