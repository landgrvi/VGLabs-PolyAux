#include "plugin.hpp"

using namespace rack;

// From MindMeldModular GenericComponents.cpp

void drawRectHalo(const Widget::DrawArgs &args, Vec boxSize, NVGcolor haloColor, float posX) {
	// some of the code in this block is adapted from LightWidget::drawHalo() and the composite call is from LightWidget::drawLayer()
	
	nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);

	const float brightness = 0.8f;
	NVGcolor color = nvgRGBAf(0, 0, 0, 0);
	NVGcolor hc = haloColor;
	hc.a *= math::clamp(brightness, 0.f, 1.f);
	color = color::screen(color, hc);
	color = color::clamp(color);
	
	nvgBeginPath(args.vg);
	nvgRect(args.vg, -9 + posX, -9, boxSize.x + 18, boxSize.y + 18);
	
	NVGcolor icol = color::mult(color, settings::haloBrightness);
	NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
	NVGpaint paint = nvgBoxGradient(args.vg, -4.5f + posX, -4.5f, boxSize.x + 9, boxSize.y + 9, 6, 8, icol, ocol);// tlx, tly, w, h, radius, feather, icol, ocol
	
	nvgFillPaint(args.vg, paint);
	nvgFill(args.vg);
	
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
}

void drawRoundHalo(const Widget::DrawArgs &args, Vec boxSize, NVGcolor haloColor) {
	// some of the code in this block is adapted from LightWidget::drawHalo() and the composite call is from LightWidget::drawLayer()
	nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);

	const float brightness = 0.8f;
	NVGcolor color = nvgRGBAf(0, 0, 0, 0);
	NVGcolor hc = haloColor;
	hc.a *= math::clamp(brightness, 0.f, 1.f);
	color = color::screen(color, hc);
	color = color::clamp(color);
	
	math::Vec c = boxSize.div(2);
	float radius = std::min(boxSize.x, boxSize.y) / 2.0;
	float oradius = radius + std::min(radius * 4.f, 15.f);

	nvgBeginPath(args.vg);
	nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

	NVGcolor icol = color::mult(color, settings::haloBrightness);
	NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
	NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
	nvgFillPaint(args.vg, paint);
	nvgFill(args.vg);
	
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
}

// End direct copy from MindMeldModular

// Adapted from MindMeldModular GenericComponents.cpp

void drawRoundedRectHalo(const Widget::DrawArgs &args, Vec boxSize, NVGcolor haloColor) {
	// some of the code in this block is adapted from LightWidget::drawHalo() and the composite call is from LightWidget::drawLayer()
	
	nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);

	const float brightness = 0.8f;
	NVGcolor color = nvgRGBAf(0, 0, 0, 0);
	NVGcolor hc = haloColor;
	hc.a *= math::clamp(brightness, 0.f, 1.f);
	color = color::screen(color, hc);
	color = color::clamp(color);

	math::Vec c = boxSize.div(2);
	float radius = std::min(boxSize.x, boxSize.y) / 4.0;
	float oradius = radius + std::min(radius * 4.f, 10.f);
	float spread = boxSize.x / 4;
	
	nvgBeginPath(args.vg);
	nvgRect(args.vg, -9, -9, boxSize.x + 18, boxSize.y + 18);
	
	NVGcolor icol = color::mult(color, settings::haloBrightness);
	NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
	
	// Left circle
	NVGpaint paint = nvgRadialGradient(args.vg, c.x - spread, c.y, radius, oradius, icol, ocol);
	nvgFillPaint(args.vg, paint);
	nvgFill(args.vg);
	
	// Fill in the middle a bit
	paint = nvgRadialGradient(args.vg, c.x, c.y, 0.f, oradius, icol, ocol);
	nvgFillPaint(args.vg, paint);
	nvgFill(args.vg);
	
	// Right circle
	paint = nvgRadialGradient(args.vg, c.x + spread, c.y, radius, oradius, icol, ocol);
	nvgFillPaint(args.vg, paint);
	nvgFill(args.vg);
	
	nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
}
