cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(libssynth)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core Gui Xml)
add_library(ssynth
  src/ssynth/Parser/EisenParser.cpp
  src/ssynth/Parser/Preprocessor.cpp
  src/ssynth/Parser/Tokenizer.cpp

  src/ssynth/Model/Action.cpp
  src/ssynth/Model/AmbiguousRule.cpp
  src/ssynth/Model/Builder.cpp
  src/ssynth/Model/CustomRule.cpp
  src/ssynth/Model/PrimitiveRule.cpp
  src/ssynth/Model/RuleSet.cpp
  src/ssynth/Model/State.cpp
  src/ssynth/Model/Transformation.cpp

  src/ssynth/Model/Rendering/TemplateRenderer.cpp
  src/ssynth/Model/Rendering/ObjRenderer.cpp

  src/ssynth/ColorPool.cpp
  src/ssynth/ColorUtils.cpp
  src/ssynth/Logging.cpp
  src/ssynth/MiniParser.cpp
  src/ssynth/RandomStreams.cpp
)
target_link_libraries(ssynth PUBLIC Qt5::Core Qt5::Gui Qt5::Xml)
target_include_directories(ssynth PUBLIC src)

add_executable(ssynthgen src/CommandLine.cpp)
target_link_libraries(ssynthgen PRIVATE ssynth)
