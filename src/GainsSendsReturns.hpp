#pragma once

#include "plugin.hpp"
#include "Aux8.hpp"

using namespace rack;

struct GainsSendsReturns : Aux8<GainsSendsReturns> {
	enum ParamId {
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
		TEST_LIGHT,
		LIGHTS_LEN
	};
	
	comboAudio pregainAudio;
	comboAudio wetAudio;
	comboAudioLinked<comboAudio, comboAudioOut> pregainWithSend;
	comboAudioLinked<comboAudioIn, comboAudio> returnWithWet;
	
	GainsSendsReturns() ;
	void process(const ProcessArgs &args) override ;
};

struct GainsSendsReturnsWidget : Aux8Widget<GainsSendsReturns> {
	GainsSendsReturnsWidget(GainsSendsReturns* module) : Aux8Widget<GainsSendsReturns>(module, "res/GainsSendsReturns_6hp_Plus.svg") { }
};

Model* modelGainsSendsReturns = createModel<GainsSendsReturns, GainsSendsReturnsWidget>("GainsSendsReturns");
