#pragma once

#include "plugin.hpp"
#include "Aux8.hpp"

using namespace rack;

struct InsOutsGains : Aux8<InsOutsGains> {
	enum ParamId {
		MASTER_PAN_PARAM,
		MASTER_GAIN_PARAM,
		MASTER_MUTE_PARAM,
		DRYPLUS_PARAM,
		ENUMS(GAIN_PARAMS, 8),
		ENUMS(MUTE_PARAMS, 8),
		RETURN_PAN_PARAM,
		RETURN_GAIN_PARAM,
		SOLO_PARAM,
		MUTE_PARAM,
		TEST_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INTERLEAVED_INPUT,
		LEFT_INPUT,
		RIGHT_INPUT,
		INTERLEAVED_RETURN,
		LEFT_RETURN,
		RIGHT_RETURN,
		INPUTS_LEN
	};
	enum OutputId {
		INTERLEAVED_PREGAIN_OUTPUT,
		LEFT_PREGAIN_OUTPUT,
		RIGHT_PREGAIN_OUTPUT,
		INTERLEAVED_WET_OUTPUT,
		LEFT_WET_OUTPUT,
		RIGHT_WET_OUTPUT,
		INTERLEAVED_SEND,
		LEFT_SEND,
		RIGHT_SEND,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(MUTE_LIGHTS, 16), // 8 GreenRedLights
		BLINK_LIGHT,
		SOLO_LIGHT,
		MUTE_LIGHT,
		TEST_LIGHT,
		LIGHTS_LEN
	};
	
	LedDisplayTextFieldRoundedRect<InsOutsGains>* trackLabels[8];
	char trackLabelChars[33] = "-01--02--03--04--05--06--07--08-";
	bool loadLabels = true;
	float masterPanVals[4] = {1.f, 0.f, 0.f, 1.f};
	float oldMasterPanVal = -2.f; //start out of bounds
	float oldMasterLevel = -1.f;
	float oldMute = -1.f;
	
	float debugValue1 = -5.f;
	float debugValue2 = -6.f;
	
	comboAudioIn firstInput; // from input ports - does not scale
	comboAudioOut wetOutput; // to output ports - scales

	comboAudioIn wetInput; // from right module - already scaled
	
	comboAudioLinked<comboAudioIn, comboAudioOut> firstWithSend; // for applying channel gains/mutes on firstInput to sendOutput
	comboAudioLinked<comboAudioIn, comboAudioOut> returnWithWet; // for adding returnInput to wetOutput

	comboAudioBlended returnAndFirstWithWet;
	
	InsOutsGains() ;
	void process(const ProcessArgs &args) override ;
	inline bool calcLeftExpansion() override ;
	void updateGains() override ;
	json_t* dataToJson() override ;
	void dataFromJson(json_t* rootJ) override ;
	void onReset(const ResetEvent& e) override ;
};

struct InsOutsGainsWidget : Aux8Widget<InsOutsGains> {
	unsigned int oldModules = 0;
	Label* labelTotal;

	InsOutsGainsWidget(InsOutsGains* module) ;
	void appendContextMenu(Menu* menu) override ;
	void step() override ;
};

