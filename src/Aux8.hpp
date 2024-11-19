#pragma once

#include "plugin.hpp"

using namespace rack;

template<typename TModule>
struct Aux8 : Module {

    float debugValue = 0;
	float blinkPhase = 0;
	
	simd::float_4 monoGains[2] = { };
	simd::float_4 ilGains[4] = { };

	unsigned int soloMe = 0;
	unsigned int soloToRight = 0;
	unsigned int soloTracks = 0;
	unsigned int numMe = 0;
	unsigned int numModules = 0;
	bool muteMe = false;

	bool expandsRightward = false;
	bool expandsLeftward = false;
	
	int panLaw = 0;
	simd::float_4 trackPanLeft = 0.5f;
	simd::float_4 trackPanRight = 0.5f;
	
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
	
    void updateGains() {
		for (unsigned int i = 0; i < 4; i++) {
			for (unsigned int j = 0; j < 4; j++) {
				unsigned int p = (i * 2) + (j / 2); //0 0 1 1 2 2 ...
				float mute_value = this->params[TModule::MUTE_PARAMS + p].getValue();
				float gain_value = this->params[TModule::GAIN_PARAMS + p].getValue();
				ilGains[i][j] = mute_value > 0 ? 0.f : gain_value;
				if (i < 2) { 
					unsigned int p = (i * 4) + j; //0 1 2 3 4 ...
					float mute_value = this->params[TModule::MUTE_PARAMS + p].getValue();
					float gain_value = this->params[TModule::GAIN_PARAMS + p].getValue();
					monoGains[i][j] =  mute_value > 0 ? 0.f : gain_value;
					this->lights[TModule::MUTE_LIGHTS + p].setBrightness(mute_value);
				}
			}
		}
		this->lights[TModule::SOLO_LIGHT].setBrightness(soloMe = this->params[TModule::SOLO_PARAM].getValue());
		this->lights[TModule::MUTE_LIGHT].setBrightness(muteMe = this->params[TModule::MUTE_PARAM].getValue());
		this->trackPanLeft = params[TModule::RETURN_PAN_PARAM].getValue();
		this->trackPanRight = 1.f - this->trackPanLeft;
	} //updateGains
	
	virtual inline bool calcLeftExpansion() {
		return leftExpander.module && (leftExpander.module->model == modelGainsSendsReturns || leftExpander.module->model == modelInsOutsGains);
	}

	virtual inline bool calcRightExpansion() {
		return rightExpander.module && rightExpander.module->model == modelGainsSendsReturns;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "panLaw", json_integer(panLaw));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* panLawJ = json_object_get(rootJ, "panLaw");
		if (panLawJ)
			panLaw = json_integer_value(panLawJ);
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
			addParam(createLightParamCentered<ClearLightLatch<SmallLight<RedLight>>>(Vec(xpos, mm2px(12+(15*i))), module, TModule::MUTE_PARAMS + i, TModule::MUTE_LIGHTS + i));
		}
		
		// Last column: Sends & Returns (also wants fader, mute, solo, pan(?), vu meters?? a la mindmeld auxpander)
		xpos = box.pos.x + box.size.x - 23.5;
		addOutput(createOutputCentered<WhiteRedPJ301MPort>(Vec(xpos, box.pos.y + mm2px(12)), module, TModule::INTERLEAVED_SEND));
		addOutput(createOutputCentered<WhitePJ301MPort>(Vec(xpos, box.pos.y + mm2px(21)), module, TModule::LEFT_SEND));
		addOutput(createOutputCentered<RedPJ301MPort>(Vec(xpos, box.pos.y + mm2px(30)), module, TModule::RIGHT_SEND));
		
		addInput(createInputCentered<WhiteRedPJ301MPort>(Vec(xpos, box.pos.y + mm2px(43)), module, TModule::INTERLEAVED_RETURN));
		addInput(createInputCentered<WhitePJ301MPort>(Vec(xpos, box.pos.y + mm2px(52)), module, TModule::LEFT_RETURN));
		addInput(createInputCentered<RedPJ301MPort>(Vec(xpos, box.pos.y + mm2px(61)), module, TModule::RIGHT_RETURN));

		addParam(createParamCentered<ChickenHeadKnobIvory>(Vec(xpos, mm2px(12+(15*4))), module, TModule::RETURN_PAN_PARAM));
		addParam(createParamCentered<VCVSlider>(Vec(xpos, box.pos.y + mm2px(93)), module, TModule::RETURN_GAIN_PARAM));
		//addParam(createLightParamCentered<VCVLightBezelLatch<MediumLight<GreenLight>>>(Vec(xpos, box.pos.y + mm2px(105)), module, TModule::SOLO_PARAM, TModule::SOLO_LIGHT));
		//addParam(createLightParamCentered<VCVLightBezelLatch<MediumLight<RedLight>>>(Vec(xpos, box.pos.y + mm2px(114)), module, TModule::MUTE_PARAM, TModule::MUTE_LIGHT));
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
		menu->addChild(new MenuSeparator);
		menu->addChild(createIndexPtrSubmenuItem("Pan law", {"-6 dB (linear)", "-3 dB"}, &module->panLaw));
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

			module->updateGains();
		}
		ModuleWidget::step();
	}
}; // Aux8Widget


