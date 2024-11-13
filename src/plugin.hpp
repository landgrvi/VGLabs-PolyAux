#pragma once
#include <rack.hpp>
#include "components.hpp"
#include "expander.hpp"
#include "polyaux.hpp"

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelMyModule;
extern Model* modelInsOutsGains;
extern Model* modelGainsSendsReturns;
extern Model* modelOuts;
