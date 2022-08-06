#pragma once

#include <ssynth/Model/Rendering/Renderer.h>
#include <ssynth/Vector3.h>

#include <QDomDocument>
#include <QFile>
#include <QString>
#include <QStringList>

#include <map>
#include <set>

namespace ssynth
{
namespace Model
{
namespace Rendering
{

using namespace Math;
// using namespace GLEngine;

/// A TemplatePrimitive is the definition for a single primitive (like Box or Sphere).
/// It is a simple text string with placeholders for stuff like coordinates and color.
class TemplatePrimitive
{
public:
  TemplatePrimitive() = default;
  TemplatePrimitive(QString def)
      : def(std::move(def)){};
  TemplatePrimitive(const TemplatePrimitive& t) { this->def = t.def; };

  const QString& getText() { return def; }

  void substitute(const QString& before, const QString& after)
  {
    def.replace(before, after);
  };
  void substitute(const QString& before, const std::string& after)
  {
    def.replace(before, QLatin1String(after.data(), after.size()));
  };

  bool contains(const QString& input) { return def.contains(input); };

private:
  QString def;
};

// A Template contains a number of TemplatePrimitives:
// text definitions for each of the standard primitives (box, sphere, ...) with placeholders
// for stuff like coordinates and color.
class Template
{
public:
  Template() = default;
  Template(const Template&) = default;
  Template(Template&&) noexcept = default;
  Template& operator=(const Template&) = default;
  Template& operator=(Template&&) noexcept = default;

  Template(QFile& file) { read(file); }
  Template(QString xmlString) { read(xmlString); }

  void read(QFile& file);
  void read(const QString& xmlString);
  void parse(QDomDocument& doc);

  std::map<QString, TemplatePrimitive>& getPrimitives() { return primitives; }
  TemplatePrimitive get(const QString& name) { return primitives[name]; }
  QString getDescription() { return description; }
  QString getFullText() { return fullText; }
  QString getName() { return name; }
  QString getDefaultExtension() { return defaultExtension; }
  QString getRunAfter() { return runAfter; }

private:
  std::map<QString, TemplatePrimitive> primitives;
  QString description;
  QString name;
  QString defaultExtension;
  QString fullText;
  QString runAfter;
};

/// A renderer implementation based on the SyntopiaCore POV widget.
class TemplateRenderer : public Renderer
{
public:
  TemplateRenderer();
  TemplateRenderer(const QString& xmlDefinitionFile);
  TemplateRenderer(Template myTemplate);

  virtual ~TemplateRenderer();

  virtual QString renderClass() { return "template"; }

  /// The primitives
  virtual void drawBox(
      Math::Vector3f base,
      Math::Vector3f dir1,
      Math::Vector3f dir2,
      Math::Vector3f dir3,
      PrimitiveClass* classID);

  virtual void drawSphere(Math::Vector3f center, float radius, PrimitiveClass* classID);

  virtual void drawMesh(
      Math::Vector3f startBase,
      Math::Vector3f startDir1,
      Math::Vector3f startDir2,
      Math::Vector3f endBase,
      Math::Vector3f endDir1,
      Math::Vector3f endDir2,
      PrimitiveClass* classID);

  virtual void drawGrid(
      Math::Vector3f base,
      Math::Vector3f dir1,
      Math::Vector3f dir2,
      Math::Vector3f dir3,
      PrimitiveClass* classID);

  virtual void drawLine(Math::Vector3f from, Math::Vector3f to, PrimitiveClass* classID);

  virtual void drawDot(Math::Vector3f pos, PrimitiveClass* classID);

  virtual void drawTriangle(
      Math::Vector3f p1,
      Math::Vector3f p2,
      Math::Vector3f p3,
      PrimitiveClass* classID);

  virtual void callGeneric(PrimitiveClass* classID);

  virtual void begin();
  virtual void end();

  virtual void setColor(Math::Vector3f rgb) { this->rgb = rgb; }
  virtual void setBackgroundColor(Math::Vector3f rgb);
  virtual void setAlpha(double alpha) { this->alpha = alpha; }

  virtual void setPreviousColor(Math::Vector3f rgb) { this->oldRgb = rgb; }
  virtual void setPreviousAlpha(double alpha) { this->oldAlpha = alpha; }

  QString getOutput();

  // Issues a command for a specific renderclass such as 'template' or 'opengl'
  virtual void callCommand(const QString& renderClass, const QString& command);

  bool assertPrimitiveExists(const QString& templateName);

  void setCamera(
      Vector3f cameraPosition,
      Vector3f cameraUp,
      Vector3f cameraRight,
      Vector3f cameraTarget,
      int width,
      int height,
      double aspect,
      double fov)
  {
    this->cameraPosition = cameraPosition;
    this->cameraUp = cameraUp;
    this->cameraRight = cameraRight;
    this->cameraTarget = cameraTarget;
    this->width = width;
    this->height = height;
    this->aspect = aspect;
    this->fov = fov;
  }

  void doBeginEndSubstitutions(TemplatePrimitive& t);

  void doStandardSubstitutions(
      Math::Vector3f base,
      Math::Vector3f dir1,
      Math::Vector3f dir2,
      Math::Vector3f dir3,
      TemplatePrimitive& t);

private:
  Math::Vector3f cameraPosition;
  Math::Vector3f cameraUp;
  Math::Vector3f cameraRight;
  Math::Vector3f cameraTarget;

  Math::Vector3f rgb;
  Math::Vector3f backRgb;
  double alpha{};
  Template workingTemplate;
  QStringList output;
  int counter;
  int width{};
  int height{};
  double aspect{};
  double fov{};
  std::set<QString> missingTypes;
  Math::Vector3f oldRgb;
  double oldAlpha{};
};

}
}
}
