#pragma once

struct expMessage { // contains both directions
// These things travel rightwards
	simd::float_4 pregainAudio[4] = { };
	unsigned int pregainChans = 0;
	unsigned int soloSoFar = 0;
	unsigned int numModulesSoFar = 0;
	unsigned int masterPanMode = 3;
// These things travel leftwards
	simd::float_4 wetAudio[4] = { };
	unsigned int wetChans = 0;
	unsigned int soloTotal = 0;
	unsigned int numModulesTotal = 0;
};

// Find a PanelBorder instance in the given widget's children - from MindMeldModular I'm pretty sure
inline PanelBorder* findBorder(Widget* widget) {
	for (auto it = widget->children.begin(); it != widget->children.end(); ) {
		PanelBorder *bwChild = dynamic_cast<PanelBorder*>(*it);
		if (bwChild) {
			return bwChild;
		}
		else {
			++it;
		}
	}
	return NULL;
}

