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
		//setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/chickenhead_ivory_tweaked.svg")));
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ch3divory.svg")));
		//setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ch3divoryBG.svg")));
		//setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/radialgradient.svg")));
		//setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "/home/victoria/Software/DISTRHO/Cardinal/plugins/ZamAudio/res/components/button-on.svg")));
		//setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ch3divoryPTR.svg")));
		//bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "/home/victoria/Software/DISTRHO/Cardinal/plugins/ZamAudio/res/components/button-on.svg")));
	}
};	

template <typename TLight = RedLight>
struct ClearLightLatch : SvgSwitch {
    app::ModuleLightWidget* light;
 
	ClearLightLatch() {
		momentary = false;
		shadow->opacity = 0.0;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/transparent_circle_purplegray.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/transparent_circle_red.svg")));

        light = new TLight;
        // Move center of light to center of box
        light->box.pos = box.size.div(2).minus(light->box.size.div(2));
        light->bgColor = color::BLACK_TRANSPARENT;
        light->borderColor = color::BLACK_TRANSPARENT;
        addChild(light);
	}

    app::ModuleLightWidget* getLight() {
        return light;
    }
};

struct WhitePJ301MPort : SvgPort {
	WhitePJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MWhite.svg")));
	}
};

struct RedPJ301MPort : SvgPort {
	RedPJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MRed.svg")));
	}
};

struct PinkPJ301MPort : SvgPort {
	PinkPJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MPink.svg")));
	}
};

struct WhiteRedPJ301MPort : SvgPort {
	WhiteRedPJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MWhiteRed.svg")));
	}
};

