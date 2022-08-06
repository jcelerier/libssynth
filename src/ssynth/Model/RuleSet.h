#pragma once

#include <ssynth/Model/CustomRule.h>
#include <ssynth/Model/PrimitiveClass.h>
#include <ssynth/Model/Rule.h>

namespace ssynth
{
namespace Model
{

// using namespace GLEngine;

/// Container for all rules.
class RuleSet
{
public:
  /// Constructor. Automatically adds built-in rules.
  RuleSet();

  /// Deletes rules
  ~RuleSet();

  /// Added rules belong to the RuleSet and will be deleted by the RuleSet destructor.
  void addRule(Rule* rule);

  /// Resolve symbolic names into pointers
  /// Returns a list of the primitives used
  QStringList resolveNames();

  /// TODO: Implement
  QStringList getUnreferencedNames();

  Rule* getStartRule() const;

  CustomRule* getTopLevelRule() const { return topLevelRule; }

  /// For debug
  void dumpInfo() const;

  void setRecurseDepthFirst(bool value) { recurseDepth = value; };
  bool recurseDepthFirst() { return recurseDepth; }
  void setRulesMaxDepth(int maxDepth);

  /// Returns the PrimitiveClass with this name. Constructs a new one if missing.
  PrimitiveClass* getPrimitiveClass(const QString& classLabel);
  bool existsPrimitiveClass(const QString& classLabel);
  PrimitiveClass* getDefaultClass() { return defaultClass; }

private:
  std::vector<Rule*> rules;
  std::vector<PrimitiveClass*> primitiveClasses;
  PrimitiveClass* defaultClass;
  CustomRule* topLevelRule;
  bool recurseDepth;
};

}
}
