#pragma once

#include "plugin.hpp"
#include "svgtheme.hpp"
#include "svt_rack.hpp"
#include <array>

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
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/chickenhead_black.svg")));
	}
};	

struct ChickenHeadKnobIvory : ChickenHeadKnob {
	ChickenHeadKnobIvory() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/ch3divory.svg")));
	}
};	

struct GreenRedLightLatch : SvgSwitch {
    SmallLight<GreenRedLight>* light;
 
	GreenRedLightLatch() {
		momentary = false;
		shadow->opacity = 0.0;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/transparent_circle_green.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/transparent_circle_red.svg")));

        light = new SmallLight<GreenRedLight>;
        // Move center of light to center of box
        light->box.pos = box.size.div(2).minus(light->box.size.div(2));
        light->bgColor = color::BLACK_TRANSPARENT;
        light->borderColor = color::BLACK_TRANSPARENT;
        addChild(light);
	}
	
    SmallLight<GreenRedLight>* getLight() {
        return light;
    }
};

struct WhitePJ301MPort : SvgPort {
	WhitePJ301MPort() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/PJ301MWhite.svg")));
		shadow->opacity = 0.0;
	}
};

struct RedPJ301MPort : SvgPort {
	RedPJ301MPort() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/PJ301MRed.svg")));
		shadow->opacity = 0.0;
	}
};

struct PinkPJ301MPort : SvgPort {
	PinkPJ301MPort() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/PJ301MPink.svg")));
		shadow->opacity = 0.0;
	}
};

struct WhiteRedPJ301MPort : SvgPort {
	WhiteRedPJ301MPort() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/PJ301MWhiteRed.svg")));
		shadow->opacity = 0.0;
	}
};


// Derived from MindMeldModular GenericComponents.hpp:
void drawRoundedRectHalo(const Widget::DrawArgs &args, Vec boxSize, NVGcolor haloColor);

struct SvgButtonWithRoundedRectHalo : SvgSwitch, svg_theme::IApplyTheme {

	std::array<std::string, 2> filenames;
	
	// internal
	bool manualDrawTopOverride = false;
	
	// derived classes must set up
	NVGcolor haloColor = nvgRGB(0xFF, 0xFF, 0xFF);// this should match the color of fill of the on button

	SvgButtonWithRoundedRectHalo(std::array<std::string, 2> files) {
		filenames = files;
		momentary = false;
		shadow->opacity = 0.0f;// Turn off shadows
		addFrame(Svg::load(filenames[0]));
		addFrame(Svg::load(filenames[1]));
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

    // implement IApplyTheme
    bool applyTheme(svg_theme::SvgThemes& themes, std::shared_ptr<svg_theme::Theme> theme) override {
		bool modified = false;
		if (themes.applyTheme(theme, filenames[0], frames[0])) modified = true;
		if (themes.applyTheme(theme, filenames[1], frames[1])) modified = true;
		if (themes.applyTheme(theme, filenames[getParamQuantity()->getValue() < 0.5f ? 0 : 1], sw->svg)) modified = true;
        return modified;
    }

};	

struct btnMute : SvgButtonWithRoundedRectHalo {

	btnMute() : SvgButtonWithRoundedRectHalo(std::array<std::string, 2>{asset::plugin(pluginInstance, "res/MuteButtonGrey.svg"), asset::plugin(pluginInstance, "res/MuteButtonRed.svg")}) {
		haloColor = nvgRGB(0xE9, 0x1D, 0x0E); // this should match the color of fill of the on button
	}

};
		
struct btnSolo : SvgButtonWithRoundedRectHalo {

	btnSolo() : SvgButtonWithRoundedRectHalo(std::array<std::string, 2>{asset::plugin(pluginInstance, "res/SoloButtonGrey.svg"), asset::plugin(pluginInstance, "res/SoloButtonGreen.svg")}) {
		haloColor = nvgRGB(0x41, 0xB6, 0x33); // this should match the color of fill of the on button
	}

};

// from Rack's Svg.cpp
static NVGcolor getNVGColor(uint32_t color) {
	return nvgRGBA(
		(color >> 0) & 0xff,
		(color >> 8) & 0xff,
		(color >> 16) & 0xff,
		(color >> 24) & 0xff);
}

	
template<typename TModule>
struct LedDisplayLimitedTextField : LedDisplayTextField, svg_theme::IApplyTheme {
	TModule* module;
	unsigned int index = 0;
	int numChars = 4;

	void onChange( const event::Change &e ) override {
		if (cursor > numChars) {
			text.resize(numChars);
			cursor = numChars;
			selection = numChars;
		}
		if (module) {
			std::string label = getText();
			unsigned int s = index * 4;
			unsigned int n = std::min(label.size(), (size_t)4);
			strncpy(&(module->trackLabelChars[s]), label.c_str(), n);
			for (unsigned int i = n; i < 4; i++) {
				module->trackLabelChars[s + i] = ' ';
			}
		}
		LedDisplayTextField::onChange(e);
	}

    bool applyTheme(svg_theme::SvgThemes& themes, std::shared_ptr<svg_theme::Theme> theme) override {
		if (theme->getStyle("leddisplaytext") && theme->getStyle("leddisplaytext")->isApplyStroke()) { 
			// DEBUG("%s %x", theme->name.c_str(), theme->getStyle("leddisplaytext")->stroke.getColor());
			color = getNVGColor(theme->getStyle("leddisplaytext")->stroke.getColor());
			return true;
		}
		return false;
    }
};

struct VGLabsSlider : SvgSlider, svg_theme::IApplyTheme {
	VGLabsSlider() {
		setBackgroundSvg(Svg::load(asset::plugin(pluginInstance, "res/VGLabsSlider.svg")));
		setHandleSvg(Svg::load(asset::plugin(pluginInstance, "res/VGLabsSliderHandle.svg")));
		setHandlePosCentered(math::Vec(box.size.x / 2, box.size.y - 5), math::Vec(box.size.x / 2, 5));
	}

    // implement IApplyTheme
    bool applyTheme(svg_theme::SvgThemes& themes, std::shared_ptr<svg_theme::Theme> theme) override
    {
		bool modified = false;
		if (themes.applyTheme(theme, asset::plugin(pluginInstance, "res/VGLabsSlider.svg"), background->svg)) modified = true;
		if (themes.applyTheme(theme, asset::plugin(pluginInstance, "res/VGLabsSliderHandle.svg"), handle->svg)) modified = true;
		return modified;
    }

};
