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
	// Just in case: 
	float* ilAudioPtrs[16] = {&(leftAudio[0][0]), &(rightAudio[0][0]),
		                      &(leftAudio[0][1]), &(rightAudio[0][1]), 
							  &(leftAudio[0][2]), &(rightAudio[0][2]),		                     
							  &(leftAudio[0][3]), &(rightAudio[0][3]),		                     
							  &(leftAudio[1][0]), &(rightAudio[1][0]),		                     
							  &(leftAudio[1][1]), &(rightAudio[1][1]),		                     
							  &(leftAudio[1][2]), &(rightAudio[1][2]),		                     
							  &(leftAudio[1][3]), &(rightAudio[1][3])};
	bool hasPorts = false;
	Port* ilPort = nullptr;
	Port* leftPort = nullptr;
	Port* rightPort = nullptr;
	bool scales = false;
	float scalingVals[4] = {1.f, 0.f, 0.f, 1.f};
	
	void setScaling(float panVals[4], float level) {
		scales = true;
		for (unsigned int i = 0; i < 4; i++) {
			scalingVals[i] = panVals[i] * simd::pow(level, 2);
		}
	}
	
	void resetScaling() {
		scales = false;
		scalingVals[0] = 1.f;
		scalingVals[1] = 0.f;
		scalingVals[2] = 0.f;
		scalingVals[3] = 1.f;
	}
	
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
		if (hasPorts) {
			ilPort->setChannels(ilc);
			leftPort->setChannels(lc);
			rightPort->setChannels(rc);
		}
	} //setChannels	

	void setChannels(unsigned int ilc) {
		setChannels(ilc, ilc / 2, ilc / 2);
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
	
	virtual void pushAudio() {} // I don't remember why. Do we need this?
	
}; //comboAudio

struct comboAudioIn : comboAudio {
	//we have the member variables and setPorts from comboAudio

	void updateChannels() {
		if (hasPorts) {
			ilChannels = std::min(ilPort->getChannels(), 16);
			leftChannels = std::min(leftPort->getChannels(), 8); //silently drops higher channels - sorry, user!
			rightChannels = std::min(rightPort->getChannels(), 8);
			if (ilChannels % 2) ilChannels++;
			ilChannels = std::max({leftChannels * 2, rightChannels * 2, ilChannels});
			leftChannels = rightChannels = ilChannels / 2;
		}
	} //updateChannels
	
	void pullAudio(bool trackEnabled = true) {
		updateChannels(); //now ilChannels == leftChannels (or rightChannels) * 2
		if (hasPorts && trackEnabled) {
			float* ilp = ilPort->getVoltages();
			float* lp = leftPort->getVoltages();
			float* rp = rightPort->getVoltages();
			unsigned int c = 0;
			unsigned int mc = 0;
// i:  0               1
// j:  0   1   2   3   0   1   2   3
// c:  0 1 2 3 4 5 6 7 8 9 101112131415
// mc: 0   1   2   3   4   5   6   7
			if (scales) {
				float amtLeft = scalingVals[0];
				float amtRightToLeft = scalingVals[1];
				float amtLeftToRight = scalingVals[2];
				float amtRight = scalingVals[3];
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
				for (unsigned int i = 0; i < 2; i++) {
					for (unsigned int j = 0; j < 4; j++) {
						leftAudio[i][j] = ilp[c++]  + lp[mc];
						rightAudio[i][j] = ilp[c++]  + rp[mc++];
					}
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
		if (hasPorts) {
			ilPort->setChannels(ilChannels);
			leftPort->setChannels(leftChannels);
			rightPort->setChannels(rightChannels);
		}
	} //setChannelsFromInput

	void pushAudio() override {
		if (hasPorts) {
			unsigned int c = 0;
			unsigned int mc = 0;
			float* lp = leftPort->getVoltages();
			float* rp = rightPort->getVoltages();
			if (scales) {
				for (unsigned int i = 0; i < 2; i++) {
					leftPort->setVoltageSimd(leftAudio[i] * scalingVals[0] + rightAudio[i] * scalingVals[1], i*4);
					rightPort->setVoltageSimd(rightAudio[i] * scalingVals[3] + leftAudio[i] * scalingVals[2], i*4);
					for (unsigned int j = 0; j < 4; j++) {
						ilPort->setVoltage(lp[mc], c++);
						ilPort->setVoltage(rp[mc++], c++);
					}
				}
			} else {
				for (unsigned int i = 0; i < 2; i++) {
					leftPort->setVoltageSimd(leftAudio[i], i*4);
					rightPort->setVoltageSimd(rightAudio[i], i*4);
					for (unsigned int j = 0; j < 4; j++) {
						ilPort->setVoltage(lp[mc], c++);
						ilPort->setVoltage(rp[mc++], c++);
					}
				}
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
	
}; //comboAudioLinked

struct comboAudioBlended {
	comboAudioIn* caInWet = nullptr;
	comboAudioIn* caInDry = nullptr;
	comboAudioIn* caInExtra = nullptr;
	comboAudioOut* caOut = nullptr;
	
	void setPorts(comboAudioIn* wet, comboAudioIn* dry, comboAudioOut* output, comboAudioIn* extra = nullptr) {
		caInWet = wet;
		caInDry = dry;
		caOut = output;
		caInExtra = extra ? extra : nullptr;
	} //setPorts
	
	void blendAudio(float wetAmount) {
		if (caInWet && caInDry && caOut) {
			float dryAmount = 1.f - wetAmount;
			if (caInExtra) {
				for (unsigned int i = 0; i < 4; i++) {
					caOut->allAudio[i] = (caInDry->allAudio[i] * dryAmount) + ((caInWet->allAudio[i] + caInExtra->allAudio[i]) * wetAmount);
				}
			} else {
				for (unsigned int i = 0; i < 4; i++) {
					caOut->allAudio[i] = (caInDry->allAudio[i] * dryAmount) + (caInWet->allAudio[i] * wetAmount);
				}
			}
			caOut->pushAudio();
		}
	} //blendAudio

}; //comboAudioBlended

