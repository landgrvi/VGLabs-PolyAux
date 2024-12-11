#pragma once

#include "plugin.hpp"
#include "Aux8.hpp"

using namespace rack;

struct Loop8 : Aux8<Loop8> {
	enum ParamId {
		ENUMS(GAIN_PARAMS, 8),
		ENUMS(MUTE_PARAMS, 8),
		RETURN_PAN_PARAM,
		RETURN_GAIN_PARAM,
		SOLO_PARAM,
		MUTE_PARAM,
		SCHEMA_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INTERLEAVED_RETURN,
		LEFT_RETURN,
		RIGHT_RETURN,
		INPUTS_LEN
	};
	enum OutputId {
		INTERLEAVED_SEND,
		LEFT_SEND,
		RIGHT_SEND,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(MUTE_LIGHTS, 16), // 8 GreenRedLights
		SOLO_LIGHT,
		MUTE_LIGHT,
		LEFT_RETURN_LIGHT,
		RIGHT_RETURN_LIGHT,
		TEST_LIGHT,
		LIGHTS_LEN
	};
	
	comboAudio dryAudio;
	comboAudio wetAudio;
	comboAudioLinked<comboAudio, comboAudioOut> dryWithSend;
	comboAudioLinked<comboAudioIn, comboAudio> returnWithWet;
	
	Loop8() ;
	void process(const ProcessArgs &args) override ;
};

struct Loop8Widget : Aux8Widget<Loop8> {
	Loop8Widget(Loop8* module) : Aux8Widget<Loop8>(module, "res/Loop8_6hp_Plus.svg") { }
};

Model* modelLoop8 = createModel<Loop8, Loop8Widget>("Loop8");
