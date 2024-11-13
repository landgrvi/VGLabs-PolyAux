#pragma once

#include "plugin.hpp"

using namespace rack;

//Note: "il" means "interleaved": L R L R L R...

struct comboAudioStub {
	unsigned int ilChannels = 0;
	unsigned int leftChannels = 0;
	unsigned int rightChannels = 0;
	simd::float_4 allAudio[8] = { };
	simd::float_4* ilAudio = &(allAudio[0]);
	simd::float_4* leftAudio = &(allAudio[4]);
	simd::float_4* rightAudio = &(allAudio[6]);
	bool hasPorts = false;
}; //comboAudioStub
	
struct comboAudio : comboAudioStub {
	Port* ilPort = nullptr;
	Port* leftPort = nullptr;
	Port* rightPort = nullptr; 
	
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
		for (unsigned int i = 0; i < 8; i++) {
			allAudio[i] = 0.f;
		}
		if (hasPorts) {
			ilPort->setChannels(0);  //also clears voltages; see rack::engine::Port Struct Reference
			leftPort->setChannels(0);
			rightPort->setChannels(0);
		}
	} //clearAudio
	
	void setAudio(simd::float_4* aa) {
		for (unsigned int i = 0; i < 8; i++) {
			allAudio[i] = aa[i];
		}
	} //setAudio
	
	void setAudio(simd::float_4* ila, simd::float_4* la, simd::float_4* ra) {
		for (unsigned int i = 0; i < 4; i++) {
			ilAudio[i] = ila[i];
		}
		for (unsigned int i = 0; i < 2; i++) {
			leftAudio[i] = la[i];
			rightAudio[i] = ra[i];
		}
	} //setAudio
	
	virtual void pushAudio() {}
}; //comboAudio

struct comboAudioIn : comboAudio {
	//we have the member variables and setPorts from comboAudio

	void updateChannels() {
		if (!hasPorts) return;
		ilChannels = std::min(ilPort->getChannels(), 16);
		leftChannels = std::min(leftPort->getChannels(), 8); //silently drops higher channels - sorry, user!
		rightChannels = std::min(rightPort->getChannels(), 8);
		if (ilChannels % 2) {
			ilChannels++;
		}
		ilChannels = std::max(std::max(leftChannels * 2, ilChannels), std::max(rightChannels * 2, ilChannels));
		leftChannels = rightChannels = ilChannels / 2;
		//ilPort->setChannels(ilChannels);
		//leftPort->setChannels(leftChannels);
		//rightPort->setChannels(rightChannels);
	} //updateChannels
	
	void pullAudio(unsigned int trackEnabled = 1) {
		if (!hasPorts) return;
		updateChannels(); //now ilChannels == leftChannels (or rightChannels) * 2

// 0 1 2 3 4 5 6 7 8 9 101112131415
// 0   1   2   3   4   5   6   7
// 0   0   0   0   1   1   1   1
// 0   1   2   3   0   1   2   3
// 0 0 0 0 1 1 1 1 2 2 2 2 3 3 3 3
// 0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3
		if (trackEnabled) {
			unsigned int c = 0;
			unsigned int mc = 0;
			unsigned int mi = 0;
			unsigned int mj = 0;
			float* ilp = ilPort->getVoltages();
			float* lp = leftPort->getVoltages();
			float* rp = rightPort->getVoltages();
			for (unsigned int i = 0; i < 4; i++) {
				for (unsigned int j = 0; j < 4; j++) {
					c = (i * 4) + j;
					mc = c / 2;
					mi = i / 2;
					mj = mc % 4;
					if (c % 2 == 0) {
						leftAudio[mi][mj] = ilAudio[i][j] = ilp[c] + lp[mc];
					} else {
						rightAudio[mi][mj] = ilAudio[i][j] = ilp[c] + rp[mc];
					}
				}
			}
		} else {
			for (unsigned int i = 0; i < 8; i++) {
				allAudio[i] = 0.f;
			}
		}	
	} //pullAudio
}; //comboAudioIn

struct comboAudioOut : comboAudio {
	//we have the member variables and setPorts from comboAudio

	void setChannelsFromInput (comboAudioIn* caIn) {
		ilChannels = caIn->ilChannels;
		leftChannels = caIn->leftChannels;
		rightChannels = caIn->rightChannels;
		if (!hasPorts) return;
		ilPort->setChannels(ilChannels);
		leftPort->setChannels(leftChannels);
		rightPort->setChannels(rightChannels);
	} //setChannelsFromInput
	
	void setChannelsFromInput (comboAudio* caIn) {
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
		for (unsigned int i = 0; i < 4; i++) {
			ilPort->setVoltageSimd(ilAudio[i], i*4);
		}
		for (unsigned int i = 0; i < 2; i++) {
			leftPort->setVoltageSimd(leftAudio[i], i*4);
			rightPort->setVoltageSimd(rightAudio[i], i*4);
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
			for (unsigned int i = 0; i < 8; i++) {
				caOut->allAudio[i] = caIn->allAudio[i];
			}
			caOut->pushAudio();
		}
	} //copyAudio
	
	void addAudio() {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 8; i++) {
				caOut->allAudio[i] += caIn->allAudio[i];
			}
			caOut->pushAudio();
		}
	} //addAudio
	
	void gainAudio(simd::float_4* ilGains, simd::float_4* monoGains) {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 4; i++) {
				caOut->ilAudio[i] = caIn->ilAudio[i] * simd::ifelse(ilGains[i] > 0, simd::pow(ilGains[i], 2), 0);
			}
			for (unsigned int i = 0; i < 2; i++) {
				caOut->leftAudio[i] = caIn->leftAudio[i] * simd::ifelse(monoGains[i] > 0, simd::pow(monoGains[i], 2), 0);
				caOut->rightAudio[i] = caIn->rightAudio[i] * simd::ifelse(monoGains[i] > 0, simd::pow(monoGains[i], 2), 0);
			}
			caOut->pushAudio();
		}
	} //gainAudio (separate gains)
	
	void levelAudio(float level) {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 8; i++) {
				caOut->allAudio[i] = caIn->allAudio[i] * simd::pow(level, 2);
			}
			caOut->pushAudio();
		}
	} //levelAudio (single gain, used for level)

	void addLevelAudio(float level) {
		if (caIn && caOut) {
			for (unsigned int i = 0; i < 8; i++) {
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
	
	void blendAudio(float wetAmount, float level, comboAudioIn* caInExtra = nullptr) {
		if (caInWet && caInDry && caOut) {
			float dryAmount = 1.f - wetAmount;
			for (unsigned int i = 0; i < 8; i++) {
				caOut->allAudio[i] = (caInWet->allAudio[i] * simd::pow(level, 2) * wetAmount) + (caInDry->allAudio[i] * dryAmount) + (caInExtra ? caInExtra->allAudio[i] * wetAmount : 0.f);
			}
			caOut->pushAudio();
		}
	} //blendAudio
	
	void addLevelAudio(float level) {
		if (caInWet && caInDry && caOut) {
			for (unsigned int i = 0; i < 8; i++) {
				caOut->allAudio[i] = (caInWet->allAudio[i] * simd::pow(level, 2)) + caInDry->allAudio[i];
			}
			caOut->pushAudio();
		}
	} // addLevelAudio
}; //comboAudioBlended

