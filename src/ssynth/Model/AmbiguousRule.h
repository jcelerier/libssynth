#pragma once

#include <ssynth/Model/CustomRule.h>
#include <ssynth/Model/Rule.h>

namespace ssynth
{
namespace Model
{

/// If several definitions for the same rule exists,
/// an Ambiguous Rule is created which contains the multiple definitions.
///
/// When the rule is executed, a random rule is chosen from the multiple definitions,
/// taking their weights into account.
class AmbiguousRule : public Rule
{
public:
  AmbiguousRule(const QString& name)
      : Rule(name){};
  ~AmbiguousRule() { qDeleteAll(rules); }

  virtual void apply(Builder* builder) const;

  /// Returns a list over rules that this rule references.
  virtual std::vector<RuleRef*> getRuleRefs() const;

  std::vector<CustomRule*> getRules() { return rules; };

  void appendRule(CustomRule* r) { rules.push_back(r); }

  virtual void setMaxDepth(int maxDepth)
  {
    for (int i = 0; i < rules.size(); i++)
      rules[i]->setMaxDepth(maxDepth);
  }

private:
  std::vector<CustomRule*> rules;
};

}
}
