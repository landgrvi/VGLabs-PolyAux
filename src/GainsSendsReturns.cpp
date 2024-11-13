#include "plugin.hpp"
#include "GainsSendsReturns.hpp"

using namespace rack;

GainsSendsReturns::GainsSendsReturns() {
	pregainWithSend.setPorts(&pregainAudio, &sendOutput);
	returnWithWet.setPorts(&returnInput, &wetAudio);
}

void GainsSendsReturns::process(const ProcessArgs &args) {
	
	debugValue = 0.f;

	expandsLeftward = leftExpander.module && (leftExpander.module->model == modelInsOutsGains || leftExpander.module->model == modelGainsSendsReturns);
	expandsRightward = rightExpander.module && (rightExpander.module->model == modelGainsSendsReturns);

	expMessage* rightSink = expandsRightward ? (expMessage*)(rightExpander.module->leftExpander.producerMessage) : nullptr; // this is the rightward module's; I write to it and request flip
	expMessage* rightSource = (expMessage*)(rightExpander.consumerMessage); // this is mine; my rightExpander.producer message is written by the rightward module, which requests flip
	expMessage* leftSink = expandsLeftward ? (expMessage*)(leftExpander.module->rightExpander.producerMessage) : nullptr; // this is the leftward module's; I write to it and request flip
	expMessage* leftSource = (expMessage*)(leftExpander.consumerMessage); // this is mine; my leftExpander.producer message is written by the leftward module, which requests flip

	
/* This should all be ordered to logically process the audio:
 * get pregain from left (if there's a left)
 * send pregain to right (if there's a right)
 * apply gains and push audio to send ports
 * get wet from right (if there's a right)
 * pull audio from return ports
 * apply fader and add return to wet
 * send wet to left (if there's a left)
 * 
 * And for solos & number of modules:
 * get info from left (if there's a left)
 * adjust as needed
 * send info to right (if there's a right)
 * get total from right (if there's a right)
 * send total to left (if there's a left)
 * 
 * fliprequest right & left
*/

	if (expandsLeftward) {
		pregainAudio.setAudio(leftSource->pregainAudio);
		pregainAudio.setChannels(leftSource->pregainChans);
		soloToRight = leftSource->soloSoFar + params[SOLO_PARAM].getValue();
		numMe = leftSource->numModulesSoFar + 1;
	} else { // I'm the leftmost. Don't keep old crap!
		pregainAudio.clearAudio(); 
		soloToRight = params[SOLO_PARAM].getValue();
		numMe = 1;
	}

	if (expandsRightward) {
		for (unsigned int i = 0; i < 8; i++) {
			rightSink->pregainAudio[i] = pregainAudio.allAudio[i];
		}
		rightSink->pregainChans = pregainAudio.ilChannels;
		rightSink->soloSoFar = soloToRight;
		rightSink->numModulesSoFar = numMe;
		soloTracks = rightSource->soloTotal; // total from rightmost module
		numModules = rightSource->numModulesTotal; // total from rightmost module
	} else { // I'm the rightmost. Close the loop!
		soloTracks = soloToRight;
		numModules = numMe;
	}

	sendOutput.setChannelsFromInput(&pregainAudio);
	pregainWithSend.gainAudio(ilGains, monoGains);

	//if there's no leftward module, no reason to process return audio or to load wet audio since there's nowhere to send it
	if (expandsLeftward) {
		returnInput.pullAudio(!muteMe && (soloMe || (soloTracks == 0)));
		
		if (expandsRightward) {
			wetAudio.setAudio(rightSource->wetAudio);
			wetAudio.setChannels(std::max(returnInput.ilChannels, rightSource->wetChans));
		} else { // I'm the rightmost. Start with a clean slate!
			wetAudio.clearAudio();
			wetAudio.setChannels(returnInput.ilChannels);
		}
		returnWithWet.addLevelAudio(params[RETURN_GAIN].getValue());
		for (unsigned int i = 0; i < 8; i++) {
			leftSink->wetAudio[i] = wetAudio.allAudio[i];
		}
		leftSink->wetChans = wetAudio.ilChannels;
		leftSink->soloTotal = soloTracks; // total goes left
		leftSink->numModulesTotal = numModules; // total goes left
	}
	
	if (expandsRightward) {
		rightExpander.module->leftExpander.messageFlipRequested = true; // tell the rightward module it can flip, putting the producer I wrote to in its consumer
	}
	if (expandsLeftward) {
		leftExpander.module->rightExpander.messageFlipRequested = true; // tell the leftward module it can flip, putting the producer I wrote to in its consumer
	}

	blinkPhase += args.sampleTime;
	if (blinkPhase >= 0.5f) {
		blinkPhase -= 0.5f;
		//DEBUG("%i %i %i %i", sendOutput.ilChannels, sendOutput.leftChannels, sendOutput.rightChannels, argggggh);
		//DEBUG("%li soloMe:%i soloTracks:%i soloToRight:%i numModules:%i", id, soloMe, soloTracks, soloToRight, numModules);
		//DEBUG("%f", wetOutput.ilAudio[0][3]);
		//DEBUG("%li %f", id, debugValue);
		//DEBUG("%li numModules:%i", id, numModules);
	}

} //process

struct GainsSendsReturnsWidget : Aux8Widget<GainsSendsReturns> {
	
	GainsSendsReturnsWidget(GainsSendsReturns* module) : Aux8Widget<GainsSendsReturns>(module, "res/GainsSendsReturns_6hp_Plus.svg") { }

};

Model* modelGainsSendsReturns = createModel<GainsSendsReturns, GainsSendsReturnsWidget>("GainsSendsReturns");

