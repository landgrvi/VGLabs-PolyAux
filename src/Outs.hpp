#pragma once

#include "plugin.hpp"
#include "InsOutsGains.hpp"
#include "PachdeThemedModule.hpp"

using namespace rack;

struct Outs : PachdeThemedModule {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		INTERLEAVED_PREGAIN_OUTPUT,
		LEFT_PREGAIN_OUTPUT,
		RIGHT_PREGAIN_OUTPUT,
		ENUMS(WET_OUTPUTS,16),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

    float debugValue = 0;
	float blinkPhase = 0;
	
	bool expandsLeftward = false;
	bool expandsRightward = false;
	
	expMessage rightMessages[2][1]; // messages to & from right module
	
	comboAudioOut pregainOutput; // from right module, to output ports
	comboAudio wetAudio; // from right module, to output ports

	
    Outs() ;
	inline bool calcLeftExpansion() { return false; }
	inline bool calcRightExpansion() { return rightExpander.module && rightExpander.module->model == modelInsOutsGains; }
	void process(const ProcessArgs &args) override ;
}; // Outs

struct OutsWidget : PachdeThemedModuleWidget {
	
	PanelBorder* panelBorder;
	OutsWidget(Outs* module) ;
	void draw(const DrawArgs& args) override ;
	void step() override ;
}; // OutsWidget

