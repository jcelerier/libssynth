#include <ssynth/Model/State.h>

namespace ssynth::Model
{

State::State()
    : matrix(Math::Matrix4f::Identity())
    , hsv(Math::Vector3f(0, 1.0f, 1.0f))
    , alpha(1.0f)
    , previous(nullptr)
    , seed(0)
{
}

auto State::operator=(const State& rhs) -> State&
{
  this->matrix = rhs.matrix;
  this->hsv = rhs.hsv;
  this->alpha = rhs.alpha;
  this->maxDepths = rhs.maxDepths;
  this->seed = rhs.seed;
  if (rhs.previous)
  {
    delete (this->previous);
    this->previous = new PreviousState();
    *(this->previous) = *rhs.previous;
  }
  else
  {
    delete (this->previous);
    this->previous = nullptr;
  }
  return *this;
}

void State::setPreviousState(Math::Matrix4f matrix, Math::Vector3f hsv, float alpha)
{
  if (previous)
  {
    delete (previous);
  }

  this->previous = new PreviousState();
  this->previous->matrix = matrix;
  this->previous->hsv = hsv;
  this->previous->alpha = alpha;
}

State::State(const State& rhs)
    : matrix(rhs.matrix)
    , hsv(rhs.hsv)
    , alpha(rhs.alpha)
    , maxDepths(rhs.maxDepths)
    , previous(nullptr)
    , seed(rhs.seed)
{

  if (rhs.previous)
  {
    delete (this->previous);
    this->previous = new PreviousState();
    *(this->previous) = *rhs.previous;
  }
  else
  {
    delete (this->previous);
    this->previous = nullptr;
  }
}

State::~State()
{
  delete (previous);
}
}
