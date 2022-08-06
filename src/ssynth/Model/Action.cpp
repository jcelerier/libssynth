#include <ssynth/Logging.h>
#include <ssynth/Model/Action.h>
#include <ssynth/Model/Builder.h>
#include <ssynth/Model/ExecutionStack.h>

namespace ssynth
{
using namespace Logging;

namespace Model
{
void Action::apply(Builder* b, const Rule* callingRule, int ruleDepth) const
{
  bool rememberPreviousMatrix
      = true; // at some point we might make this optional -> only needed for grid meshes...

  if (set != nullptr)
  {
    b->setCommand(set->key, set->value);
    return;
  }

  State s = b->getState();

  std::vector<int> counters(loops.size(), 1);

  if (counters.size() == 0)
  {
    if (callingRule)
    {
      s.maxDepths[callingRule] = ruleDepth;
    }
    b->getNextStack().push_back(RuleState(rule->rule(), s));
    return;
  }

  bool done = false;
  while (!done)
  {

    // create state
    State s0 = s;
    if (rememberPreviousMatrix)
    {
      // Copy the old matrix...
      s0.setPreviousState(s.matrix, s.hsv, s.alpha);
    }
    for (int i = 0; i < counters.size(); i++)
    {
      for (int j = 0; j < counters[i]; j++)
      {
        s0 = loops[i].transformation.apply(s0, b->getColorPool());
      }
    }
    if (callingRule)
    {
      s0.maxDepths[callingRule] = ruleDepth;
    }
    b->getNextStack().push_back(RuleState(rule->rule(), s0));

    // increase lowest counter...
    counters[0]++;
    for (int i = 0; i < counters.size(); i++)
    {
      if (counters[i] > loops[i].repetitions)
      {
        if (i == counters.size() - 1)
        {
          done = true;
        }
        else
        {
          counters[i] = 1;
          counters[i + 1]++;
        }
      }
    }
  }
}

Action::Action(const QString& key, const QString& value)
{
  set = std::make_shared<SetAction>();
  set->key = key;
  set->value = value;
  rule = nullptr;
}

Action::~Action()
{
  // TODO: Handle leaks (Actions are treated as value types, and hence rule,set ptrs are duped)
  //delete(rule);
}

void Action::addTransformationLoop(const TransformationLoop& tl)
{
  loops.push_back(tl);
}

void Action::setRule(const QString& ruleName)
{
  rule = std::make_shared<RuleRef>(ruleName);
  set = nullptr;
}

Action::Action(const Transformation& t, const QString& ruleName)
{
  loops.emplace_back(1, t);
  rule = std::make_shared<RuleRef>(ruleName);
  set = nullptr;
}

Action::Action(const QString& ruleName)
{
  rule = std::make_shared<RuleRef>(ruleName);
  set = nullptr;
}

}
}
