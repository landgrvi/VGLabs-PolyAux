#pragma once

#include "plugin.hpp"
#include "Aux8.hpp"

using namespace rack;

struct InsOutsGains : Aux8<InsOutsGains> {

	comboAudioBlended returnAndFirstWithWet;
	
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
		ENUMS(MUTE_LIGHTS, 8),
		BLINK_LIGHT,
		SOLO_LIGHT,
		MUTE_LIGHT,
		TEST_LIGHT,
		LIGHTS_LEN
	};
	
	simd::float_4 masterPanLeft = 0.5f;
	simd::float_4 masterPanRight = 0.5f;
	float debugValue1 = -5.f;
	float debugValue2 = -6.f;
	
	comboAudioIn firstInput; // from input ports
	comboAudioOut pregainOutput; // to right module and output ports
	comboAudioOut wetOutput; // to output ports

	comboAudioIn wetInput; // from right module
	
	comboAudioLinked<comboAudioIn, comboAudioOut> firstWithPregain; // for making straight copy of firstInput to pregainOutput
	comboAudioLinked<comboAudioIn, comboAudioOut> firstWithSend; // for applying channel gains/mutes on firstInput to sendOutput
	comboAudioLinked<comboAudioIn, comboAudioOut> returnWithWet; // for applying track fader on returnInput to wetOutput

	InsOutsGains() ;
	void process(const ProcessArgs &args) override ;
	inline bool calcLeftExpansion() override ;
	//void updateGains() ;
	
	
	
};
