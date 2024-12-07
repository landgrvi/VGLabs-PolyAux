#pragma once

struct expMessage { // contains both directions
// These things travel rightwards
	simd::float_4 pregainAudio[4] = { };
	unsigned int pregainChans = 0;
	unsigned int soloSoFar = 0;
	unsigned int numModulesSoFar = 0;
	unsigned int masterPanMode = 3;
	std::string leftTheme;
// These things travel leftwards
	simd::float_4 wetAudio[4] = { };
	unsigned int wetChans = 0;
	unsigned int soloTotal = 0;
	unsigned int numModulesTotal = 0;
};


