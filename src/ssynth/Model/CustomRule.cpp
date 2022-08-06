#include <ssynth/Logging.h>
#include <ssynth/Model/Builder.h>
#include <ssynth/Model/CustomRule.h>

namespace ssynth
{
using namespace Logging;

namespace Model
{

CustomRule::CustomRule(const QString& name)
    : Rule{name}
    , weight(1.0)
    , retirementRule(nullptr)
{
}

CustomRule::~CustomRule()
{
  //delete (retirementRule);
}

void CustomRule::apply(Builder* b) const
{

  int newDepth = -1;
  /// If there is a maxdepth set for this object check it.
  if (getMaxDepth() != -1)
  {
    if (!b->getState().maxDepths.contains(this))
    {
      /// We will add a new maxdepth for this rule to the state.
      newDepth = getMaxDepth() - 1;
    }
    else
    {
      int depth = b->getState().maxDepths[this];
      if (depth <= 0)
      {
        /// This rule is retired.
        if (retirementRule)
        {
          b->getState().maxDepths[this] = maxDepth;
          retirementRule->rule()->apply(b);
        }
        return;
      }
      else
      {
        /// Decrease depth.
        newDepth = depth - 1;
      }
    }
  }

  /// Apply all actions.
  for (const auto& action : actions)
  {
    if (getMaxDepth() != -1)
    {
      action.apply(b, this, newDepth);
    }
    else
    {
      action.apply(b, nullptr, 0);
    }
  }
}

auto CustomRule::getRuleRefs() const -> std::vector<RuleRef*>
{
  std::vector<RuleRef*> list;
  for (const auto& action : actions)
  {
    RuleRef* a = action.getRuleRef();
    if (a)
      list.push_back(a);
  }
  if (retirementRule)
    list.push_back(retirementRule);

  return list;
}

}
}
