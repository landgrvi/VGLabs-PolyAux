#pragma once

#include "plugin.hpp"
#include "BaseLoop8.hpp"
#include "PachdeThemedModule.hpp"

using namespace rack;

struct Outs8 : PachdeThemedModule {
	enum ParamId {
		SCHEMA_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		INTERLEAVED_DRY_OUTPUT,
		LEFT_DRY_OUTPUT,
		RIGHT_DRY_OUTPUT,
		ENUMS(WET_OUTPUTS,16),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

    float debugValue = 0;
	float blinkPhase = 0;
	
	unsigned int monoOutputMode = 0;
	
	bool expandsLeftward = false;
	bool expandsRightward = false;
	
	expMessage rightMessages[2][1]; // messages to & from right module
	
	comboAudioOut dryOutput; // from right module, to output ports
	comboAudio wetAudio; // from right module, to output ports

	
    Outs8() ;
	inline bool calcLeftExpansion() { return false; }
	inline bool calcRightExpansion() { return rightExpander.module && rightExpander.module->model == modelBaseLoop8; }
	void process(const ProcessArgs &args) override ;
	json_t* dataToJson() override ;
	void dataFromJson(json_t* rootJ) override ;
	void onReset(const ResetEvent& e) override ;

}; // Outs8

struct Outs8Widget : PachdeThemedModuleWidget {
	
	PanelBorder* panelBorder;
	Outs8Widget(Outs8* module) ;
	void appendContextMenu(Menu* menu) override ;
	void draw(const DrawArgs& args) override ;
	void step() override ;
}; // Outs8Widget

