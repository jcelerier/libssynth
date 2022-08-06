#pragma once

#include <ssynth/Model/RuleRef.h>
#include <ssynth/Model/TransformationLoop.h>

#include <memory>
namespace ssynth
{
namespace Model
{

struct SetAction
{
  QString key;
  QString value;
};

/// An actions is a number of loops, that is applied to a rule.
///
/// Rules with only one transformation, e.g.:
///    { x 1 } R1
/// are represented as a loop transformation with one repetitions.
class Action
{
public:
  Action(const Transformation& t, const QString& ruleName);
  Action(const QString& ruleName);
  Action(const QString& key, const QString& value);
  Action() = default;
  Action(const Action&) = default;
  Action(Action&&) noexcept = default;
  Action& operator=(const Action&) = default;
  Action& operator=(Action&&) noexcept = default;

  void addTransformationLoop(const TransformationLoop& tl);
  void setRule(const QString& rule);

  ~Action();

  /// If 'callingRule' != 0 the new states generated will be set with
  /// a depth equal to 'ruleDepth'
  void apply(Builder* b, const Rule* callingRule, int ruleDepth) const;
  RuleRef* getRuleRef() const { return rule.get(); }

private:
  std::vector<TransformationLoop> loops;
  // The rule that will be called after all transformations.
  std::shared_ptr<RuleRef> rule{};
  std::shared_ptr<SetAction> set{};
};

}
}
