#include "plugin.hpp"
#include "Loop8.hpp"

using namespace rack;

Loop8::Loop8() {
	dryWithSend.setPorts(&dryAudio, &sendOutput);
	returnWithWet.setPorts(&returnInput, &wetAudio);
}

void Loop8::process(const ProcessArgs &args) {
	
	if (((args.frame + this->id) % 64) == 0) updateGains();
	
	debugValue = 0.f;

	expandsLeftward = leftExpander.module && (leftExpander.module->model == modelBaseLoop8 || leftExpander.module->model == modelLoop8);
	expandsRightward = rightExpander.module && (rightExpander.module->model == modelLoop8);

	expMessage* rightSink = expandsRightward ? (expMessage*)(rightExpander.module->leftExpander.producerMessage) : nullptr; // this is the rightward module's; I write to it and request flip
	expMessage* rightSource = (expMessage*)(rightExpander.consumerMessage); // this is mine; my rightExpander.producer message is written by the rightward module, which requests flip
	expMessage* leftSink = expandsLeftward ? (expMessage*)(leftExpander.module->rightExpander.producerMessage) : nullptr; // this is the leftward module's; I write to it and request flip
	expMessage* leftSource = (expMessage*)(leftExpander.consumerMessage); // this is mine; my leftExpander.producer message is written by the leftward module, which requests flip

	
/* This is ordered to logically process the audio:
 * get dry from left (if there's a left)
 * send dry unaltered to right (if there's a right)
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
		dryAudio.setAudio(leftSource->dryAudio);
		dryAudio.setChannels(leftSource->dryChans);
		soloToRight = leftSource->soloSoFar + params[SOLO_PARAM].getValue();
		numMe = leftSource->numModulesSoFar + 1;
		masterPanMode = leftSource->masterPanMode;
	} else { // I'm the leftmost. Don't keep old crap!
		dryAudio.clearAudio(); 
		soloToRight = params[SOLO_PARAM].getValue();
		numMe = 1;
		masterPanMode = 3;
	}

	if (expandsRightward) {
		for (unsigned int i = 0; i < 4; i++) {
			rightSink->dryAudio[i] = dryAudio.allAudio[i];
		}
		rightSink->dryChans = dryAudio.ilChannels;
		rightSink->soloSoFar = soloToRight;
		rightSink->numModulesSoFar = numMe;
		rightSink->masterPanMode = masterPanMode;
		rightSink->leftTheme = theme;
		soloTracks = rightSource->soloTotal; // total from rightmost module
		numModules = rightSource->numModulesTotal; // total from rightmost module
	} else { // I'm the rightmost. Close the loop.
		soloTracks = soloToRight;
		numModules = numMe;
	}

	sendOutput.setChannelsFromInput(&dryAudio);
	if (muteMe || (!soloMe && soloTracks > 0)) {
		for (unsigned int i = 0; i < 4; i++) {
			sendOutput.allAudio[i] = 0;
		}
		sendOutput.pushAudio();
	} else {
		dryWithSend.gainAudio(monoGains);
	}

	//if there's no leftward module, no reason to process return audio or to load wet audio since there's nowhere to send it
	if (expandsLeftward) {
		returnInput.pullAudio(((!muteMe && (soloMe || (soloTracks == 0))) ? true : false), 1 - monoInputMode);
		
		if (expandsRightward) {
			wetAudio.setAudio(rightSource->wetAudio);
			wetAudio.setChannels(std::max(returnInput.ilChannels, rightSource->wetChans));
		} else { // I'm the rightmost. Start with a clean slate!
			wetAudio.clearAudio();
			wetAudio.setChannels(returnInput.ilChannels);
		}
		returnWithWet.addAudio();
		for (unsigned int i = 0; i < 4; i++) {
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

	// This is from the tutorial module, and is a good place to put debug statements
	blinkPhase += args.sampleTime;
	if (blinkPhase >= 0.5f) {
		blinkPhase -= 0.5f;
	}
} //process


