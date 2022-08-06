#pragma once

#include <ssynth/Model/Rule.h>
#include <ssynth/Model/State.h>

namespace ssynth
{
namespace Model
{

struct RuleState
{
  RuleState(){};
  RuleState(Rule* rule, State state)
      : rule(rule)
      , state(state)
  {
    Q_ASSERT(rule);
  };

  Rule* rule{};
  State state;
};

/// The ExecutionStack keeps track of which operations to perform next.
/// Rules are executed in generations:
///  The rules on the stack are all executed in each generation,
///  and each rule will add a number of new rules to the next generation of the stack.
///  Only one level is recursion is followed at each generation.
typedef std::vector<RuleState> ExecutionStack;

/*
        struct ExecutionStack {
            std::vector< RuleState > currentStack;
        };
        */

}
}
