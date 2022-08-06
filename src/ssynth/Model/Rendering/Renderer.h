#pragma once

#include <ssynth/Matrix4.h>
#include <ssynth/Model/PrimitiveClass.h>
#include <ssynth/Vector3.h>

#include <QString>

namespace ssynth
{
namespace Model
{
namespace Rendering
{

// using namespace GLEngine;

/// Abstract base class for implementing a renderer
class Renderer
{
public:
  Renderer(){};
  virtual ~Renderer(){};

  /// Flow
  virtual void begin(){};
  virtual void end(){};

  /// This defines the identifier for our renderer.
  virtual QString renderClass() { return ""; }

  /// The primitives
  virtual void drawBox(
      Math::Vector3f base,
      Math::Vector3f dir1,
      Math::Vector3f dir2,
      Math::Vector3f dir3,
      PrimitiveClass* classID)
      = 0;

  virtual void drawMesh(
      Math::Vector3f startBase,
      Math::Vector3f startDir1,
      Math::Vector3f startDir2,
      Math::Vector3f endBase,
      Math::Vector3f endDir1,
      Math::Vector3f endDir2,
      PrimitiveClass* classID)
      = 0;

  virtual void drawGrid(
      Math::Vector3f base,
      Math::Vector3f dir1,
      Math::Vector3f dir2,
      Math::Vector3f dir3,
      PrimitiveClass* classID)
      = 0;

  virtual void drawLine(Math::Vector3f from, Math::Vector3f to, PrimitiveClass* classID)
      = 0;

  virtual void drawDot(Math::Vector3f pos, PrimitiveClass* classID) = 0;

  virtual void drawSphere(Math::Vector3f center, float radius, PrimitiveClass* classID)
      = 0;

  virtual void drawTriangle(
      Math::Vector3f p1,
      Math::Vector3f p2,
      Math::Vector3f p3,
      PrimitiveClass* classID)
      = 0;

  virtual void callGeneric(PrimitiveClass*){};

  // Color
  // RGB in [0;1] intervals.
  virtual void setColor(Math::Vector3f rgb) = 0;
  virtual void setBackgroundColor(Math::Vector3f rgb) = 0;
  virtual void setAlpha(double alpha) = 0;

  virtual void setPreviousColor(Math::Vector3f rgb) = 0;
  virtual void setPreviousAlpha(double alpha) = 0;

  // Camera settings
  virtual void setTranslation(Math::Vector3f /*translation*/){};
  virtual void setScale(double /*scale*/){};
  virtual void setRotation(Math::Matrix4f /*rotation*/){};
  virtual void setPivot(Math::Vector3f /*pivot*/){};
  virtual void setPerspectiveAngle(double /*angle*/){};

  // Issues a command for a specific renderclass such as 'template' or 'opengl'
  virtual void callCommand(const QString& /*renderClass*/, const QString& /*command*/){};
};

}
}
}
