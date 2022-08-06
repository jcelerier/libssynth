#pragma once

#include <QString>
// #include <QProgressDialog>
#include <ssynth/ColorPool.h>
#include <ssynth/Model/ExecutionStack.h>
#include <ssynth/Model/Rendering/Renderer.h>
#include <ssynth/Model/RuleSet.h>
#include <ssynth/Model/State.h>

// #include <ssynth/Matrix4.h>
// #include <ssynth/GLEngine/EngineWidget.h>

struct QProgressDialog
{
  QProgressDialog(auto&&...) { }
  void setValue(int) { }
  void setLabelText(QString) { }
  bool wasCanceled() const noexcept { return false; }
  void setMinimumDuration(int) { }
  void show() { }
  void hide() { }
  void setWindowModality(auto) { }
};
namespace ssynth
{
namespace Model
{

/// A Builder executes the rule set on a Renderer object
class Builder
{
public:
  Builder(Rendering::Renderer* renderTarget, RuleSet* ruleSet, bool verbose);
  ~Builder();
  void build();

  void setCommand(const QString& command, QString param);
  ExecutionStack& getNextStack();
  State& getState() { return state; };
  Rendering::Renderer* getRenderer() { return renderTarget; };
  void increaseObjectCount() { objects++; };

  // True, if the random seed was changed by the builder (by 'set seed <int>')
  bool seedChanged() { return hasSeedChanged; }
  int getNewSeed() { return newSeed; }
  ColorPool* getColorPool() { return colorPool; }
  // std::vector<GLEngine::Command> getRaytracerCommands() { return raytracerCommands; };
  bool wasCancelled() { return userCancelled; }

private:
  void recurseBreadthFirst(
      QProgressDialog& progressDialog,
      int& maxTerminated,
      int& minTerminated,
      int& generationCounter);
  void recurseDepthFirst(
      QProgressDialog& progressDialog,
      int& maxTerminated,
      int& minTerminated,
      int& generationCounter);

  State state;

  bool userCancelled;

  ExecutionStack stack;
  ExecutionStack nextStack;
  Rendering::Renderer* renderTarget;
  RuleSet* ruleSet;
  bool verbose;
  int maxGenerations;
  int maxObjects;
  int objects;
  int newSeed;
  bool hasSeedChanged;
  float minDim;
  float maxDim;
  bool syncRandom;
  int initialSeed;
  State* currentState{};
  ColorPool* colorPool{};
  // std::vector<GLEngine::Command> raytracerCommands;
};

}
}
