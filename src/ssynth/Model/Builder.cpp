#include <ssynth/ColorUtils.h>
#include <ssynth/Exception.h>
#include <ssynth/Logging.h>
#include <ssynth/MiniParser.h>
#include <ssynth/Model/Builder.h>
#include <ssynth/RandomStreams.h>
#include <ssynth/Vector3.h>

#include <list>

namespace ssynth
{
using namespace Logging;
using namespace Math;
using namespace Exceptions;
using namespace Misc;

namespace Model
{

Builder::Builder(Rendering::Renderer* renderTarget, RuleSet* ruleSet, bool verbose)
    : userCancelled(false)
    , renderTarget(renderTarget)
    , ruleSet(ruleSet)
    , verbose(verbose)
    , maxGenerations(1000)
    , maxObjects(100000)
    , objects(0)
    , minDim(0)
    , maxDim(0)
    , newSeed(0)
    , hasSeedChanged(false)
    , syncRandom(false)
    , initialSeed(0)
    , colorPool{new ColorPool("RandomHue")} {};

void Builder::recurseDepthFirst(
    ProgressDialog& progressDialog,
    int& maxTerminated,
    int& minTerminated,
    int& generationCounter)
{
  int lastValue = 0;

  if (maxGenerations > 0)
  {
    ruleSet->setRulesMaxDepth(maxGenerations);
  }

  std::list<RuleState> ruleStates;
  ruleStates.push_back(stack[0]);
  while (ruleStates.size() != 0 && objects < maxObjects)
  {

    /*
                QStringList l;
                foreach (RuleState r, ruleStates) {
                    QString name = r.rule->getName();
                    int d = r.state.maxDepths[r.rule];
                    l.push_back(QString("%1(%2)").arg(name).arg(d));
                }
                INFO(l.join(" "));
                */

    double p = 0;
    if (maxObjects > 0)
    {
      p = objects / (double)maxObjects;
    }
    double progress = p;
    if (maxObjects <= 0)
    {
      progress = (generationCounter % 9) / 9.0;
    }

    if (lastValue != (int)(progress * 100.0))
    {
      progressDialog.setValue((int)(progress * 100.0));
      progressDialog.setLabelText(QString("Building objects...\r\n\r\nGeneration: "
                                          "%1\r\nObjects: %2\r\nPending rules: %3")
                                      .arg(generationCounter)
                                      .arg(objects)
                                      .arg(stack.size()));
      //qApp->processEvents();
      if (progressDialog.wasCanceled())
      {
        userCancelled = true;
        break;
      }
    }
    lastValue = (int)(progress * 100.0);

    generationCounter++; // Notice this does not make sense for depth first search.

    // Now iterate though all RuleState's on stack and create next generation.
    nextStack.clear();
    currentState = &ruleStates.front().state;
    if (currentState->seed != 0)
    {
      RandomStreams::SetSeed(currentState->seed);
      currentState->seed = RandomStreams::Geometry()->getInt();
    }
    state = ruleStates.front().state;

    // Check the dimensions against the min and max limits.
    if (maxDim != 0 || minDim != 0)
    {
      Vector3f s = state.matrix * Vector3f(1, 1, 1) - state.matrix * Vector3f(0, 0, 0);
      double l = s.length();
      if (maxDim && l > maxDim)
      {
        maxTerminated++;
        continue;
      }
      if (minDim && l < minDim)
      {
        minTerminated++;
        continue;
      }
    }

    ruleStates.front().rule->apply(this);
    ruleStates.pop_front();

    auto it = ruleStates.begin();

    for (const RuleState& r : nextStack)
    {
      ruleStates.insert(it, r);
    }
  }
}

void Builder::recurseBreadthFirst(
    ProgressDialog& progressDialog,
    int& maxTerminated,
    int& minTerminated,
    int& generationCounter)
{
  int syncSeed = 0;
  if (syncRandom)
  {
    syncSeed = RandomStreams::Geometry()->getInt();
  }

  int lastValue = 0;

  while (stack.size() != 0 && generationCounter < maxGenerations && objects < maxObjects
         && stack.size() < maxObjects)
  {

    syncSeed = RandomStreams::Geometry()->getInt();

    double p = 0;
    if (maxObjects > 0)
    {
      p = objects / (double)maxObjects;
    }

    double p2 = 0;
    if (maxGenerations > 0)
    {
      p2 = generationCounter / (double)maxGenerations;
    }

    double progress = p;
    if (p2 > p)
      progress = p2;

    if (maxObjects <= 0 && maxGenerations <= 0)
    {
      progress = (generationCounter % 9) / 9.0;
    }

    if (lastValue != (int)(progress * 100.0))
    {
      progressDialog.setValue((int)(progress * 100.0));
      progressDialog.setLabelText(QString("Building objects...\r\n\r\nGeneration: "
                                          "%1\r\nObjects: %2\r\nPending rules: %3")
                                      .arg(generationCounter)
                                      .arg(objects)
                                      .arg(stack.size()));
      //qApp->processEvents();
    }

    lastValue = (int)(progress * 100.0);

    if (progressDialog.wasCanceled())
    {
      userCancelled = true;
      break;
    }

    generationCounter++;

    // Now iterate though all RuleState's on stack and create next generation.
    //INFO(QString("Executing generation %1 with %2 individuals").arg(generationCounter).arg(stack.size()));
    nextStack.clear();
    for (int i = 0; i < stack.size(); i++)
    {
      //	INFO("Executing: " + stack[i].rule->getName());
      currentState = &stack[i].state;
      if (currentState->seed != 0)
      {
        RandomStreams::SetSeed(currentState->seed);
        currentState->seed = RandomStreams::Geometry()->getInt();
      }
      state = stack[i].state;

      // if we are synchronizing random numbers every state must get the same rands
      if (syncRandom)
      {
        RandomStreams::SetSeed(syncSeed);
      }

      // Check the dimensions against the min and max limits.
      if (maxDim != 0 || minDim != 0)
      {
        Vector3f s = state.matrix * Vector3f(1, 1, 1) - state.matrix * Vector3f(0, 0, 0);
        double l = s.length();
        if (maxDim && l > maxDim)
        {
          maxTerminated++;
          continue;
        }
        if (minDim && l < minDim)
        {
          minTerminated++;
          continue;
        }
      }

      Q_ASSERT(stack.size() > i);
      Q_ASSERT(stack[i].rule);
      stack[i].rule->apply(this);
    }
    stack = nextStack;
  }
}

void Builder::build()
{
  objects = 0;
  if (verbose)
    INFO("Starting builder...");

  /// Push first generation state
  stack.push_back(RuleState(ruleSet->getStartRule(), State()));
  int generationCounter = 0;

  ProgressDialog progressDialog("Building objects...", "Cancel", 0, 100, 0);
  progressDialog.setWindowModality(Qt::WindowModal);
  if (verbose)
  {
    progressDialog.setMinimumDuration(0);
    progressDialog.show();
  }
  else
  {
    progressDialog.setMinimumDuration(4000);
  }
  progressDialog.setValue(0);

  int maxTerminated = 0;
  int minTerminated = 0;

  if (ruleSet->recurseDepthFirst())
  {
    recurseDepthFirst(progressDialog, maxTerminated, minTerminated, generationCounter);
  }
  else
  {
    recurseBreadthFirst(progressDialog, maxTerminated, minTerminated, generationCounter);
  }

  progressDialog.setValue(100);
  progressDialog.hide();

  if (verbose)
  {
    if (progressDialog.wasCanceled())
    {
      userCancelled = true;
      INFO("User terminated.");
    }

    if (objects >= maxObjects)
    {
      INFO(QString("Terminated because maximum number of objects reached (%1).")
               .arg(maxObjects));
      INFO(QString("Use 'Set MaxObjects' command to increase this number."));
    }

    if (stack.size() >= objects)
    {
      INFO(QString("Terminated because the number of pending rules reached (%1).")
               .arg(maxObjects));
      INFO(QString("Use 'Set MaxObjects' command to run for longer time."));
    }

    if (generationCounter == maxGenerations)
    {
      INFO(QString("Terminated because maximum number of generations reached (%1).")
               .arg(maxGenerations));
      INFO(QString("Use 'Set Maxdepth' command to increase this number."));
    }

    if (maxTerminated != 0)
    {
      INFO(QString("Terminated %1 branches, because the dimension was greater than max "
                   "size (%2)")
               .arg(maxTerminated)
               .arg(maxDim));
    }
    if (minTerminated != 0)
    {
      INFO(QString("Terminated %1 branches, because the dimension was less than min "
                   "size (%2)")
               .arg(minTerminated)
               .arg(minDim));
    }

    //INFO("Done building...");
  }
}

void Builder::setCommand(const QString& command, QString param)
{
  if (command.toLower().startsWith("raytracer::"))
  {
    const QString& c = command.toLower().remove("raytracer::");
    if (c.contains("::"))
    {
      QStringList l = c.split("::");
      QString classID = l[0];
      QString prop = l[1];
      param.remove("[");
      param.remove("]");
      if (!ruleSet->existsPrimitiveClass(classID))
      {
        WARNING("Trying to set property for unused class: " + classID);
      }
      else
      {
        PrimitiveClass* pc = ruleSet->getPrimitiveClass(classID);

        if (prop == "reflection")
        {
          MiniParser(param, ',').getDouble(pc->reflection);
        }
        else if (prop == "phong")
        {
          MiniParser(param, ',')
              .getDouble(pc->ambient)
              .getDouble(pc->diffuse)
              .getDouble(pc->specular);
          INFO(QString("Lightning for %1 set to: ambient: %1, diffuse: %2, specular: %3")
                   .arg(classID)
                   .arg(pc->ambient)
                   .arg(pc->diffuse)
                   .arg(pc->specular));
        }
        else
        {
          // raytracerCommands.push_back(GLEngine::Command(c,param));
        }
      }
    }
    else
    {
      PrimitiveClass* pc = ruleSet->getDefaultClass();
      const QString& prop = c;
      param.remove("[");
      param.remove("]");
      if (prop == "reflection")
      {
        MiniParser(param, ',').getDouble(pc->reflection);
      }
      else if (prop == "phong")
      {
        MiniParser(param, ',')
            .getDouble(pc->ambient)
            .getDouble(pc->diffuse)
            .getDouble(pc->specular);
        INFO(QString("Default lightning set to: ambient: %1, diffuse: %2, specular: %3")
                 .arg(pc->ambient)
                 .arg(pc->diffuse)
                 .arg(pc->specular));
      }
      else
      {
        // raytracerCommands.push_back(GLEngine::Command(c,param));
      }
    }
  }
  else if (command == "maxdepth")
  {
    bool succes = 0;
    int i = param.toInt(&succes);
    if (!succes)
      throw Exception(QString("Command 'maxdepth' expected integer parameter. Found: %1")
                          .arg(param));
    maxGenerations = i;

    if (ruleSet->recurseDepthFirst())
    {
      if (maxGenerations > 0)
      {
        ruleSet->setRulesMaxDepth(maxGenerations);
      }
    }
  }
  else if (command == "colorpool")
  {
    delete colorPool;
    colorPool
        = nullptr; // Important - prevents crash if ColorPool constructor throws exception
    colorPool = new ColorPool(param); // will throw exception for invalid pools.
  }
  else if (command == "recursion")
  {
  }
  else if (command == "rng")
  {
    WARNING("Using the old random number generators is an obsolete option.");
    if (param.toLower() == "old")
    {
      //	RandomStreams::UseOldRandomGenerators(true);
    }
    else if (param.toLower() == "new")
    {
      //	RandomStreams::UseOldRandomGenerators(false);
    }
    else
    {
      throw Exception("Command 'set rng' expects either 'old' or 'new' as argument.");
    }
  }
  else if (command == "syncrandom")
  {
    if (param.toLower() == "true")
    {
      syncRandom = true;
    }
    else if (param.toLower() == "false")
    {
      syncRandom = false;
    }
    else
    {
      throw Exception(
          QString("Command 'syncrandom' expected either 'true' or 'false'. Found: %1")
              .arg(param));
    }
  }
  else if (command == "maxsize")
  {
    bool succes = 0;
    float f = param.toFloat(&succes);
    if (!succes)
      throw Exception(
          QString("Command 'maxsize' expected floating-point parameter. Found: %1")
              .arg(param));
    maxDim = f;
  }
  else if (command == "minsize")
  {
    bool succes = 0;
    float f = param.toFloat(&succes);
    if (!succes)
      throw Exception(
          QString("Command 'minsize' expected floating-point parameter. Found: %1")
              .arg(param));
    minDim = f;
  }
  else if (command == "maxobjects")
  {
    //INFO(QString("Setting 'maxgenerations' to %1").arg(param));
    bool succes = 0;
    int i = param.toInt(&succes);
    if (!succes)
      throw Exception(
          QString("Command 'maxobjects' expected integer parameter. Found: %1")
              .arg(param));
    maxObjects = i;
  }
  else if (command == "seed")
  {

    if (param.toLower() == "initial")
    {
      if (initialSeed == 0)
      {
        initialSeed = RandomStreams::Geometry()->getInt();
      }
      currentState->seed = initialSeed;
      state.seed = initialSeed;
    }
    else
    {
      bool succes = 0;
      int i = param.toInt(&succes);
      if (!succes)
        throw Exception(
            QString("Command 'seed' expected integer parameter or 'initial'. Found: %1")
                .arg(param));
      RandomStreams::SetSeed(i);
      hasSeedChanged = true;
      newSeed = i;
    }
  }
  else if (command == "background")
  {
    QColor c(param);
    if (!c.isValid())
      throw Exception(
          QString("Command 'background' expected a valid color identifier: Found: %1")
              .arg(param));
    renderTarget->setBackgroundColor(
        Vector3f(c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0));
  }
  else if (command == "scale")
  {
    bool succes = 0;
    double s = param.toDouble(&succes);
    if (!succes)
      throw Exception(
          QString("Command 'scale' expected floating point parameter. Found: %1")
              .arg(param));
    renderTarget->setScale(s);
  }
  else if (command == "translation")
  {
    bool succes = 0;
    Vector3f v3(param, succes);
    if (!succes)
      throw Exception(
          QString(
              "Command 'translation' expected vector (such as [1 3 -10.1]). Found: %1")
              .arg(param));
    renderTarget->setTranslation(v3);
  }
  else if (command == "pivot")
  {
    bool succes = 0;
    Vector3f v3(param, succes);
    if (!succes)
      throw Exception(
          QString("Command 'pivot' expected vector (such as [1 3 -10.1]). Found: %1")
              .arg(param));
    renderTarget->setPivot(v3);
  }
  else if (command == "rotation")
  {
    bool succes = 0;
    Matrix4f m4(param, succes);
    if (!succes)
      throw Exception(QString("Command 'rotation' expected matrix (such as [1 0 0 0 1 0 "
                              "0 0 1]). Found: %1")
                          .arg(param));
    renderTarget->setRotation(m4);
  }
  else if (command == "perspective-angle")
  {
    bool succes = 0;
    double s = param.toDouble(&succes);
    if (!succes)
      throw Exception(
          QString(
              "Command 'perspective-angle' expected floating point parameter. Found: %1")
              .arg(param));
    renderTarget->setPerspectiveAngle(s);
  }
  else if (command == "opengl")
  {
    INFO("Render commands for 'opengl' not impl'ed yet!");
  }
  else if (command == "template")
  {
    renderTarget->callCommand(command, param);
  }
  else
  {
    throw Exception(QString("Unknown command: %1").arg(command));
  }
}

auto Builder::getNextStack() -> ExecutionStack&
{
  return nextStack;
}

Builder::~Builder()
{
  //delete(ruleSet);
  //delete(currentState);
  delete (colorPool);
}
}
}
