#pragma once

#include "plugin.hpp"

using namespace rack;

//Note: "il" means "interleaved": L R L R L R...

struct comboAudio {
	unsigned int ilChannels = 0;
	unsigned int leftChannels = 0;
	unsigned int rightChannels = 0;
	simd::float_4 allAudio[4] = { };
	//simd::float_4* ilAudio = &(allAudio[0]);
	simd::float_4* leftAudio = &(allAudio[0]);
	simd::float_4* rightAudio = &(allAudio[2]);
	bool hasPorts = false;
	Port* ilPort = nullptr;
	Port* leftPort = nullptr;
	Port* rightPort = nullptr;
	// Just in case: 
	float* ilAudioPtrs[16] = {&(leftAudio[0][0]), &(rightAudio[0][0]),
		                      &(leftAudio[0][1]), &(rightAudio[0][1]), 
							  &(leftAudio[0][2]), &(rightAudio[0][2]),		                     
							  &(leftAudio[0][3]), &(rightAudio[0][3]),		                     
							  &(leftAudio[1][0]), &(rightAudio[1][0]),		                     
							  &(leftAudio[1][1]), &(rightAudio[1][1]),		                     
							  &(leftAudio[1][2]), &(rightAudio[1][2]),		                     
							  &(leftAudio[1][3]), &(rightAudio[1][3])};
	
	void setPorts(Port* interleaved, Port* left, Port* right) {
		ilPort = interleaved;
		leftPort = left;
		rightPort = right;
		hasPorts = true;
	} //setPorts

	void setChannels(unsigned int ilc, unsigned int lc, unsigned int rc) {
		ilChannels = ilc;
		leftChannels = lc;
		rightChannels = rc;
		if (!hasPorts) return;
		ilPort->setChannels(ilc);
		leftPort->setChannels(lc);
		rightPort->setChannels(rc);
	} //setChannels	

	void setChannels(unsigned int ilc) {
		ilChannels = ilc;
		leftChannels = ilc / 2;
		rightChannels = ilc / 2;
		if (!hasPorts) return;
		ilPort->setChannels(ilc);
		leftPort->setChannels(ilc / 2);
		rightPort->setChannels(ilc / 2);
	} //setChannels	

	void clearAudio() {
		ilChannels = 0;
		leftChannels = 0;
		rightChannels = 0;
		for (unsigned int i = 0; i < 4; i++) {
			allAudio[i] = 0.f;
		}
		if (hasPorts) {
			ilPort->setChannels(0);  //also clears voltages; see rack::engine::Port Struct Reference
			leftPort->setChannels(0);
			rightPort->setChannels(0);
		}
	} //clearAudio
	
	void setAudio(simd::float_4* aa) {
		for (unsigned int i = 0; i < 4; i++) {
			allAudio[i] = aa[i];
		}
	} //setAudio
	
	void setAudio(simd::float_4* la, simd::float_4* ra) {
		for (unsigned int i = 0; i < 2; i++) {
			leftAudio[i] = la[i];
			rightAudio[i] = ra[i];
		}
	} //setAudio
	
	void reLevelAudio(float level) {
		for (unsigned int i = 0; i < 4; i++) {
			allAudio[i] *= simd::pow(level, 2);
		}
	} // reLevelAudio
	
	virtual void pushAudio() {}
}; //comboAudio

struct comboAudioIn : comboAudio {
	//we have the member variables and setPorts from comboAudio

	void updateChannels() {
		if (!hasPorts) return;
		ilChannels = std::min(ilPort->getChannels(), 16);
		leftChannels = std::min(leftPort->getChannels(), 8); //silently drops higher channels - sorry, user!
		rightChannels = std::min(rightPort->getChannels(), 8);
		if (ilChannels % 2) ilChannels++;
		ilChannels = std::max({leftChannels * 2, rightChannels * 2, ilChannels});
		leftChannels = rightChannels = ilChannels / 2;
		//ilPort->setChannels(ilChannels);
		//leftPort->setChannels(leftChannels);
		//rightPort->setChannels(rightChannels);
	} //updateChannels
	
	void pullAudio(bool trackEnabled = true) {
		if (!hasPorts) return;
		updateChannels(); //now ilChannels == leftChannels (or rightChannels) * 2
		if (trackEnabled) {
			float* ilp = ilPort->getVoltages();
			float* lp = leftPort->getVoltages();
			float* rp = rightPort->getVoltages();
			unsigned int c = 0;
			unsigned int mc = 0;
// i:  0               1
// j:  0   1   2   3   0   1   2   3
// c:  0 1 2 3 4 5 6 7 8 9 101112131415
// mc: 0   1   2   3   4   5   6   7
			for (unsigned int i = 0; i < 2; i++) {
				for (unsigned int j = 0; j < 4; j++) {
					leftAudio[i][j] = ilp[c++] + lp[mc]; // increment interleaved channel
					rightAudio[i][j] = ilp[c++] + rp[mc++]; // increment interleaved channel, mono channel
				}
			}
		} else {
			for (unsigned int i = 0; i < 4; i++) {
				allAudio[i] = 0.f;
			}
		}	
	} //pullAudio

	void pullAudio(bool trackEnabled, float pans[4]) {
		if (!hasPorts) return;
		updateChannels(); //now ilChannels == leftChannels (or rightChannels) * 2
		if (trackEnabled) {
			float* ilp = ilPort->getVoltages();
			float* lp = leftPort->getVoltages();
			float* rp = rightPort->getVoltages();
			float amtLeft = pans[0];
			float amtRightToLeft = pans[1];
			float amtLeftToRight = pans[2];
			float amtRight = pans[3];
			unsigned int c = 0;
			unsigned int mc = 0;
// i:  0               1
// j:  0   1   2   3   0   1   2   3
// c:  0 1 2 3 4 5 6 7 8 9 101112131415
// mc: 0   1   2   3   4   5   6   7
			for (unsigned int i = 0; i < 2; i++) {
				for (unsigned int j = 0; j < 4; j++) {
					leftAudio[i][j] = amtLeft * (ilp[c]  + lp[mc]) + amtRightToLeft * (rp[mc] + ilp[c + 1]);
					c++;  // increment interleaved channel
					rightAudio[i][j] = amtRight * (ilp[c]  + rp[mc]) + amtLeftToRight * (lp[mc] + ilp[c - 1]);
					c++;  // increment interleaved channel
					mc++; // increment mono channel
				}
			}
		} else {
			for (unsigned int i = 0; i < 4; i++) {
				allAudio[i] = 0.f;
			}
		}	
	} //pullAudio


}; //comboAudioIn

struct comboAudioOut : comboAudio {
	//we have the member variables and setPorts from comboAudio

template<typename TAudio>
	void setChannelsFromInput (TAudio* caIn) { 
		ilChannels = caIn->ilChannels;
		leftChannels = caIn->leftChannels;
		rightChannels = caIn->rightChannels;
		if (!hasPorts) return;
		ilPort->setChannels(ilChannels);
		leftPort->setChannels(leftChannels);
		rightPort->setChannels(rightChannels);
	} //setChannelsFromInput

	void pushAudio() override {
		if (!hasPorts) return;
/*
		for (unsigned int i = 0; i < 4; i++) {
			ilPort->setVoltageSimd(ilAudio[i], i*4);
		}
*/
		unsigned int c = 0;
		for (unsigned int i = 0; i < 2; i++) {
			leftPort->setVoltageSimd(leftAudio[i], i*4);
			rightPort->setVoltageSimd(rightAudio[i], i*4);
			for (unsigned int j = 0; j < 4; j++) {
				ilPort->setVoltage(leftAudio[i][j], c++);
				ilPort->setVoltage(rightAudio[i][j], c++);
			}
		}
	} //pushAudio
	
}; //comboAudioOut

template<typename TIn = comboAudioIn, typename TOut = comboAudioOut>
struct comboAudioLinked {
	TIn* caIn = nullptr;
	TOut* caOut = nullptr;

	void setPorts(TIn* input, TOut* output) {
		caIn = input;
		caOut = output;
	}
	
	void copyAudio() {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 4; i++) {
				caOut->allAudio[i] = caIn->allAudio[i];
			}
			caOut->pushAudio();
		}
	} //copyAudio
	
	void addAudio() {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 4; i++) {
				caOut->allAudio[i] += caIn->allAudio[i];
			}
			caOut->pushAudio();
		}
	} //addAudio
	
	void gainAudio(simd::float_4* monoGains) {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 2; i++) {
				caOut->leftAudio[i] = caIn->leftAudio[i] * simd::ifelse(monoGains[i] > 0, simd::pow(monoGains[i], 2), 0);
				caOut->rightAudio[i] = caIn->rightAudio[i] * simd::ifelse(monoGains[i] > 0, simd::pow(monoGains[i], 2), 0);
			}
			caOut->pushAudio();
		}
	} //gainAudio (separate gains)
	
	void levelAudio(float level) {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 4; i++) {
				caOut->allAudio[i] = caIn->allAudio[i] * simd::pow(level, 2);
			}
			caOut->pushAudio();
		}
	} //levelAudio (single gain, used for level)

	void addLevelAudio(float level) {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 4; i++) {
				caOut->allAudio[i] += caIn->allAudio[i] * simd::pow(level, 2);
			}
			caOut->pushAudio();
		}
	} // addLevelAudio
}; //comboAudioLinked

struct comboAudioBlended {
	comboAudioIn* caInWet = nullptr;
	comboAudioIn* caInDry = nullptr;
	comboAudioOut* caOut = nullptr;
	
	void setPorts(comboAudioIn* wet, comboAudioIn* dry, comboAudioOut* output) {
		caInWet = wet;
		caInDry = dry;
		caOut = output;
	} //setPorts
	
	void blendAudio(float wetAmount, float level, comboAudioIn* caInExtra = nullptr, float masterLevel = 1.f) {
		if (caInWet && caInDry && caOut) {
			float dryAmount = 1.f - wetAmount;
			for (unsigned int i = 0; i < 4; i++) {
				caOut->allAudio[i] = ((caInWet->allAudio[i] * simd::pow(level, 2) * wetAmount) + (caInDry->allAudio[i] * dryAmount) + (caInExtra ? caInExtra->allAudio[i] * wetAmount : 0.f)) * simd::pow(masterLevel, 2);
			}
			caOut->pushAudio();
		}
	} //blendAudio
	
	void blendAudio(float wetAmount, float level, float masterLevel = 1.f) {
		blendAudio(wetAmount, level, nullptr, masterLevel);
	}

 	void addLevelAudio(float level) {
		if (caInWet && caInDry && caOut) {
			for (unsigned int i = 0; i < 4; i++) {
				caOut->allAudio[i] = (caInWet->allAudio[i] * simd::pow(level, 2)) + caInDry->allAudio[i];
			}
			caOut->pushAudio();
		}
	} // addLevelAudio
}; //comboAudioBlended

