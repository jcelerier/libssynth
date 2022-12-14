#pragma once

#include <QColor>
#include <QImage>
#include <QString>

#include <vector>

namespace ssynth
{
namespace Model
{

/// A ColorPool is a set or colors. It is used for drawing random colors using the 'color random' operator.
/// The Builder maintainse single color pool in a project.
class ColorPool
{
  enum PoolType
  {
    RandomHue,
    GreyScale,
    RandomRGB,
    Picture,
    ColorList
  };

public:
  ColorPool(QString initString);
  ~ColorPool();
  QColor drawColor(); // returns a random color from the pool (in HSV)
private:
  PoolType type;
  std::vector<QColor> colorList; // only used by type: ColorList.
  QImage* picture;               // Only used by type: Picture.
};

}
}
