#include "plugin.hpp"
#include "InsOutsGains.hpp"
#include "Outs.hpp"

Outs::Outs() : PachdeThemedModule("res/themes.json") {
	//defaultTheme = "BlueGreenPurple";
	DEBUG("%s", defaultTheme.c_str());
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

void Outs::process(const ProcessArgs &args) {
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

OutsWidget::OutsWidget(Outs* module) : PachdeThemedModuleWidget(module, "res/Outs_4hp_Plus.svg") {
	setModule(module);
	SvgPanel* svgPanel = static_cast<SvgPanel*>(getPanel());
	panelBorder = findBorder(svgPanel->fb);
	float xpos = 10.2;
	float ypos = 13.4;
	addOutput(createOutputCentered<WhiteRedPJ301MPort>(mm2px(Vec(xpos, ypos)), module, Outs::INTERLEAVED_PREGAIN_OUTPUT));
	addOutput(createOutputCentered<WhitePJ301MPort>(mm2px(Vec(xpos, ypos + 9)), module, Outs::LEFT_PREGAIN_OUTPUT));
	addOutput(createOutputCentered<RedPJ301MPort>(mm2px(Vec(xpos, ypos + 18)), module, Outs::RIGHT_PREGAIN_OUTPUT));
	for (unsigned int i = 0; i < 16; i += 2) {
		addOutput(createOutputCentered<WhitePJ301MPort>(mm2px(Vec(6, 48.5 + (i * 4.5))), module, Outs::WET_OUTPUTS + i));
		addOutput(createOutputCentered<RedPJ301MPort>(mm2px(Vec(14.4, 48.5 + (i * 4.5))), module, Outs::WET_OUTPUTS + i + 1));
	}

} // OutsWidget constructor

void OutsWidget::draw(const DrawArgs& args) {
	if (module) {
		Outs* module = static_cast<Outs*>(this->module);
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

void OutsWidget::step()	{
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
	PachdeThemedModuleWidget::step();
}
	
Model* modelOuts = createModel<Outs, OutsWidget>("Outs");



	
