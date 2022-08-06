#include <boost/range/algorithm_ext.hpp>
#include <ssynth/Exception.h>
#include <ssynth/Logging.h>
#include <ssynth/Model/AmbiguousRule.h>
#include <ssynth/Model/CustomRule.h>
#include <ssynth/Model/PrimitiveRule.h>
#include <ssynth/Model/RuleRef.h>
#include <ssynth/Model/RuleSet.h>
#include <ssynth/Vector3.h>

#include <QStringList>

#include <map>
#include <typeinfo>

namespace ssynth
{
using namespace Exceptions;
using namespace Logging;
using namespace Math;

namespace Model
{

/// Constructor. Automatically adds built-in rules.
RuleSet::RuleSet()
    : recurseDepth(false)
    , defaultClass(new PrimitiveClass())
{
  topLevelRule = new CustomRule("TopLevelRule");

  /// Add instances of predefined built-in types.
  rules.push_back(new PrimitiveRule(PrimitiveRule::Box, defaultClass));
  rules.push_back(new PrimitiveRule(PrimitiveRule::Sphere, defaultClass));
  rules.push_back(new PrimitiveRule(PrimitiveRule::Cylinder, defaultClass));
  rules.push_back(new PrimitiveRule(PrimitiveRule::Mesh, defaultClass));
  rules.push_back(new PrimitiveRule(PrimitiveRule::Line, defaultClass));
  rules.push_back(new PrimitiveRule(PrimitiveRule::Dot, defaultClass));
  rules.push_back(new PrimitiveRule(PrimitiveRule::Grid, defaultClass));
  rules.push_back(new PrimitiveRule(PrimitiveRule::Template, defaultClass));
  rules.push_back(topLevelRule);
};

void RuleSet::setRulesMaxDepth(int maxDepth)
{
  for (auto& rule : rules)
  {
    int md = rule->getMaxDepth();
    //INFO(QString("Rule: %1, %2 -> %3").arg(rules[i]->getName()).arg(md).arg(maxDepth));
    if (md <= 0)
      rule->setMaxDepth(maxDepth);
  }
}

/// Deletes rules
RuleSet::~RuleSet()
{
  delete defaultClass;
  for (auto& rule : rules)
    delete rule;
  //for (int i = 0; i < primitiveClasses.size(); i++) delete(primitiveClasses[i]);
}

void RuleSet::addRule(Rule* rule)
{
  // Check if the rule name is already used...
  QString name = rule->getName();

  for (int i = 0; i < rules.size(); i++)
  {

    if (rules[i]->getName() == name)
    {
      if (typeid(*rules[i]) == typeid(CustomRule))
      {
        // A Custom rule already exists with the same name.
        // Now we must remove the existing rule, and create a new ambiguous rule hosting them both.

        Rule* r = rules[i];
        boost::range::remove_erase(rules, r);
        auto* cr1 = dynamic_cast<CustomRule*>(r);

        auto* ar = new AmbiguousRule(name);
        ar->appendRule(cr1);

        auto* cr2 = dynamic_cast<CustomRule*>(rule);
        if (!cr2)
          throw Exception(
              "Trying to add non-custom rule to ambiguous rule: '%1'. " + name);
        ar->appendRule(cr2);

        rules.push_back(ar);

        return;
      }
      else if (typeid(*rules[i]) == typeid(PrimitiveRule))
      {
        // A primitive rule already exists with the same name. This is not acceptable.
        throw Exception(QString("A primitive rule already exists with the name: '%1'. "
                                "New definitions can not merged.")
                            .arg(name));
      }
      else if (typeid(*rules[i]) == typeid(AmbiguousRule))
      {
        // A ambiguous rule already exists with the same name. We will add to it.
        auto* ar = dynamic_cast<AmbiguousRule*>(rules[i]);
        auto* cr = dynamic_cast<CustomRule*>(rule);
        if (!cr)
          throw Exception(
              "Trying to add non-custom rule to ambiguous rule: '%1'. " + name);
        ar->appendRule(cr);
        return;
      }
      else
      {
        WARNING("Unknown typeid");
      }
    }
  }

  rules.push_back(rule);
}

/// Resolve symbolic names into pointers
auto RuleSet::resolveNames() -> QStringList
{

  // build map
  std::map<QString, Rule*> map;
  for (auto& rule : rules)
    map[rule->getName()] = rule;

  QStringList usedPrimitives;

  // resolve rules.
  for (auto& rule : rules)
  {

    std::vector<RuleRef*> refs = rule->getRuleRefs();

    for (auto& ref : refs)
    {
      QString name = ref->getReference();
      if (!map.contains(name))
      {
        // We could not resolve the name.
        // Check if it has a class specifier.
        QStringList sl = name.split("::");
        if (sl.size() == 2)
        {
          QString baseName = sl[0];
          QString classID = sl[1];

          if (!map.contains(baseName))
          {
            throw Exception(QString("Unable to resolve base rule name: %1 for rule %2")
                                .arg(baseName)
                                .arg(name));
          }

          // Now we have to create a new instance of this rule.
          Rule* r = map[baseName];

          if (typeid(*r) != typeid(PrimitiveRule))
          {
            throw Exception(QString("Only primitive rules (box, sphere, ...) may have a "
                                    "class specifier: %1 is invalid")
                                .arg(name));
          }

          auto* pr = dynamic_cast<PrimitiveRule*>(r);
          auto* newRule = new PrimitiveRule(*pr);
          newRule->setClass(getPrimitiveClass(classID));

          map[name] = newRule;

          //INFO("Created new class for rule: " + name);
        }
        else
        {
          // The Polygons rules (i.e. Triangle[x,y,z]) are special rules, each created on the fly.
          QRegExp r("triangle\\[(.*)\\]");
          if (r.exactMatch(name))
          {
            // Check the arguments.
            INFO("Found:" + r.cap(1));
            std::vector<Vector3f> v;
            QStringList l = r.cap(1).split(";");
            if (l.size() != 3)
            {
              throw Exception(QString("Unable to parse Triangle definition - must be "
                                      "triangle(p1;p2;p3) - found : %1")
                                  .arg(name));
            }

            for (unsigned int i = 0; i < 3; i++)
            {
              QStringList l2 = l[i].split(",");
              if (l2.size() != 3)
              {
                throw Exception(
                    QString("Unable to parse Triangle definition - coordinates must be "
                            "like '0.1,0.2,0.3' - found : %1")
                        .arg(l[i]));
              }
              bool ok = false;
              float f1 = l2[0].toFloat(&ok);
              if (!ok)
                throw Exception(QString("Unable to parse Triangle definition - error in "
                                        "first coordinate - found in : %1")
                                    .arg(name));
              float f2 = l2[1].toFloat(&ok);
              if (!ok)
                throw Exception(QString("Unable to parse Triangle definition - error in "
                                        "second coordinate - found in : %1")
                                    .arg(name));
              float f3 = l2[2].toFloat(&ok);
              if (!ok)
                throw Exception(QString("Unable to parse Triangle definition - error in "
                                        "third coordinate - found in : %1")
                                    .arg(name));
              v.emplace_back(f1, f2, f3);
            }

            map[name] = new TriangleRule(v[0], v[1], v[2], defaultClass);
          }
          else
          {
            throw Exception(QString("Unable to resolve rule: %1").arg(name));
          }
        }
      }
      if (dynamic_cast<PrimitiveRule*>(map[name]))
      {
        if (!usedPrimitives.contains(name))
          usedPrimitives.push_back(name);
      }
      ref->setRef(map[name]);
    }
  }

  return usedPrimitives;
}

///
auto RuleSet::getUnreferencedNames() -> QStringList
{

  WARNING("RuleSet::getUnreferencedNames(): Not implemented yet!");
  return {};
};

auto RuleSet::getStartRule() const -> Rule*
{
  return topLevelRule;
};

/// For debug
void RuleSet::dumpInfo() const
{
  int custom = 0;
  int ambi = 0;
  int primitive = 0;
  int rulesCount = 0;

  for (auto rule : rules)
  {
    rulesCount++;

    auto* cr = dynamic_cast<CustomRule*>(rule);
    if (cr)
      custom++;

    auto* ar = dynamic_cast<AmbiguousRule*>(rule);
    if (ar)
      ambi++;

    auto* pr = dynamic_cast<PrimitiveRule*>(rule);
    if (pr)
      primitive++;
  }

  Debug(QString("Loaded %1 user rules: %2 Custom Rules, %3 Ambiguous Rules")
            .arg(rulesCount - primitive)
            .arg(custom)
            .arg(ambi));
  Debug(QString("Loaded %1 built-in rules.").arg(primitive));
}

auto RuleSet::existsPrimitiveClass(const QString& classLabel) -> bool
{
  for (auto& primitiveClasse : primitiveClasses)
  {
    if (primitiveClasse->name == classLabel)
      return true;
  }
  return false;
}

auto RuleSet::getPrimitiveClass(const QString& classLabel) -> PrimitiveClass*
{
  for (auto& primitiveClasse : primitiveClasses)
  {
    if (primitiveClasse->name == classLabel)
      return primitiveClasse;
  }
  auto* p = new PrimitiveClass(*defaultClass);
  p->name = classLabel;
  //INFO("Created new primitiveClass: " + classLabel);
  primitiveClasses.push_back(p);
  return p;
}

}
}
