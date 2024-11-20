#pragma once

#include "plugin.hpp"
#include "InsOutsGains.hpp"

using namespace rack;

struct Outs : Module {
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
	comboAudio wetAudio; // from right module, (todo: to output ports)
	
    Outs() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		char strBuf[32];
		configOutput(INTERLEAVED_PREGAIN_OUTPUT, "Interleaved pre-gain");
		configOutput(LEFT_PREGAIN_OUTPUT, "Left pre-gain");
		configOutput(RIGHT_PREGAIN_OUTPUT, "Right pre-gain");
		pregainOutput.setPorts(&outputs[INTERLEAVED_PREGAIN_OUTPUT], &outputs[LEFT_PREGAIN_OUTPUT], &outputs[RIGHT_PREGAIN_OUTPUT]);
		
		for (unsigned int i = 0; i < 16; i += 2) {
			snprintf(strBuf, 32, "Left wet %d", i / 2 + 1);
			configOutput(WET_OUTPUTS + i, strBuf);
			snprintf(strBuf, 32, "Right wet %d", i / 2 + 1);
			configOutput(WET_OUTPUTS + i + 1, strBuf);
		}
		//wetOutput.setPorts(&inputs[TModule::INTERLEAVED_RETURN], &inputs[TModule::LEFT_RETURN], &inputs[TModule::RIGHT_RETURN]);
		
		rightExpander.producerMessage = rightMessages[0];
		rightExpander.consumerMessage = rightMessages[1];

	} // Outs constructor
	
	virtual inline bool calcLeftExpansion() {
		return false;
	}

	virtual inline bool calcRightExpansion() {
		return rightExpander.module && rightExpander.module->model == modelInsOutsGains;
	}

	void process(const ProcessArgs &args) override  {
		expandsRightward = calcRightExpansion();
		if (expandsRightward) {
			expMessage* rightSource = (expMessage*)(rightExpander.consumerMessage); // this is mine; my rightExpander.producer message is written by the rightward module, which requests flip
			pregainOutput.setAudio(rightSource->pregainAudio);
			pregainOutput.setChannels(rightSource->pregainChans);
			wetAudio.setAudio(rightSource->wetAudio);
		}
		else {
			pregainOutput.clearAudio();
		}
		pregainOutput.pushAudio();
		unsigned int c = 0;
		for (unsigned int i = 0; i < 2; i++) {
			for (unsigned int j = 0; j < 4; j++) {
				outputs[WET_OUTPUTS + c++].setVoltage(wetAudio.leftAudio[i][j]);
				outputs[WET_OUTPUTS + c++].setVoltage(wetAudio.rightAudio[i][j]);
			}
		}
				
	} // process
	
}; // Outs

struct OutsWidget : ModuleWidget {
	
	PanelBorder* panelBorder;

	OutsWidget(Outs* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Outs_4hp_Plus.svg")));
		SvgPanel* svgPanel = static_cast<SvgPanel*>(getPanel());
		panelBorder = findBorder(svgPanel->fb);
		addOutput(createOutputCentered<WhiteRedPJ301MPort>(mm2px(Vec(10.2, 12)), module, Outs::INTERLEAVED_PREGAIN_OUTPUT));
		addOutput(createOutputCentered<WhitePJ301MPort>(mm2px(Vec(10.2, 21)), module, Outs::LEFT_PREGAIN_OUTPUT));
		addOutput(createOutputCentered<RedPJ301MPort>(mm2px(Vec(10.2, 30)), module, Outs::RIGHT_PREGAIN_OUTPUT));
		for (unsigned int i = 0; i < 16; i += 2) {
			addOutput(createOutputCentered<WhitePJ301MPort>(mm2px(Vec(6, 43 + (i * 4.5))), module, Outs::WET_OUTPUTS + i));
			addOutput(createOutputCentered<RedPJ301MPort>(mm2px(Vec(14.4, 43 + (i * 4.5))), module, Outs::WET_OUTPUTS + i + 1));
		}
	}

	void draw(const DrawArgs& args) override {
		if (module) {
			Outs* module = static_cast<Outs*>(this->module);
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
	
	void step() override {
		if (module) {
			Outs* module = static_cast<Outs*>(this->module);
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
		ModuleWidget::step();
	}
}; // OutsWidget

Model* modelOuts = createModel<Outs, OutsWidget>("Outs");
