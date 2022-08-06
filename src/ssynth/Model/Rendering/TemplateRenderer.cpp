#include <ssynth/Exception.h>
#include <ssynth/Logging.h>
#include <ssynth/Model/PrimitiveClass.h>
#include <ssynth/Model/Rendering/TemplateRenderer.h>
#include <ssynth/Vector3.h>

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>

#define FMT_HEADER_ONLY 1
#include <fmt/format.h>

#include <map>

namespace ssynth
{
using namespace Math;
using namespace Logging;
using namespace Exceptions;

namespace Model::Rendering
{

static QString getAlternateId(PrimitiveClass* classID)
{
  if (!(classID->name).isEmpty())
    return "::" + (classID->name);
  else
    return QString{};
}

void Template::read(const QString& xml)
{
  QDomDocument doc;

  QString errorMessage;
  int errorLine = 0;
  int errorColumn = 0;

  if (!doc.setContent(xml, false, &errorMessage, &errorLine, &errorColumn))
  {
    QString error = QString("[Line %1, Col %2] %3")
                        .arg(errorLine)
                        .arg(errorColumn)
                        .arg(errorMessage);
    WARNING("Unable to parse xml: " + error);

    throw Exception("Unable to parse xml from string: " + error);

    return;
  }

  fullText = doc.toString();

  parse(doc);
}

void Template::read(QFile& file)
{
  QDomDocument doc;
  if (!file.open(QIODevice::ReadOnly))
  {
    throw Exception("Unable to open file: " + QFileInfo(file).absoluteFilePath());
  }
  QString errorMessage;
  int errorLine = 0;
  int errorColumn = 0;

  if (!doc.setContent(&file, false, &errorMessage, &errorLine, &errorColumn))
  {
    file.close();
    QString error = QString("[Line %1, Col %2] %3")
                        .arg(errorLine)
                        .arg(errorColumn)
                        .arg(errorMessage);

    throw Exception(
        "Unable to parse file: " + error
        + " in file: " + QFileInfo(file).absoluteFilePath());
  }
  file.close();

  fullText = doc.toString();
  parse(doc);
}

void Template::parse(QDomDocument& doc)
{
  QDomElement docElem = doc.documentElement();

  QDomNode n = docElem.firstChild();

  QDomElement ne = docElem.toElement(); // try to convert the node to an element.
  if (!ne.isNull())
  {
    if (ne.hasAttribute("name"))
    {
      this->name = ne.attribute("name");
    }
    else
    {
      this->name = "NONAME";
    }

    if (ne.hasAttribute("defaultExtension"))
    {
      this->defaultExtension = ne.attribute("defaultExtension");
    }
    else
    {
      this->defaultExtension = "Unknown file type (*.txt)";
    }

    if (ne.hasAttribute("runAfter"))
    {
      this->runAfter = ne.attribute("runAfter");
    }
    else
    {
      this->runAfter = "";
    }
  }

  while (!n.isNull())
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if (!e.isNull())
    {
      if (e.tagName() == "primitive" || e.tagName() == "substitution")
      {
        if (e.tagName() == "substitution")
        {
          WARNING(
              "Element-name 'substitution' is a deprecated name. Please rename to "
              "'primitive'.");
        }

        if (!e.hasAttribute("name"))
        {
          WARNING("Primitive without name attribute found!");
          n = n.nextSibling();
          continue;
        }

        QString type = "";
        if (e.hasAttribute("type"))
        {
          type = "::" + e.attribute("type");
        }

        QString name = e.attribute("name") + type;
        primitives[name] = TemplatePrimitive(e.text());
      }
      else if (e.tagName() == "description")
      {
        description = e.text();
      }
      else
      {

        WARNING("Expected 'primitive' or 'description' element, found: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
}

TemplateRenderer::TemplateRenderer(const QString& xmlDefinitionFile)
    : counter(0)
{

  QFile file(xmlDefinitionFile);

  workingTemplate.read(file);
}

TemplateRenderer::TemplateRenderer(Template myTemplate)
    : counter(0)
    , workingTemplate(std::move(myTemplate))
{
}

TemplateRenderer::TemplateRenderer()
    : counter(0)
{
}

TemplateRenderer::~TemplateRenderer() = default;

auto TemplateRenderer::assertPrimitiveExists(const QString& templateName) -> bool
{
  if (!workingTemplate.getPrimitives().contains(templateName))
  {
    QString error = QString("Template error: the primitive '%1' is not defined.")
                        .arg(templateName);

    if (!missingTypes.contains(error))
    {
      // Only show each error once.
      WARNING(error);
      INFO(
          "(A template may not support all drawing primitives. Either update the "
          "template or choose another primitive)");
      missingTypes.insert(error);
    }
    return false;
  }
  return true;
}

void TemplateRenderer::doBeginEndSubstitutions(TemplatePrimitive& t)
{
  t.substitute("{CamPosX}", QString::number(cameraPosition.x()));
  t.substitute("{CamPosY}", QString::number(cameraPosition.y()));
  t.substitute("{CamPosZ}", QString::number(cameraPosition.z()));

  t.substitute("{CamUpX}", QString::number(cameraUp.x()));
  t.substitute("{CamUpY}", QString::number(cameraUp.y()));
  t.substitute("{CamUpZ}", QString::number(cameraUp.z()));

  Vector3f cameraDir = cameraTarget - cameraPosition;
  cameraDir.normalize();

  t.substitute("{CamDirX}", QString::number(cameraDir.x()));
  t.substitute("{CamDirY}", QString::number(cameraDir.y()));
  t.substitute("{CamDirZ}", QString::number(cameraDir.z()));

  t.substitute("{CamRightX}", QString::number(cameraRight.x()));
  t.substitute("{CamRightY}", QString::number(cameraRight.y()));
  t.substitute("{CamRightZ}", QString::number(cameraRight.z()));

  t.substitute("{CamTargetX}", QString::number(cameraTarget.x()));
  t.substitute("{CamTargetY}", QString::number(cameraTarget.y()));
  t.substitute("{CamTargetZ}", QString::number(cameraTarget.z()));

  if (t.contains("{CamColumnMatrix}"))
  {
    const Vector3f u = -cameraRight;
    const Vector3f v = cameraUp;
    const Vector3f w = -cameraDir;

    QString mat = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 0.0 0.0 0.0 1.0")
                      .arg(u.x())
                      .arg(v.x())
                      .arg(w.x())
                      .arg(cameraPosition.x())
                      .arg(u.y())
                      .arg(v.y())
                      .arg(w.y())
                      .arg(cameraPosition.y())
                      .arg(u.z())
                      .arg(v.z())
                      .arg(w.z())
                      .arg(cameraPosition.z());

    t.substitute("{CamColumnMatrix}", mat);
  }

  t.substitute("{aspect}", QString::number(aspect));
  t.substitute("{width}", QString::number(width));
  t.substitute("{height}", QString::number(height));
  t.substitute("{fov}", QString::number(fov));

  t.substitute("{BR}", QString::number(backRgb.x()));
  t.substitute("{BG}", QString::number(backRgb.y()));
  t.substitute("{BB}", QString::number(backRgb.z()));

  t.substitute("{BR256}", QString::number(backRgb.x() * 255));
  t.substitute("{BG256}", QString::number(backRgb.y() * 255));
  t.substitute("{BB256}", QString::number(backRgb.z() * 255));
}

void TemplateRenderer::doStandardSubstitutions(
    Math::Vector3f base,
    Math::Vector3f dir1,
    Math::Vector3f dir2,
    Math::Vector3f dir3,
    TemplatePrimitive& t)
{
  if (t.contains("{matrix}"))
  {
    auto mats = fmt::format(
        "{} {} {} 0 {} {} {} 0 {} {} {} 0 {} {} {} 1",
        dir1.x(),
        dir1.y(),
        dir1.z(),
        dir2.x(),
        dir2.y(),
        dir2.z(),
        dir3.x(),
        dir3.y(),
        dir3.z(),
        base.x(),
        base.y(),
        base.z());
    t.substitute("{matrix}", QString::fromStdString(mats));
  }

  if (t.contains("{columnmatrix}"))
  {
    auto mats = fmt::format(
        "{} {} {} {} {} {} {} {} {} {} {} {} 0 0 0 1",
        dir1.x(),
        dir2.x(),
        dir3.x(),
        base.x(),
        dir1.y(),
        dir2.y(),
        dir3.y(),
        base.y(),
        dir1.z(),
        dir2.z(),
        dir3.z(),
        base.z());

    t.substitute("{columnmatrix}", mats);
  }

  if (t.contains("{povmatrix}"))
  {
    auto mats = fmt::format(
        "{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}",
        dir1.x(),
        dir1.y(),
        dir1.z(),
        dir2.x(),
        dir2.y(),
        dir2.z(),
        dir3.x(),
        dir3.y(),
        dir3.z(),
        base.x(),
        base.y(),
        base.z());

    t.substitute("{povmatrix}", mats);
  }

  t.substitute("{r}", fmt::to_string(rgb.x()));
  t.substitute("{g}", fmt::to_string(rgb.y()));
  t.substitute("{b}", fmt::to_string(rgb.z()));
  t.substitute("{alpha}", fmt::to_string(alpha));
  t.substitute("{oneminusalpha}", fmt::to_string(1. - alpha));
}

void TemplateRenderer::drawBox(
    Math::Vector3f base,
    Math::Vector3f dir1,
    Math::Vector3f dir2,
    Math::Vector3f dir3,
    PrimitiveClass* classID)
{
  QString alternateID = getAlternateId(classID);

  if (!assertPrimitiveExists("box" + alternateID))
    return;
  TemplatePrimitive t(workingTemplate.get("box" + alternateID));

  doStandardSubstitutions(base, dir1, dir2, dir3, t);

  if (t.contains("{uid}"))
  {
    t.substitute("{uid}", QString("Box%1").arg(counter++));
  }

  output.push_back(t.getText());
};

void TemplateRenderer::drawTriangle(
    Math::Vector3f p1,
    Math::Vector3f p2,
    Math::Vector3f p3,
    PrimitiveClass* classID)
{
  QString alternateID = getAlternateId(classID);
  if (!assertPrimitiveExists("triangle" + alternateID))
    return;
  TemplatePrimitive t(workingTemplate.get("triangle" + alternateID));

  if (t.contains("{uid}"))
  {
    t.substitute("{uid}", QString("Triangle%1").arg(counter++));
  }

  t.substitute("{p1x}", QString::number(p1.x()));
  t.substitute("{p1y}", QString::number(p1.y()));
  t.substitute("{p1z}", QString::number(p1.z()));
  t.substitute("{p2x}", QString::number(p2.x()));
  t.substitute("{p2y}", QString::number(p2.y()));
  t.substitute("{p2z}", QString::number(p2.z()));
  t.substitute("{p3x}", QString::number(p3.x()));
  t.substitute("{p3y}", QString::number(p3.y()));
  t.substitute("{p3z}", QString::number(p3.z()));

  t.substitute("{alpha}", QString::number(alpha));
  t.substitute("{oneminusalpha}", QString::number(1 - alpha));

  output.push_back(t.getText());
}

void TemplateRenderer::drawGrid(
    Math::Vector3f base,
    Math::Vector3f dir1,
    Math::Vector3f dir2,
    Math::Vector3f dir3,
    PrimitiveClass* classID)
{
  QString alternateID = getAlternateId(classID);
  if (!assertPrimitiveExists("grid" + alternateID))
    return;
  TemplatePrimitive t(workingTemplate.get("grid" + alternateID));

  doStandardSubstitutions(base, dir1, dir2, dir3, t);

  if (t.contains("{uid}"))
  {
    t.substitute("{uid}", QString("Grid%1").arg(counter++));
  }

  output.push_back(t.getText());
};

void TemplateRenderer::drawLine(
    Math::Vector3f from,
    Math::Vector3f to,
    PrimitiveClass* classID)
{
  QString alternateID = getAlternateId(classID);
  if (!assertPrimitiveExists("line" + alternateID))
    return;
  TemplatePrimitive t(workingTemplate.get("line" + alternateID));
  t.substitute("{x1}", QString::number(from.x()));
  t.substitute("{y1}", QString::number(from.y()));
  t.substitute("{z1}", QString::number(from.z()));

  t.substitute("{x2}", QString::number(to.x()));
  t.substitute("{y2}", QString::number(to.y()));
  t.substitute("{z2}", QString::number(to.z()));

  t.substitute("{alpha}", QString::number(alpha));
  t.substitute("{oneminusalpha}", QString::number(1 - alpha));

  if (t.contains("{uid}"))
  {
    t.substitute("{uid}", QString("Line%1").arg(counter++));
  }

  output.push_back(t.getText());
};

void TemplateRenderer::drawDot(Math::Vector3f v, PrimitiveClass* classID)
{
  QString alternateID = getAlternateId(classID);
  if (!assertPrimitiveExists("dot" + alternateID))
    return;
  TemplatePrimitive t(workingTemplate.get("dot" + alternateID));
  t.substitute("{x}", QString::number(v.x()));
  t.substitute("{y}", QString::number(v.y()));
  t.substitute("{z}", QString::number(v.z()));

  t.substitute("{r}", QString::number(rgb.x()));
  t.substitute("{g}", QString::number(rgb.y()));
  t.substitute("{b}", QString::number(rgb.z()));

  t.substitute("{alpha}", QString::number(alpha));
  t.substitute("{oneminusalpha}", QString::number(1 - alpha));

  if (t.contains("{uid}"))
  {
    t.substitute("{uid}", QString("Dot%1").arg(counter++));
  }

  output.push_back(t.getText());
};

void TemplateRenderer::drawSphere(
    Math::Vector3f center,
    float radius,
    PrimitiveClass* classID)
{
  QString alternateID = getAlternateId(classID);
  if (!assertPrimitiveExists("sphere" + alternateID))
    return;
  TemplatePrimitive t(workingTemplate.get("sphere" + alternateID));
  t.substitute("{cx}", QString::number(center.x()));
  t.substitute("{cy}", QString::number(center.y()));
  t.substitute("{cz}", QString::number(center.z()));

  t.substitute("{rad}", QString::number(radius));

  t.substitute("{r}", QString::number(rgb.x()));
  t.substitute("{g}", QString::number(rgb.y()));
  t.substitute("{b}", QString::number(rgb.z()));

  t.substitute("{alpha}", QString::number(alpha));
  t.substitute("{oneminusalpha}", QString::number(1 - alpha));

  if (t.contains("{uid}"))
  {
    t.substitute("{uid}", QString("Sphere%1").arg(counter++));
  }

  output.push_back(t.getText());
};

void TemplateRenderer::begin()
{
  if (!assertPrimitiveExists("begin"))
    return;
  TemplatePrimitive t(workingTemplate.get("begin"));

  doBeginEndSubstitutions(t);

  output.push_back(t.getText());
};

void TemplateRenderer::end()
{
  if (!assertPrimitiveExists("end"))
    return;
  TemplatePrimitive t(workingTemplate.get("end"));

  doBeginEndSubstitutions(t);

  output.push_back(t.getText());
};

void TemplateRenderer::callGeneric(PrimitiveClass* classID)
{
  QString alternateID = getAlternateId(classID);
  if (!assertPrimitiveExists("template" + alternateID))
    return;
  TemplatePrimitive t(workingTemplate.get("template" + alternateID));
  output.push_back(t.getText());
}

void TemplateRenderer::setBackgroundColor(Math::Vector3f rgb)
{
  backRgb = rgb;
}

void TemplateRenderer::drawMesh(
    Math::Vector3f startBase,
    Math::Vector3f startDir1,
    Math::Vector3f startDir2,
    Math::Vector3f endBase,
    Math::Vector3f endDir1,
    Math::Vector3f endDir2,
    PrimitiveClass* classID)
{
  QString alternateID = getAlternateId(classID);
  if (!assertPrimitiveExists("mesh" + alternateID))
    return;
  TemplatePrimitive t(workingTemplate.get("mesh"));
  if (t.contains("{uid}"))
  {
    t.substitute("{uid}", QString("Box%1").arg(counter++));
  }

  // TODO: This really isn't a matrix, we need to find a better way to export the mesh.
  if (t.contains("{matrix}"))
  {
    QString mat = QString(
                      "%1 %2 %3 0 %4 %5 %6 0 %7 %8 %9 0 %10 %11 %12 0 %13 %14 %15 0 %16 "
                      "%17 %18 1")
                      .arg(startBase.x())
                      .arg(startBase.y())
                      .arg(startBase.z())
                      .arg(startDir1.x())
                      .arg(startDir1.y())
                      .arg(startDir1.z())
                      .arg(startDir2.x())
                      .arg(startDir2.y())
                      .arg(startDir2.z())
                      .arg(endBase.x())
                      .arg(endBase.y())
                      .arg(endBase.z())
                      .arg(endDir1.x())
                      .arg(endDir1.y())
                      .arg(endDir1.z())
                      .arg(endDir2.x())
                      .arg(endDir2.y())
                      .arg(endDir2.z());

    t.substitute("{matrix}", mat);
  }

  t.substitute("{r}", QString::number(rgb.x()));
  t.substitute("{g}", QString::number(rgb.y()));
  t.substitute("{b}", QString::number(rgb.z()));
  t.substitute("{alpha}", QString::number(alpha));
  t.substitute("{oneminusalpha}", QString::number(1 - alpha));

  output.push_back(t.getText());
};

void TemplateRenderer::callCommand(
    const QString& renderClass,
    const QString& /*command*/)
{
  if (renderClass != this->renderClass())
    return;
}

auto TemplateRenderer::getOutput() -> QString
{
  QString out = output.join("");

  // Normalize output (seems the '\n' converts to CR+LF on windows while saving
  // whereas '\r\n' converts to CR+CR+LF? so we remove the \r's).
  out = out.replace("\r", "");
  return out;
}

}
}
