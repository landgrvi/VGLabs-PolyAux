#include "plugin.hpp"
#include "InsOutsGains.hpp"
#include "Outs.hpp"
#include <window/Window.hpp>
#include <window/Svg.hpp>

using namespace rack;
using namespace window;

//InsOutsGains : Module declared in InsOutsGains.hpp, base clase in Aux8.hpp

InsOutsGains::InsOutsGains() {
//common stuff happens in Aux8 constructor

	configParam(MASTER_PAN_PARAM, -1.f, 1.f, 0.f, "Master pan", "%", 0, 100);
	configParam(MASTER_GAIN_PARAM, 0.f, M_SQRT2, 1.f, "Master level", " dB", -10, 40);
	configParam(MASTER_MUTE_PARAM, 0.f, 1.f, 0.f, "Master mute");
	configParam(DRYPLUS_PARAM, 0.f, 1.f, 1.f, "Dry/Wet mix", "%", 0.f, 100.f);
	configInput(INTERLEAVED_INPUT, "Interleaved");
	configInput(LEFT_INPUT, "Left");
	configInput(RIGHT_INPUT, "Right");
	configOutput(INTERLEAVED_WET_OUTPUT, "Interleaved wet");
	configOutput(LEFT_WET_OUTPUT, "Left wet");
	configOutput(RIGHT_WET_OUTPUT, "Right wet");

	firstInput.setPorts(&inputs[INTERLEAVED_INPUT], &inputs[LEFT_INPUT], &inputs[RIGHT_INPUT]);
	wetOutput.setPorts(&outputs[INTERLEAVED_WET_OUTPUT], &outputs[LEFT_WET_OUTPUT], &outputs[RIGHT_WET_OUTPUT]);
	wetInput.clearAudio();

	firstWithSend.setPorts(&firstInput, &sendOutput);
	returnWithWet.setPorts(&returnInput, &wetOutput);
}

void InsOutsGains::process(const ProcessArgs &args) {
	
	if (((args.frame + this->id) % 64) == 0) updateGains();

	blinkPhase += args.sampleTime;
	if (blinkPhase >= 0.5f) {
		blinkPhase -= 0.5f;
		//DEBUG("%i %i %i %i", inChans, lChans, rChans, tsChans);
		//DEBUG("%d %d %d", pregainOutput.ilChannels, pregainOutput.leftChannels, pregainOutput.rightChannels);
		//DEBUG("%" PRId64 " muteMe: %i soloMe:%i soloTracks:%i soloToRight:%i numModules:%i", id, muteMe, soloMe, soloTracks, soloToRight, numModules);
		//DEBUG("wetOutput: %f   firstInput: %f", wetOutput.ilAudio[0][3], firstInput.ilAudio[0][3]);
		//DEBUG("%" PRId64 " numModules:%i", id, numModules);
		//DEBUG("%" PRId64, id);
		//DEBUG("%f %f", debugValue1, debugValue2);
		//DEBUG("%" PRId64 " copyMono: %d", id, firstInput.polyAuxDbg);
		//updateGains();
		//DEBUG("%li", svgCacheByTheme.size());
	}

	firstInput.pullAudio(true, monoInputMode);
	sendOutput.setChannelsFromInput(&firstInput);

	firstWithSend.gainAudio(monoGains);

//	returnInput.pullAudio(!muteMe && (soloMe || (soloTracks == 0))); //broken, but why
	returnInput.pullAudio(((!muteMe && (soloMe || (soloTracks == 0))) ? true : false), monoInputMode);
	wetOutput.setChannelsFromInput(&returnInput);

	// set up details for the expander
	expandsRightward = calcRightExpansion(); // rightExpander.module && (rightExpander.module->model == modelGainsSendsReturns);
	expandsLeftward = calcLeftExpansion();

	if (expandsRightward) {
		expMessage* rightSink = (expMessage*)(rightExpander.module->leftExpander.producerMessage); // this is the rightward module's; I write to it and request flip
		expMessage* rightSource = (expMessage*)(rightExpander.consumerMessage); // this is mine; my rightExpander.producer message is written by the rightward module, which requests flip
		
		for (unsigned int i = 0; i < 4; i++) {
			rightSink->pregainAudio[i] = firstInput.allAudio[i];
		}
		rightSink->pregainChans = firstInput.ilChannels;
		wetInput.setAudio(rightSource->wetAudio);
		wetInput.setChannels(rightSource->wetChans);
		wetOutput.setChannels(std::max(wetInput.ilChannels, wetOutput.ilChannels), std::max(wetInput.leftChannels, wetOutput.leftChannels), std::max(wetInput.rightChannels, wetOutput.rightChannels));
		returnAndFirstWithWet.setPorts(&returnInput, &firstInput, &wetOutput, &wetInput);
		returnAndFirstWithWet.blendAudio(params[DRYPLUS_PARAM].getValue());

		soloToRight = params[SOLO_PARAM].getValue();
		numMe = 1;
		rightSink->soloSoFar = soloToRight;
		rightSink->numModulesSoFar = numMe;
		rightSink->masterPanMode = masterPanMode;
		soloTracks = rightSource->soloTotal;
		numModules = rightSource->numModulesTotal;
		
		rightExpander.module->leftExpander.messageFlipRequested = true; // request rightward module to flip its leftExpander, as I've now written to its producer
	} else {
		returnAndFirstWithWet.setPorts(&returnInput, &firstInput, &wetOutput);
		returnAndFirstWithWet.blendAudio(params[DRYPLUS_PARAM].getValue());
		soloTracks = params[SOLO_PARAM].getValue();
		numModules = 1;
	}
	
	if (expandsLeftward) {
		expMessage* leftSink = (expMessage*)(leftExpander.module->rightExpander.producerMessage); // this is the left module's; I write to it and request flip
		for (unsigned int i = 0; i < 4; i++) {
			leftSink->pregainAudio[i] = firstInput.allAudio[i];
		}
		leftSink->pregainChans = firstInput.ilChannels;
		if (wetOutput.scales) {
			for (unsigned int i = 0; i < 2; i++) {
				leftSink->wetAudio[i] = wetOutput.leftAudio[i] * wetOutput.scalingVals[0] + wetOutput.rightAudio[i] * wetOutput.scalingVals[1];
				leftSink->wetAudio[i+2] = wetOutput.rightAudio[i] * wetOutput.scalingVals[3] + wetOutput.leftAudio[i] * wetOutput.scalingVals[2];
			}
		} else {
			for (unsigned int i = 0; i < 4; i++) {
				leftSink->wetAudio[i] = wetOutput.allAudio[i];
			}
		}
		leftSink->wetChans = std::max(wetOutput.ilChannels, returnInput.ilChannels);
		leftExpander.module->rightExpander.messageFlipRequested = true; // request left module to flip its rightExpander, as I've now written to its producer
	}
} //process

void InsOutsGains::updateGains() {
	Aux8::updateGains();
	bool changed = false;
	float pv = params[MASTER_PAN_PARAM].getValue();
	if (pv != oldMasterPanVal || masterPanMode != oldMasterPanMode) {
		changed = true;
		oldMasterPanVal = pv;
		oldMasterPanMode = masterPanMode;
		setPanVals(masterPanVals, pv, masterPanMode);
	}
	float ml = params[MASTER_GAIN_PARAM].getValue();
	float mute = params[MASTER_MUTE_PARAM].getValue();
	if (ml != oldMasterLevel || mute != oldMute) {
		changed = true;
		oldMasterLevel = ml;
		oldMute = mute;
	}
	if (changed) {
		wetOutput.setScaling(masterPanVals, ml * (1.f - mute));
	}
}

bool InsOutsGains::calcLeftExpansion() {
	return leftExpander.module && leftExpander.module->model == modelOuts;
}

json_t* InsOutsGains::dataToJson() {
	json_t* rootJ = Aux8<InsOutsGains>::dataToJson();
	json_object_set_new(rootJ, "masterPanMode", json_integer(masterPanMode));
	json_object_set_new(rootJ, "trackLabels", json_string(trackLabelChars));
	return rootJ;
}

void InsOutsGains::dataFromJson(json_t* rootJ) {
	Aux8<InsOutsGains>::dataFromJson(rootJ);
	json_t* masterPanModeJ = json_object_get(rootJ, "masterPanMode");
	if (masterPanModeJ)	masterPanMode = json_integer_value(masterPanModeJ);
	json_t* trackLabelsJ = json_object_get(rootJ, "trackLabels");
	if (trackLabelsJ) sprintf(trackLabelChars, "%32s", json_string_value(trackLabelsJ));
}

void InsOutsGains::onReset(const ResetEvent& e) {
	masterPanMode = 3;
	strncpy(trackLabelChars,"-01--02--03--04--05--06--07--08-", 33);
	loadLabels = true;
	Aux8::onReset(e);
}	

InsOutsGainsWidget::InsOutsGainsWidget(InsOutsGains* module) : Aux8Widget<InsOutsGains>(module, "res/InsOutsGains_11hp_Plus.svg") {
	// Column 1: Inputs, outputs, dry inmix knob
	float xpos = 9;
	float ypos = 13.4;
	addInput(createInputCentered<WhiteRedPJ301MPort>(mm2px(Vec(xpos, ypos)), module, InsOutsGains::INTERLEAVED_INPUT));
	addInput(createInputCentered<WhitePJ301MPort>(mm2px(Vec(xpos, ypos + 9)), module, InsOutsGains::LEFT_INPUT));
	addInput(createInputCentered<RedPJ301MPort>(mm2px(Vec(xpos, ypos + 18)), module, InsOutsGains::RIGHT_INPUT));

	ypos = 15;
	addOutput(createOutputCentered<WhiteRedPJ301MPort>(mm2px(Vec(xpos, ypos + 32)), module, InsOutsGains::INTERLEAVED_WET_OUTPUT));
	addOutput(createOutputCentered<WhitePJ301MPort>(mm2px(Vec(xpos, ypos + 41)), module, InsOutsGains::LEFT_WET_OUTPUT));
	addOutput(createOutputCentered<RedPJ301MPort>(mm2px(Vec(xpos, ypos + 50)), module, InsOutsGains::RIGHT_WET_OUTPUT));
	
	addParam(createParamCentered<ChickenHeadKnobIvory>(mm2px(Vec(xpos, ypos + 65)), module, InsOutsGains::MASTER_PAN_PARAM));
	addParam(createParamCentered<VGLabsSlider>(mm2px(Vec(xpos, 97)), module, InsOutsGains::MASTER_GAIN_PARAM));
	addParam(createParamCentered<btnMute>(mm2px(Vec(xpos, 110)), module, InsOutsGains::MASTER_MUTE_PARAM));
	addParam(createParamCentered<ChickenHeadKnobIvory>(mm2px(Vec(xpos, 120)), module, InsOutsGains::DRYPLUS_PARAM));

	//Column 2: Track labels
	for (unsigned int i = 0; i < 8; i++) {
		LedDisplayLimitedTextField<InsOutsGains>* d = createWidget<LedDisplayLimitedTextField<InsOutsGains>>(mm2px(Vec(20,12)));
		d->module = module ? module : nullptr;
		d->index = i;
		d->box.size = Vec(28, 15);
		d->textOffset = Vec(-2, -2);
		d->box.pos = mm2px(Vec(18, 12.1 + 14 * i));
		d->color = color::GREEN;
		d->bgColor = VGLABSPURPLE;
		d->text = "ABCD";
		if (module) {
			d->text.assign(&(module->trackLabelChars[i * 4]), 4);
			module->trackLabels[i] = d;
		}
		addChild(d);
	}
	// Columns 3&4 handled by Aux8Widget constructor

	// The preferred procedure is to subclass any widget you want to theme,
	// implementing IApplyTheme (which is quite simple to do in most cases),
	// and use this helper to apply the theme to the widget hierarchy.
	if (my_module) {
		auto themes = my_module->getThemes();
		auto theme = my_module->getTheme();
		auto svg_theme = themes.getTheme(theme);
		if (svg_theme) ApplyChildrenTheme(this, themes, svg_theme);
	}


}

void InsOutsGainsWidget::appendContextMenu(Menu* menu) {
	InsOutsGains* module = getModule<InsOutsGains>();
	menu->addChild(new MenuSeparator);
	menu->addChild(createIndexPtrSubmenuItem("Master Pan", {"True Pan (L + R)", "Linear Attenuation", "3dB boost (constant power)", "4.5dB Boost (compromise, default)", "6dB Boost (linear)"}, &module->masterPanMode));
	Aux8Widget<InsOutsGains>::appendContextMenu(menu);
}

void InsOutsGainsWidget::step() {
	if (module) {
		InsOutsGains* module = static_cast<InsOutsGains*>(this->module);
		if (oldModules != module->numModules) {
			//labelTotal->text = std::to_string(module->numModules);
			oldModules = module->numModules;
		}
		if (module->loadLabels) {
			for (unsigned int i = 0; i < 8; i++) {
				module->trackLabels[i]->text.assign(&(module->trackLabelChars[i * 4]), 4);
			}
			module->loadLabels = false;
		}
	}
	Aux8Widget::step();
}

Model* modelInsOutsGains = createModel<InsOutsGains, InsOutsGainsWidget>("InsOutsGains");

