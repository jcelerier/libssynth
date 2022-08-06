#include <ssynth/Logging.h>
#include <ssynth/Model/Rendering/ObjRenderer.h>

#include <QColor>

namespace ssynth
{
using namespace Math;
using namespace Logging;

namespace Model::Rendering
{

void ObjGroup::addGroup(ObjGroup g)
{
  int vertexCount = vertices.size();
  int normalCount = normals.size();

  for (auto& vertice : g.vertices)
  {
    vertices.push_back(vertice);
  }

  for (auto& normal : g.normals)
  {
    normals.push_back(normal);
  }

  for (auto& face : g.faces)
  {
    for (auto& j : face)
    {
      j.vID = j.vID + vertexCount;
      j.nID = j.nID + normalCount;
    }
    faces.push_back(face);
  }
}

// Removes redundant vertices.
void ObjGroup::reduceVertices()
{
  std::vector<Vector3f> newVertices;
  std::vector<Vector3f> newNormals;
  std::map<int, int> oldToNewVertex;
  std::map<int, int> oldToNewNormals;

  // Reduce vertices and normals (if present)
  for (int i = 0; i < vertices.size(); i++)
  {
    auto it = std::find(newVertices.begin(), newVertices.end(), vertices[i]);
    if (it == newVertices.end())
    {
      newVertices.push_back(vertices[i]);
      oldToNewVertex[i] = newVertices.size() - 1;
    }
    else
    {
      oldToNewVertex[i] = std::distance(newVertices.begin(), it);
    }
  }

  // Reduce vertices and normals (if present)
  for (int i = 0; i < normals.size(); i++)
  {
    auto it = std::find(newNormals.begin(), newNormals.end(), normals[i]);
    if (it == newNormals.end())
    {
      newNormals.push_back(normals[i]);
      oldToNewNormals[i] = newNormals.size() - 1;
    }
    else
    {
      oldToNewNormals[i] = std::distance(newNormals.begin(), it);
    }
  }

  // Update indices
  for (auto& face : faces)
  {
    for (auto& j : face)
    {
      j.vID = oldToNewVertex[j.vID - 1] + 1; // beware OBJ is 1-based!
      j.nID = oldToNewNormals[j.nID - 1] + 1;
    }
  }

  vertices = newVertices;
  normals = newNormals;
}

namespace
{
/*
                This function was taken from Paul Bourkes website:
                http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/sphere_cylinder/

                Create a unit sphere centered at the origin
                This code illustrates the concept rather than implements it efficiently
                It is called with two arguments, the theta and phi angle increments in degrees
                Note that at the poles only 3 vertex facet result
                while the rest of the sphere has 4 point facets
                */
void CreateUnitSphere(int dt, int dp, ObjGroup& motherGroup, Matrix4f m)
{
  float DTOR = 3.1415f / 180.0f;
  double dtheta = 180.0 / dt;
  double dphi = 360.0 / dp;
  ObjGroup group;
  for (int i = 0; i < dt; i++)
  {
    double theta = -90.0 + ((i * 180.0) / (double)dt);
    for (int j = 0; j < dp; j++)
    {
      double phi = ((j * 360.0) / (double)dp);
      int vi = group.vertices.size() + 1;
      int vn = group.normals.size() + 1;

      group.vertices.push_back(
          m
          * Vector3f(
              cos(theta * DTOR) * cos(phi * DTOR),
              cos(theta * DTOR) * sin(phi * DTOR),
              sin(theta * DTOR)));
      group.normals.emplace_back(
          cos(theta * DTOR) * cos(phi * DTOR),
          cos(theta * DTOR) * sin(phi * DTOR),
          sin(theta * DTOR));

      std::vector<VertexNormal> vns;
      if (theta > -90 && theta < 90)
      {
        group.vertices.push_back(
            m
            * Vector3f(
                cos(theta * DTOR) * cos((phi + dphi) * DTOR),
                cos(theta * DTOR) * sin((phi + dphi) * DTOR),
                sin(theta * DTOR)));
        group.normals.emplace_back(
            cos(theta * DTOR) * cos((phi + dphi) * DTOR),
            cos(theta * DTOR) * sin((phi + dphi) * DTOR),
            sin(theta * DTOR));
        for (int j = 0; j < 4; j++)
          vns.emplace_back(j + vi, j + vn);
      }
      else
      {
        for (int j = 0; j < 3; j++)
          vns.emplace_back(j + vi, j + vn);
      }
      group.vertices.push_back(
          m
          * Vector3f(
              cos((theta + dtheta) * DTOR) * cos((phi + dphi) * DTOR),
              cos((theta + dtheta) * DTOR) * sin((phi + dphi) * DTOR),
              sin((theta + dtheta) * DTOR)));
      group.normals.emplace_back(
          cos((theta + dtheta) * DTOR) * cos((phi + dphi) * DTOR),
          cos((theta + dtheta) * DTOR) * sin((phi + dphi) * DTOR),
          sin((theta + dtheta) * DTOR));
      group.vertices.push_back(
          m
          * Vector3f(
              cos((theta + dtheta) * DTOR) * cos(phi * DTOR),
              cos((theta + dtheta) * DTOR) * sin(phi * DTOR),
              sin((theta + dtheta) * DTOR)));
      group.normals.emplace_back(
          cos((theta + dtheta) * DTOR) * cos(phi * DTOR),
          cos((theta + dtheta) * DTOR) * sin(phi * DTOR),
          sin((theta + dtheta) * DTOR));
      group.faces.push_back(vns);
    }
  }
  group.reduceVertices();
  motherGroup.addGroup(group);
}

}

void ObjRenderer::addLineQuad(
    ObjGroup& group,
    Vector3f v1,
    Vector3f v2,
    Vector3f v3,
    Vector3f v4)
{
  int vi = group.vertices.size() + 1;
  group.vertices.push_back(v1);
  group.vertices.push_back(v2);
  group.vertices.push_back(v3);
  group.vertices.push_back(v4);

  for (int j = 0; j < 4; j++)
  {
    std::vector<VertexNormal> vns;
    vns.emplace_back(vi + j, -1);
    vns.emplace_back(vi + (j + 1 % 4), -1);
    group.faces.push_back(vns);
  }
}

void ObjRenderer::addQuad(
    ObjGroup& group,
    Vector3f v1,
    Vector3f v2,
    Vector3f v3,
    Vector3f v4)
{
  int vi = group.vertices.size() + 1;
  int vn = group.normals.size() + 1;
  group.vertices.push_back(v1);
  group.vertices.push_back(v2);
  group.vertices.push_back(v3);
  group.vertices.push_back(v4);

  Vector3f normal = Vector3f::cross((v2 - v1), (v4 - v1)).normalized();
  group.normals.push_back(normal);
  group.normals.push_back(normal);
  group.normals.push_back(normal);
  group.normals.push_back(normal);

  std::vector<VertexNormal> vns;
  vns.reserve(4);
  for (int j = 0; j < 4; j++)
    vns.emplace_back(vi + j, vn + j);
  group.faces.push_back(vns);
}

void ObjRenderer::setClass(const QString& classID, Vector3f rgb, double /*alpha*/)
{
  // Should we also group by alpha channel?
  QString className;
  if (groupByTagging)
    className += classID;
  if (groupByColor)
    className += QColor(int(rgb[0] * 255), int(rgb[1] * 255), int(rgb[2] * 255)).name();
  if (className.isEmpty())
    className = "default";
  if (!groups.contains(className))
    groups[className] = ObjGroup();
  groups[className].groupName = className;
  currentGroup = className;
}

void ObjRenderer::drawBox(
    Math::Vector3f O,
    Math::Vector3f v1,
    Math::Vector3f v2,
    Math::Vector3f v3,
    PrimitiveClass* classID)
{
  setClass(classID->name, rgb, alpha);
  ObjGroup group;
  addQuad(group, O, O + v2, O + v2 + v1, O + v1);
  addQuad(group, O + v3, O + v1 + v3, O + v2 + v1 + v3, O + v2 + v3);
  addQuad(group, O, O + v3, O + v3 + v2, O + v2);
  addQuad(group, O + v1, O + v2 + v1, O + v3 + v2 + v1, O + v3 + v1);
  addQuad(group, O, O + v1, O + v3 + v1, O + v3);
  addQuad(group, O + v2, O + v3 + v2, O + v3 + v2 + v1, O + v1 + v2);
  group.reduceVertices();
  groups[currentGroup].addGroup(group);
};

void ObjRenderer::drawMesh(
    Math::Vector3f O,
    Math::Vector3f v1,
    Math::Vector3f v2,
    Math::Vector3f endBase,
    Math::Vector3f u1,
    Math::Vector3f u2,
    PrimitiveClass* classID)
{

  setClass(classID->name, rgb, alpha);
  ObjGroup group;
  Vector3f v3 = endBase - O;
  addQuad(group, O, O + v2, O + v2 + v1, O + v1);
  addQuad(group, O + v3, O + u1 + v3, O + u2 + u1 + v3, O + u2 + v3);
  addQuad(group, O, O + v3, O + v3 + u2, O + v2);
  addQuad(group, O + v1, O + v2 + v1, O + v3 + u2 + u1, O + v3 + u1);
  addQuad(group, O, O + v1, O + v3 + u1, O + v3);
  addQuad(group, O + v2, O + v3 + u2, O + v3 + u2 + u1, O + v1 + v2);
  group.reduceVertices();
  groups[currentGroup].addGroup(group);
};

void ObjRenderer::drawGrid(
    Math::Vector3f O,
    Math::Vector3f v1,
    Math::Vector3f v2,
    Math::Vector3f v3,
    PrimitiveClass* classID)
{
  setClass(classID->name, rgb, alpha);
  ObjGroup group;
  addLineQuad(group, O, O + v2, O + v2 + v1, O + v1);
  addLineQuad(group, O + v3, O + v1 + v3, O + v2 + v1 + v3, O + v2 + v3);
  addLineQuad(group, O, O + v3, O + v3 + v2, O + v2);
  addLineQuad(group, O + v1, O + v2 + v1, O + v3 + v2 + v1, O + v3 + v1);
  addLineQuad(group, O, O + v1, O + v3 + v1, O + v3);
  addLineQuad(group, O + v2, O + v3 + v2, O + v3 + v2 + v1, O + v1 + v2);
  group.reduceVertices();
  groups[currentGroup].addGroup(group);
};

void ObjRenderer::drawLine(
    Math::Vector3f from,
    Math::Vector3f to,
    PrimitiveClass* classID)
{
  setClass(classID->name, rgb, alpha);
  ObjGroup group;
  group.vertices.push_back(from);
  group.vertices.push_back(to);
  std::vector<VertexNormal> vns;
  vns.emplace_back(1, -1);
  vns.emplace_back(2, -1);
  group.faces.push_back(vns);
  groups[currentGroup].addGroup(group);
};

void ObjRenderer::drawTriangle(
    Math::Vector3f p1,
    Math::Vector3f p2,
    Math::Vector3f p3,
    PrimitiveClass* classID)
{
  setClass(classID->name, rgb, alpha);
  ObjGroup group;
  group.vertices.push_back(p1);
  group.vertices.push_back(p2);
  group.vertices.push_back(p3);

  std::vector<VertexNormal> vns;
  vns.reserve(3);
  for (int j = 0; j < 3; j++)
    vns.emplace_back(1 + j, -1);
  group.faces.push_back(vns);
  groups[currentGroup].addGroup(group);
}

void ObjRenderer::drawDot(Math::Vector3f v, PrimitiveClass* classID)
{
  setClass(classID->name, rgb, alpha);
  ObjGroup group;
  group.vertices.push_back(v);
  std::vector<VertexNormal> vns;
  vns.emplace_back(1, -1);
  group.faces.push_back(vns);
  groups[currentGroup].addGroup(group);
};

void ObjRenderer::drawSphere(
    Math::Vector3f center,
    float radius,
    PrimitiveClass* classID)
{
  setClass(classID->name, rgb, alpha);

  Matrix4f m = Matrix4f::Translation(center.x(), center.y(), center.z())
               * (Matrix4f::ScaleMatrix(radius));

  CreateUnitSphere(sphereDT, sphereDP, groups[currentGroup], m);
};

void ObjRenderer::begin()
{
  rgb = Vector3f(1, 0, 0);
  alpha = 1;
};

void ObjRenderer::end(){};

void ObjRenderer::writeToStream(QTextStream& ts)
{
  int vertexCount = 0;
  int normalCount = 0;

  for (const auto& [key, o] : groups)
  {
    // Group name
    INFO(o.groupName);
    ts << "g " << o.groupName << Qt::endl;
    ts << "usemtl " << o.groupName << Qt::endl;

    // Vertices
    for(Vector3f v : o.vertices)
    {
      ts << "v " << QString::number(v.x()) << " " << QString::number(v.y()) << " "
         << QString::number(v.z()) << " " << Qt::endl;
    }

    // Normals
    for(Vector3f v : o.normals)
    {
      ts << "vn " << QString::number(v.x()) << " " << QString::number(v.y()) << " "
         << QString::number(v.z()) << " " << Qt::endl;
    }

    // Faces
    for (const std::vector<VertexNormal>& vi : o.faces)
    {
      if (vi.size() == 1)
      {
        ts << "p ";
      }
      else if (vi.size() == 2)
      {
        ts << "l ";
      }
      else
      {
        ts << "f ";
      }
      for (VertexNormal vn : vi)
      {
        if (vn.nID == -1)
        {
          ts << QString::number(vn.vID + vertexCount) << " ";
        }
        else
        {
          ts << QString::number(vn.vID + vertexCount) << "//"
             << QString::number(vn.nID + normalCount) << " ";
        }
      }
      ts << Qt::endl;
    }
    vertexCount += o.vertices.size();
    normalCount += o.normals.size();
  }
};

}
}
