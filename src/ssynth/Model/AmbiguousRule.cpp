#include <ssynth/Logging.h>
#include <ssynth/Model/AmbiguousRule.h>
#include <ssynth/Model/Builder.h>
#include <ssynth/RandomStreams.h>

namespace ssynth
{
using namespace Logging;

namespace Model
{

auto AmbiguousRule::getRuleRefs() const -> std::vector<RuleRef*>
{
  std::vector<RuleRef*> list;
  for (auto rule : rules)
  {
    for (int j = 0; j < rule->getRuleRefs().size(); j++)
    {
      list.push_back(rule->getRuleRefs()[j]);
    }
  }
  return list;
}

void AmbiguousRule::apply(Builder* builder) const
{
  // Calc sum of weigths
  double totalWeight = 0;
  for (auto rule : rules)
  {
    totalWeight += rule->getWeight();
  }

  double random = totalWeight * RandomStreams::Geometry()->getDouble();

  // Choose a random rule according to weights
  double accWeight = 0;
  for (auto rule : rules)
  {
    accWeight += rule->getWeight();
    if (random <= accWeight)
    {
      rule->apply(builder);
      return;
    }
  }
  rules[rules.size() - 1]->apply(builder);

  WARNING("Assertion failed: in AmbiguousRule::apply");
};

}
}
