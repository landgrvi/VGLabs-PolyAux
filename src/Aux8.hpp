#pragma once

#include "plugin.hpp"
#include "PachdeThemedModule.hpp"

using namespace rack;

template<typename TModule>
struct Aux8 : PachdeThemedModule {

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
	unsigned int monoInputMode = 0;
	
	comboAudioOut sendOutput; // to effect
	comboAudioIn returnInput; // from effect 

	expMessage leftMessages[2][1]; // messages to & from left module
	expMessage rightMessages[2][1]; // messages to & from right module
	
    Aux8() : PachdeThemedModule("res/themes.json", "VGLabs/PolyAux", "BlueGreenPurple") {
		config(TModule::PARAMS_LEN, TModule::INPUTS_LEN, TModule::OUTPUTS_LEN, TModule::LIGHTS_LEN);
		char strBuf[32];
		for (unsigned int i = 0; i < 8; i++) {
			snprintf(strBuf, 32, "Gain track %d", i + 1);
			configParam(TModule::GAIN_PARAMS + i, 0.f, 2.f, 1.f, strBuf, " dB", -10, 40.f); 
			snprintf(strBuf, 32, "Mute track %d", i + 1);
			configParam(TModule::MUTE_PARAMS + i, 0.f, 1.f, 0.f, strBuf);
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
		//probably should be done by a button event?
		soloMe = params[TModule::SOLO_PARAM].getValue();
		muteMe = params[TModule::MUTE_PARAM].getValue();

		lights[TModule::LEFT_RETURN_LIGHT].setBrightness(inputs[TModule::LEFT_RETURN].getChannels() > 8 ? 1 : 0);
		lights[TModule::RIGHT_RETURN_LIGHT].setBrightness(inputs[TModule::RIGHT_RETURN].getChannels() > 8 ? 1 : 0);
		
	} //updateGains
	
	virtual inline bool calcLeftExpansion() {
		return leftExpander.module && (leftExpander.module->model == modelLoop8 || leftExpander.module->model == modelBaseLoop8);
	}

	virtual inline bool calcRightExpansion() {
		return rightExpander.module && rightExpander.module->model == modelLoop8;
	}

	json_t* dataToJson() override {
		json_t* rootJ = PachdeThemedModule::dataToJson();
		json_object_set_new(rootJ, "returnPanMode", json_integer(returnPanMode));
		json_object_set_new(rootJ, "monoInputMode", json_integer(monoInputMode));
		return rootJ;
	} // dataToJson

	void dataFromJson(json_t* rootJ) override {
		PachdeThemedModule::dataFromJson(rootJ);
		json_t* returnPanModeJ = json_object_get(rootJ, "returnPanMode");
		if (returnPanModeJ)
			returnPanMode = json_integer_value(returnPanModeJ);
		json_t* monoInputModeJ = json_object_get(rootJ, "monoInputMode");
		if (monoInputModeJ)
			monoInputMode = json_integer_value(monoInputModeJ);
	} // dataFromJson

	void onReset(const ResetEvent& e) override {
		Module::onReset(e);
		returnPanMode = 0;
		monoInputMode = 0;
	} // onReset
}; // Aux8	

inline float px2mm(float f) { return f * MM_PER_IN / SVG_DPI; }

template<typename TModule>
struct Aux8Widget : PachdeThemedModuleWidget {
	
	PanelBorder* panelBorder;

	Aux8Widget(TModule* module, std::string panelFile) : PachdeThemedModuleWidget(module, panelFile) {
		setModule(module);
		SvgPanel* svgPanel = static_cast<SvgPanel*>(getPanel());
		panelBorder = findBorder(svgPanel->fb);
		
		// Penultimate column: Gains / mutes per channel
		float xpos = 0;
		float ypos = 15;
		xpos = px2mm(box.size.x - 65);
		for (unsigned int i = 0; i < 8; i++) {
			addParam(createParamCentered<ChickenHeadKnobIvory>(mm2px(Vec(xpos, ypos+(14*i))), module, TModule::GAIN_PARAMS + i));
			addParam(createLightParamCentered<GreenRedLightLatch>(mm2px(Vec(xpos, ypos+(14*i))), module, TModule::MUTE_PARAMS + i, TModule::MUTE_LIGHTS + (i * 2)));
		}
		
		// Last column: Sends & Returns
		xpos = px2mm(box.size.x - 28);
		ypos = 13.4;
		addOutput(createOutputCentered<WhiteRedPJ301MPort>(mm2px(Vec(xpos, ypos)), module, TModule::INTERLEAVED_SEND));
		addOutput(createOutputCentered<WhitePJ301MPort>(mm2px(Vec(xpos, ypos + 9)), module, TModule::LEFT_SEND));
		addOutput(createOutputCentered<RedPJ301MPort>(mm2px(Vec(xpos, ypos + 18)), module, TModule::RIGHT_SEND));
		
		ypos = 15;
		addInput(createInputCentered<WhiteRedPJ301MPort>(mm2px(Vec(xpos, ypos + 32)), module, TModule::INTERLEAVED_RETURN));
		addInput(createInputCentered<WhitePJ301MPort>(mm2px(Vec(xpos, ypos + 41)), module, TModule::LEFT_RETURN));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xpos + 3.8, ypos + 44.4)), module, TModule::LEFT_RETURN_LIGHT));
		addInput(createInputCentered<RedPJ301MPort>(mm2px(Vec(xpos, ypos + 50)), module, TModule::RIGHT_RETURN));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xpos + 3.8, ypos + 53.4)), module, TModule::RIGHT_RETURN_LIGHT));

		addParam(createParamCentered<ChickenHeadKnobIvory>(mm2px(Vec(xpos, ypos + 65)), module, TModule::RETURN_PAN_PARAM));
		addParam(createParamCentered<VGLabsSlider>(mm2px(Vec(xpos, 97)), module, TModule::RETURN_GAIN_PARAM));
		addParam(createParamCentered<btnMute>(mm2px(Vec(xpos, 110)), module, TModule::MUTE_PARAM));
		addParam(createParamCentered<btnSolo>(mm2px(Vec(xpos, 116)), module, TModule::SOLO_PARAM));

        // Apply theme to components
		if (my_module) {
			auto themes = my_module->getThemes();
			auto theme = my_module->getTheme();
			auto svg_theme = themes.getTheme(theme);
			if (svg_theme) ApplyChildrenTheme(this, themes, svg_theme);
		}
	} // constructor

	void draw(const DrawArgs& args) override {
		if (module) {
			TModule* module = static_cast<TModule*>(this->module);
			if (module->calcRightExpansion()) {
				DrawArgs newDrawArgs = args;
				newDrawArgs.clipBox.size.x += mm2px(0.3f); // panels have their base rectangle this much larger, to kill gap artifacts
				PachdeThemedModuleWidget::draw(newDrawArgs);
			} else {
				PachdeThemedModuleWidget::draw(args);
			}
		} else {
			PachdeThemedModuleWidget::draw(args);
		}
	} // draw
	
	void appendContextMenu(Menu* menu) override {
		TModule* module = getModule<TModule>();
		if (module->model != modelBaseLoop8) menu->addChild(new MenuSeparator);
		menu->addChild(createIndexPtrSubmenuItem("Return Pan", {"Use Master (default)", "True Pan (L + R)", "Linear Attenuation", "3dB boost (constant power)", "4.5dB Boost (compromise)", "6dB Boost (linear)"}, &module->returnPanMode));
		menu->addChild(createIndexPtrSubmenuItem("Mono Input", {"Copy L to R (default)", "Do Nothing"}, &module->monoInputMode));
		PachdeThemedModuleWidget::appendContextMenu(menu);
	} // appendContextMenu
	
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
		}
		PachdeThemedModuleWidget::step();
	} // step
}; // Aux8Widget

