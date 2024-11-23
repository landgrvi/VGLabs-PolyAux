#pragma once

#include "plugin.hpp"

using namespace rack;

extern Plugin *pluginInstance;
static const NVGcolor VGLABSPURPLE = nvgRGBA(0x46, 0x16, 0x21, 0xFF);
static const NVGcolor VGLABSPURPLEGRAY = nvgRGBA(0x46+0x3F, 0x16+0x3F, 0x21+0x3F, 0xFF); //0x85 = 133, 0x55 = 85, 0x60 = 96


struct ChickenHeadKnob : RoundKnob {
	ChickenHeadKnob() {
		shadow->opacity = 0.0;
		minAngle = -0.83 * float(M_PI);
		maxAngle = 0.83 * float(M_PI);
	}
};

struct ChickenHeadKnobBlack : ChickenHeadKnob {
	ChickenHeadKnobBlack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/chickenhead_black.svg")));
	}
};	

struct ChickenHeadKnobIvory : ChickenHeadKnob {
	ChickenHeadKnobIvory() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ch3divory.svg")));
	}
};	

struct GreenRedLightLatch : SvgSwitch {
    SmallLight<GreenRedLight>* light;
 
	GreenRedLightLatch() {
		momentary = false;
		shadow->opacity = 0.0;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/transparent_circle_green.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/transparent_circle_red.svg")));

        light = new SmallLight<GreenRedLight>;
        // Move center of light to center of box
        light->box.pos = box.size.div(2).minus(light->box.size.div(2));
        light->bgColor = color::BLACK_TRANSPARENT;
        light->borderColor = color::BLACK_TRANSPARENT;
        light->setBrightnesses({0.f, 0.f});
        addChild(light);
	}
	
    SmallLight<GreenRedLight>* getLight() {
        return light;
    }
};

struct WhitePJ301MPort : SvgPort {
	WhitePJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MWhite.svg")));
		shadow->opacity = 0.0;
	}
};

struct RedPJ301MPort : SvgPort {
	RedPJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MRed.svg")));
		shadow->opacity = 0.0;
	}
};

struct PinkPJ301MPort : SvgPort {
	PinkPJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MPink.svg")));
		shadow->opacity = 0.0;
	}
};

struct WhiteRedPJ301MPort : SvgPort {
	WhiteRedPJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MWhiteRed.svg")));
		shadow->opacity = 0.0;
	}
};


// From MindMeldModular GenericComponents.hpp
// Buttons and switches

void drawRectHalo(const Widget::DrawArgs &args, Vec boxSize, NVGcolor haloColor, float posX);
void drawRoundHalo(const Widget::DrawArgs &args, Vec boxSize, NVGcolor haloColor);


struct SvgSwitchWithHalo : SvgSwitch {
	// internal
	bool manualDrawTopOverride = false;
	
	// derived classes must set up
	NVGcolor haloColor = nvgRGB(0xFF, 0xFF, 0xFF);// this should match the color of fill of the on button
	bool isRect = false;

	SvgSwitchWithHalo() {
		shadow->opacity = 0.0f;// Turn off shadows
	}

	void draw(const DrawArgs &args) override {
		ParamQuantity* paramQuantity = getParamQuantity();
		if (!paramQuantity || paramQuantity->getValue() < 0.5f || manualDrawTopOverride) {
			SvgSwitch::draw(args);
		}
	}	
	
	void drawLayer(const DrawArgs &args, int layer) override {
		if (layer == 1) {
			ParamQuantity* paramQuantity = getParamQuantity();
			if (!paramQuantity || paramQuantity->getValue() < 0.5f) {
				// if no module or if switch is off, no need to do anything in layer 1
				return;
			}

			if (settings::haloBrightness != 0.f) {
				if (isRect) {
					drawRectHalo(args, box.size, haloColor, 0.0f);
				}
				else {
					drawRoundHalo(args, box.size, haloColor);
				}
			}
			manualDrawTopOverride = true;
			draw(args);
			manualDrawTopOverride = false;
		}
		SvgSwitch::drawLayer(args, layer);
	}
};
// End direct copy from MindMeldModular

// Derived from MMM code above:
void drawRoundedRectHalo(const Widget::DrawArgs &args, Vec boxSize, NVGcolor haloColor);

struct SvgSwitchWithRoundedRectHalo : SvgSwitch {

	// internal
	bool manualDrawTopOverride = false;
	
	// derived classes must set up
	NVGcolor haloColor = nvgRGB(0xFF, 0xFF, 0xFF);// this should match the color of fill of the on button
	bool isRect = false;

	SvgSwitchWithRoundedRectHalo() {
		shadow->opacity = 0.0f;// Turn off shadows
	}

	void draw(const DrawArgs &args) override {
		ParamQuantity* paramQuantity = getParamQuantity();
		if (!paramQuantity || paramQuantity->getValue() < 0.5f || manualDrawTopOverride) {
			SvgSwitch::draw(args);
		}
	}	
	
	void drawLayer(const DrawArgs &args, int layer) override {
		if (layer == 1) {
			ParamQuantity* paramQuantity = getParamQuantity();
			if (!paramQuantity || paramQuantity->getValue() < 0.5f) {
				// if no module or if switch is off, no need to do anything in layer 1
				return;
			}

			if (settings::haloBrightness != 0.f) {
				drawRoundedRectHalo(args, box.size, haloColor);
			}
			manualDrawTopOverride = true;
			draw(args);
			manualDrawTopOverride = false;
		}
		SvgSwitch::drawLayer(args, layer);
	}
};	

struct btnMute : SvgSwitchWithRoundedRectHalo {
	btnMute() {
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MuteButtonGrey.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MuteButtonRed.svg")));
		haloColor = nvgRGB(0xE9, 0x1D, 0x0E);// this should match the color of fill of the on button
	}

};
		
struct btnSolo : SvgSwitchWithRoundedRectHalo {
	btnSolo() {
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SoloButtonGrey.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SoloButtonGreen.svg")));
		haloColor = nvgRGB(0x41, 0xB6, 0x33);// this should match the color of fill of the on button
	}
};
		

