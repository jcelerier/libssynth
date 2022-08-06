#pragma once

#include <QString>

namespace ssynth
{
namespace Model
{

/// Every PrimitiveRule can be assigned a class.

/// Every Primitive can be assigned a class.
struct PrimitiveClass
{
  PrimitiveClass()
      : reflection(0)
      , hasShadows(true)
      , castShadows(true)
      , ambient(0.6)
      , specular(0.6)
      , diffuse(0.6){};
  QString name;
  double reflection;
  bool hasShadows;  // not implemented yet
  bool castShadows; // not implemented yet
  double ambient;
  double specular;
  double diffuse;
};

}
}
