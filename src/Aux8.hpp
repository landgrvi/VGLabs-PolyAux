#pragma once

#include "plugin.hpp"

using namespace rack;

template<typename TModule>
struct Aux8 : Module {

    float debugValue = 0;
	float blinkPhase = 0;
	
	simd::float_4 monoGains[2] = { };
	float oldMuteVals[8] = {-2.f, -2.f, -2.f, -2.f, -2.f, -2.f, -2.f, -2.f};
	float oldGainVals[8] = {-2.f, -2.f, -2.f, -2.f, -2.f, -2.f, -2.f, -2.f};
	float oldReturnPanVal = -2.f;

	unsigned int soloMe = 0;
	unsigned int soloToRight = 0;
	unsigned int soloTracks = 0;
	unsigned int numMe = 0;
	unsigned int numModules = 0;
	unsigned int muteMe = 0;

	bool expandsRightward = false;
	bool expandsLeftward = false;
	
	unsigned int returnPanMode = 0;
	unsigned int oldReturnPanMode = 8; // out of range to force initial update
	unsigned int masterPanMode = 3;
	unsigned int oldMasterPanMode = 8; // out of range to force initial update
	float trackPanVals[4] = { };
	float oldReturnLevel = -1.f;
	
	comboAudioOut sendOutput; // to effect
	comboAudioIn returnInput; // from effect 

	expMessage leftMessages[2][1]; // messages to & from left module
	expMessage rightMessages[2][1]; // messages to & from right module
	
    Aux8() {
		config(TModule::PARAMS_LEN, TModule::INPUTS_LEN, TModule::OUTPUTS_LEN, TModule::LIGHTS_LEN);
		char strBuf[32];
		for (unsigned int i = 0; i < 8; i++) {
			snprintf(strBuf, 32, "Gain channel %d", i + 1);
			configParam(TModule::GAIN_PARAMS + i, 0.f, 2.f, 1.f, strBuf, " dB", -10, 40.f); //From MindMeld Auxpander: configParam(GLOBAL_AUXSEND_PARAMS + i, 0.0f, maxAGGlobSendFader, 1.0f, strBuf, " dB", -10, 20.0f * GlobalConst::globalAuxSendScalingExponent);
			snprintf(strBuf, 32, "Mute channel %d", i + 1);
			configParam(TModule::MUTE_PARAMS + i, 0.f, 1.f, 1.f, strBuf);
		}
		configOutput(TModule::INTERLEAVED_SEND, "Interleaved send");
		configOutput(TModule::LEFT_SEND, "Left send");
		configOutput(TModule::RIGHT_SEND, "Right send");
		configInput(TModule::INTERLEAVED_RETURN, "Interleaved return");
		configInput(TModule::LEFT_RETURN, "Left return");
		configInput(TModule::RIGHT_RETURN, "Right return");
		configParam(TModule::RETURN_PAN_PARAM, -1.f, 1.f, 0.f, "Return pan", "%", 0, 100);
		//configParam(TModule::RETURN_PAN_PARAM, 0.f, M_PI_2, M_PI_4, "Return pan", "%", 0, 100);
		configParam(TModule::RETURN_GAIN_PARAM, 0.f, M_SQRT2, 1.f, "Return level", " dB", -10, 40);
		configParam(TModule::SOLO_PARAM, 0.f, 1.f, 0.f, "Solo");
		configParam(TModule::MUTE_PARAM, 0.f, 1.f, 0.f, "Mute");
		
		leftExpander.producerMessage = leftMessages[0];
		leftExpander.consumerMessage = leftMessages[1];	
		rightExpander.producerMessage = rightMessages[0];
		rightExpander.consumerMessage = rightMessages[1];

		sendOutput.setPorts(&outputs[TModule::INTERLEAVED_SEND], &outputs[TModule::LEFT_SEND], &outputs[TModule::RIGHT_SEND]);
		returnInput.setPorts(&inputs[TModule::INTERLEAVED_RETURN], &inputs[TModule::LEFT_RETURN], &inputs[TModule::RIGHT_RETURN]);

	} //Aux8 constructor
	
	//This is really a helper, should probably go in some other file
	virtual void setPanVals(float* panVals, float pv, unsigned int panMode) {
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
			case 2: // linear panning (figure 6), normalised to gain = 1 with knob in centre
				norm = 2.f;
				panVals[0] = (M_PI_2 - theta) * (2 / M_PI) * norm; 
				panVals[1] = 0.f;
				panVals[2] = 0.f;
				panVals[3] = theta * (2 / M_PI) * norm; 
				break;
			case 3: // compromise 4.5dB panning (figure 8), normalised to gain = 1 with knob in centre
				norm = 1.f / sqrt(M_SQRT2 / 4);
				panVals[0] = sqrt((M_PI_2 - theta) * (2 / M_PI) * cos(theta)) * norm;
				panVals[1] = 0.f;
				panVals[2] = 0.f;
				panVals[3] = sqrt(theta * (2 / M_PI) * sin(theta)) * norm;
				break;
			case 4: // constant power panning (figure 7), normalised to gain = 1 with knob in centre
				norm = M_SQRT2;
				panVals[0] = cos(theta) * norm;
				panVals[1] = 0.f;
				panVals[2] = 0.f;
				panVals[3] = sin(theta) * norm;
				break;
			default: // we shouldn't get here
				panVals[0] = panVals[1] = panVals[2] = panVals[3] = 0.f;
		}
	}

    virtual void updateGains() {

		bool changed = false;
		for (unsigned int i = 0; i < 8; i++) {
			if (params[TModule::MUTE_PARAMS + i].getValue() != oldMuteVals[i]) {
				changed = true;
				break;
			}
			if (params[TModule::GAIN_PARAMS + i].getValue() != oldGainVals[i]) {
				changed = true;
				break;
			}
		}
		if (changed) {
			unsigned int c = 0;
			for (unsigned int i = 0; i < 2; i++) {
				for (unsigned int j = 0; j < 4; j++) {
					oldMuteVals[c] = params[TModule::MUTE_PARAMS + c].getValue();
					lights[TModule::MUTE_LIGHTS + (c * 2)].setBrightness(1 - oldMuteVals[c]);
					lights[TModule::MUTE_LIGHTS + (c * 2) + 1].setBrightness(oldMuteVals[c]);
					oldGainVals[c] = params[TModule::GAIN_PARAMS + c].getValue();
					monoGains[i][j] =  oldMuteVals[c] > 0 ? 0.f : oldGainVals[c];
					c++;
				}
			}
		}

		float pv = params[TModule::RETURN_PAN_PARAM].getValue();
		if (pv != oldReturnPanVal || returnPanMode != oldReturnPanMode || (returnPanMode == 0 && masterPanMode != oldMasterPanMode)) {
			changed = true;
			oldReturnPanVal = pv;
			oldReturnPanMode = returnPanMode;
			oldMasterPanMode = masterPanMode;
			setPanVals(trackPanVals, pv, (returnPanMode == 0 ? masterPanMode : returnPanMode - 1));
		}
		float rl = params[TModule::RETURN_GAIN_PARAM].getValue();
		if (rl != oldReturnLevel) {
			changed = true;
			oldReturnLevel = rl;
		}
		if (changed) {
			returnInput.setScaling(trackPanVals, rl);
		}
	} //updateGains
	
	virtual inline bool calcLeftExpansion() {
		return leftExpander.module && (leftExpander.module->model == modelGainsSendsReturns || leftExpander.module->model == modelInsOutsGains);
	}

	virtual inline bool calcRightExpansion() {
		return rightExpander.module && rightExpander.module->model == modelGainsSendsReturns;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "returnPanMode", json_integer(returnPanMode));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* returnPanModeJ = json_object_get(rootJ, "returnPanMode");
		if (returnPanModeJ)
			returnPanMode = json_integer_value(returnPanModeJ);
	}

	void onReset(const ResetEvent& e) override {
		Module::onReset(e);
		returnPanMode = 0;
	}	
	
}; // Aux8	

template<typename TModule>
struct Aux8Widget : ModuleWidget {
	
	PanelBorder* panelBorder;

	Aux8Widget(TModule* module, std::string panelFile) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, panelFile)));
		SvgPanel* svgPanel = static_cast<SvgPanel*>(getPanel());
		panelBorder = findBorder(svgPanel->fb);
		
		// Penultimate column: Gains and mutes per channel
		float xpos = 0;
		xpos = box.pos.x + box.size.x - 66;
		for (unsigned int i = 0; i < 8; i++) {
			addParam(createParamCentered<ChickenHeadKnobIvory>(Vec(xpos, mm2px(12+(15*i))), module, TModule::GAIN_PARAMS + i));
			addParam(createLightParamCentered<GreenRedLightLatch>(Vec(xpos, mm2px(12+(15*i))), module, TModule::MUTE_PARAMS + i, TModule::MUTE_LIGHTS + (i * 2)));
		}
		
		// Last column: Sends & Returns
		xpos = box.pos.x + box.size.x - 23.5;
		addOutput(createOutputCentered<WhiteRedPJ301MPort>(Vec(xpos, box.pos.y + mm2px(12)), module, TModule::INTERLEAVED_SEND));
		addOutput(createOutputCentered<WhitePJ301MPort>(Vec(xpos, box.pos.y + mm2px(21)), module, TModule::LEFT_SEND));
		addOutput(createOutputCentered<RedPJ301MPort>(Vec(xpos, box.pos.y + mm2px(30)), module, TModule::RIGHT_SEND));
		
		addInput(createInputCentered<WhiteRedPJ301MPort>(Vec(xpos, box.pos.y + mm2px(43)), module, TModule::INTERLEAVED_RETURN));
		addInput(createInputCentered<WhitePJ301MPort>(Vec(xpos, box.pos.y + mm2px(52)), module, TModule::LEFT_RETURN));
		addInput(createInputCentered<RedPJ301MPort>(Vec(xpos, box.pos.y + mm2px(61)), module, TModule::RIGHT_RETURN));

		addParam(createParamCentered<ChickenHeadKnobIvory>(Vec(xpos, mm2px(12+(15*4))), module, TModule::RETURN_PAN_PARAM));
		addParam(createParamCentered<VCVSlider>(Vec(xpos, box.pos.y + mm2px(93)), module, TModule::RETURN_GAIN_PARAM));
		addParam(createParamCentered<btnMute>(Vec(xpos, box.pos.y + mm2px(110)), module, TModule::MUTE_PARAM));
		addParam(createParamCentered<btnSolo>(Vec(xpos, box.pos.y + mm2px(116)), module, TModule::SOLO_PARAM));
	}

	void draw(const DrawArgs& args) override {
		if (module) {
			TModule* module = static_cast<TModule*>(this->module);
			if (module->calcRightExpansion()) {
				DrawArgs newDrawArgs = args;
				newDrawArgs.clipBox.size.x += mm2px(0.3f); // panels have their base rectangle this much larger, to kill gap artifacts
				ModuleWidget::draw(newDrawArgs);
			} else {
				ModuleWidget::draw(args);
			}
		} else {
			ModuleWidget::draw(args);
		}
	} // draw
	
	void appendContextMenu(Menu* menu) override {
		TModule* module = getModule<TModule>();
		if (module->model != modelInsOutsGains) menu->addChild(new MenuSeparator);
		menu->addChild(createIndexPtrSubmenuItem("Return Pan Mode", {"Use Master (default)", "True Pan (L + R)", "Linear Attenuation","6dB Boost (linear)", "4.5dB Boost (compromise)", "3dB boost (constant power)"}, &module->returnPanMode));
	}
	
	void step() override {
		if (module) {
			TModule* module = static_cast<TModule*>(this->module);
			
			// Hide borders to show expansion
			int leftShift = (module->calcLeftExpansion() ? 3 : 0);
			int rightEnlarge = (module->calcRightExpansion() ? 4 : 0);
			if (panelBorder->box.size.x != (box.size.x + leftShift + rightEnlarge)) {
				panelBorder->box.pos.x = -leftShift; // move it over so left side won't display
				panelBorder->box.size.x = (box.size.x + leftShift + rightEnlarge); // make it bigger so right side won't display
				SvgPanel* svgPanel = static_cast<SvgPanel*>(getPanel());
				svgPanel->fb->dirty = true;
			}
			module->lights[TModule::SOLO_LIGHT].setBrightness(module->soloMe = module->params[TModule::SOLO_PARAM].getValue());
			module->lights[TModule::MUTE_LIGHT].setBrightness(module->muteMe = module->params[TModule::MUTE_PARAM].getValue());
		}
		ModuleWidget::step();
	}
}; // Aux8Widget


