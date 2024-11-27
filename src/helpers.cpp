#include "plugin.hpp"

void setPanVals(float* panVals, float pv, unsigned int panMode) {
	float theta = (pv + 1) * M_PI_4;  // -1 to 1 -> 0 to M_PI_2
	float norm = 1.f;
	switch (panMode) {
		case 0: //additive (MM "true panning")
			panVals[0] = pv <= 0.f ? 1.f : (1.f - pv) / 1.f; //amount of left in left channel
			panVals[1] = pv < 0.f ? -pv / 1.f : 0.f; //amount of right added to left channel
			panVals[2] = pv > 0.f ? pv / 1.f : 0.f; //amount of left added to right channel
			panVals[3] = pv >= 0.f ? 1.f : (1.f + pv) / 1.f; //amount of right in right channel
			break;
		case 1: //attenuative (MM "stereo balance linear")
			panVals[0] = pv <= 0.f ? 1.f : (1.f - pv) / 1.f; 
			panVals[1] = 0.f;
			panVals[2] = 0.f;
			panVals[3] = pv >= 0.f ? 1.f : (1.f + pv) / 1.f; 
			break;
	// From https://www.cs.cmu.edu/~music/icm-online/readings/panlaws/panlaws.pdf
		case 2: // constant power panning (figure 7), normalised to gain = 1 with knob in centre
			norm = M_SQRT2;
			panVals[0] = cos(theta) * norm;
			panVals[1] = 0.f;
			panVals[2] = 0.f;
			panVals[3] = sin(theta) * norm;
			break;
		case 3: // compromise 4.5dB panning (figure 8), normalised to gain = 1 with knob in centre
			norm = 1.f / sqrt(M_SQRT2 / 4);
			panVals[0] = sqrt((M_PI_2 - theta) * (2 / M_PI) * cos(theta)) * norm;
			panVals[1] = 0.f;
			panVals[2] = 0.f;
			panVals[3] = sqrt(theta * (2 / M_PI) * sin(theta)) * norm;
			break;
		case 4: // linear panning (figure 6), normalised to gain = 1 with knob in centre
			norm = 2.f;
			panVals[0] = (M_PI_2 - theta) * (2 / M_PI) * norm; 
			panVals[1] = 0.f;
			panVals[2] = 0.f;
			panVals[3] = theta * (2 / M_PI) * norm; 
			break;
		default: // we shouldn't get here
			panVals[0] = panVals[1] = panVals[2] = panVals[3] = 0.f;
	}
}

// Find a PanelBorder instance in the given widget's children - from MindMeldModular I'm pretty sure
PanelBorder* findBorder(Widget* widget) {
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


