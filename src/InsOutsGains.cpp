#include "plugin.hpp"
#include "InsOutsGains.hpp"
#include "Outs.hpp"

using namespace rack;

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
	firstWithPregain.setPorts(&firstInput, &pregainOutput);
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
		//updateGains();
	}

	firstInput.pullAudio();
	pregainOutput.setChannelsFromInput(&firstInput);
	sendOutput.setChannelsFromInput(&firstInput);

	firstWithPregain.copyAudio();
	firstWithSend.gainAudio(monoGains);

//	returnInput.pullAudio(!muteMe && (soloMe || (soloTracks == 0))); //broken, but why
//	if (!muteMe && (soloMe || (soloTracks == 0))) { returnInput.pullAudio(true); } else { returnInput.pullAudio(false); } //nominal
//	returnInput.pullAudio((!muteMe && (soloMe || (soloTracks == 0))) ? true : false); //nominal
	//float pans[4] = {1, 1, 0, 0};
	returnInput.pullAudio((!muteMe && (soloMe || (soloTracks == 0))) ? true : false);
	wetOutput.setChannelsFromInput(&returnInput);

	// set up details for the expander
	expandsRightward = calcRightExpansion(); // rightExpander.module && (rightExpander.module->model == modelGainsSendsReturns);
	expandsLeftward = calcLeftExpansion();

	if (expandsRightward) {
			
		expMessage* rightSink = (expMessage*)(rightExpander.module->leftExpander.producerMessage); // this is the rightward module's; I write to it and request flip
		expMessage* rightSource = (expMessage*)(rightExpander.consumerMessage); // this is mine; my rightExpander.producer message is written by the rightward module, which requests flip
		
		for (unsigned int i = 0; i < 4; i++) {
			rightSink->pregainAudio[i] = pregainOutput.allAudio[i];
		}
		rightSink->pregainChans = pregainOutput.ilChannels;
		wetInput.setAudio(rightSource->wetAudio);
		wetInput.setChannels(rightSource->wetChans);
		wetOutput.setChannels(std::max(wetInput.ilChannels, wetOutput.ilChannels), std::max(wetInput.leftChannels, wetOutput.leftChannels), std::max(wetInput.rightChannels, wetOutput.rightChannels));
		returnAndFirstWithWet.setPorts(&returnInput, &firstInput, &wetOutput, &wetInput);
		returnAndFirstWithWet.blendAudio(params[DRYPLUS_PARAM].getValue());
		//returnAndFirstWithWet.blendAudio(params[DRYPLUS_PARAM].getValue(), params[RETURN_GAIN_PARAM].getValue(), &wetInput, params[MASTER_GAIN_PARAM].getValue() * (1.f - params[MASTER_MUTE_PARAM].getValue()), masterPanVals );

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
		//returnAndFirstWithWet.blendAudio(params[DRYPLUS_PARAM].getValue(), params[RETURN_GAIN_PARAM].getValue(), params[MASTER_GAIN_PARAM].getValue() * (1.f - params[MASTER_MUTE_PARAM].getValue()), masterPanVals);
		soloTracks = params[SOLO_PARAM].getValue();
		numModules = 1;
	}
	
	if (expandsLeftward) {
		expMessage* leftSink = (expMessage*)(leftExpander.module->rightExpander.producerMessage); // this is the left module's; I write to it and request flip
		for (unsigned int i = 0; i < 4; i++) {
			leftSink->pregainAudio[i] = pregainOutput.allAudio[i];
		}
		leftSink->pregainChans = pregainOutput.ilChannels;
		for (unsigned int i = 0; i < 4; i++) {
			leftSink->wetAudio[i] = wetOutput.allAudio[i];
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
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "returnPanMode", json_integer(returnPanMode));
	json_object_set_new(rootJ, "masterPanMode", json_integer(masterPanMode));
	return rootJ;
}

void InsOutsGains::dataFromJson(json_t* rootJ) {
	json_t* returnPanModeJ = json_object_get(rootJ, "returnPanMode");
	if (returnPanModeJ)
		returnPanMode = json_integer_value(returnPanModeJ);
	json_t* masterPanModeJ = json_object_get(rootJ, "masterPanMode");
	if (masterPanModeJ)
		masterPanMode = json_integer_value(masterPanModeJ);
}

void InsOutsGains::onReset(const ResetEvent& e) {
	masterPanMode = 3;
	Aux8::onReset(e);
}	

		
struct InsOutsGainsWidget : Aux8Widget<InsOutsGains> {

	unsigned int oldModules = 0;
	Label* labelTotal;

	InsOutsGainsWidget(InsOutsGains* module) : Aux8Widget<InsOutsGains>(module, "res/InsOutsGains_9hp_Plus.svg") {
		
		// Columns 2 and 3 handled by Aux8Widget constructor

		// Column 1: Inputs, outputs, dry inmix knob

		addInput(createInputCentered<WhiteRedPJ301MPort>(mm2px(Vec(8.5, 12)), module, InsOutsGains::INTERLEAVED_INPUT));
		addInput(createInputCentered<WhitePJ301MPort>(mm2px(Vec(8.5, 21)), module, InsOutsGains::LEFT_INPUT));
		addInput(createInputCentered<RedPJ301MPort>(mm2px(Vec(8.5, 30)), module, InsOutsGains::RIGHT_INPUT));

		addOutput(createOutputCentered<WhiteRedPJ301MPort>(mm2px(Vec(8.5, 43)), module, InsOutsGains::INTERLEAVED_WET_OUTPUT));
		addOutput(createOutputCentered<WhitePJ301MPort>(mm2px(Vec(8.5, 52)), module, InsOutsGains::LEFT_WET_OUTPUT));
		addOutput(createOutputCentered<RedPJ301MPort>(mm2px(Vec(8.5, 61)), module, InsOutsGains::RIGHT_WET_OUTPUT));
		
		addParam(createParamCentered<ChickenHeadKnobIvory>(mm2px(Vec(8.5, 12+(15*4))), module, InsOutsGains::MASTER_PAN_PARAM));
		addParam(createParamCentered<VCVSlider>(mm2px(Vec(8.5, 93)), module, InsOutsGains::MASTER_GAIN_PARAM));
		addParam(createParamCentered<btnMute>(mm2px(Vec(8.5, 110)), module, InsOutsGains::MASTER_MUTE_PARAM));
		addParam(createParamCentered<ChickenHeadKnobIvory>(mm2px(Vec(8.5, 12+(15*7.2))), module, InsOutsGains::DRYPLUS_PARAM));
		//addParam(createLightParamCentered<TestButton<SmallSimpleLight<RedLight>>>(mm2px(Vec(8.5,12+(15*5))), module, InsOutsGains::TEST_PARAM, InsOutsGains::TEST_LIGHT));
		
		//addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.5, 99)), module, InsOutsGains::INTERLEAVED_PREGAIN_OUTPUT));
		//addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.5, 108)), module, InsOutsGains::LEFT_PREGAIN_OUTPUT));
		//addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.5, 117)), module, InsOutsGains::RIGHT_PREGAIN_OUTPUT));
		
/*
		labelTotal = new Label;
		labelTotal->box.size = Vec(50, 20);
		labelTotal->box.pos = mm2px(Vec(0 , 75));
		labelTotal->text = "foo";
		labelTotal->fontSize = 24;
		labelTotal->alignment = ui::Label::CENTER_ALIGNMENT;
		addChild(labelTotal);
*/
	}

	void appendContextMenu(Menu* menu) override {
		InsOutsGains* module = getModule<InsOutsGains>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createIndexPtrSubmenuItem("Master Pan Mode", {"True Pan (L + R)", "Linear Attenuation", "3dB boost (constant power)", "4.5dB Boost (compromise, default)", "6dB Boost (linear)"}, &module->masterPanMode));
		Aux8Widget<InsOutsGains>::appendContextMenu(menu);
	}
	
	
	void step() override {
		if (module) {
			InsOutsGains* module = static_cast<InsOutsGains*>(this->module);
			if (oldModules != module->numModules) {
				//labelTotal->text = std::to_string(module->numModules);
				oldModules = module->numModules;
			}
		}
		
		Aux8Widget::step();
	}
};

Model* modelInsOutsGains = createModel<InsOutsGains, InsOutsGainsWidget>("InsOutsGains");
