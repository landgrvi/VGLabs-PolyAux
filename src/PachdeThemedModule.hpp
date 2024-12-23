// This module and widget are a restructure of pachde's svgtheme demo code;
// See https://github.com/Paul-Dempsey/svg_theme for more info.

#pragma once

#include <rack.hpp>
#include "svgtheme.hpp"
#include "svt_rack.hpp"

struct PachdeThemedModule : Module {
    std::string defaultTheme = "Light";
    // This is the name of the selected theme, to save in json
    std::string theme;
    // The svg_theme engine
    svg_theme::SvgThemes themes;
    // Where the themes come from
    std::string themeFilename;
    
    PachdeThemedModule(std::string themeFile, std::string storageDir, std::string defaultTheme = "Light") {
		themeFilename = asset::plugin(pluginInstance, themeFile);
		std::string pluginThemeFile = system::getCanonical(themeFilename);
		std::string userStorage = system::getDirectory(asset::user("")) + "/" + storageDir + "/";
		std::string userThemeFile = userStorage + themeFile;
		std::string userThemeDir = system::getDirectory(userThemeFile);
		if (system::isFile(userThemeFile)) {
			themeFilename = userThemeFile;
		} else if (pluginThemeFile.size() > 0 && (system::isDirectory(userThemeDir) || system::createDirectories(userThemeDir))) {
			if (system::copy(pluginThemeFile, userThemeFile)) themeFilename = userThemeFile;
		}
			
		this->defaultTheme = defaultTheme;
	// For demo and authoring purposes, we log to the Rack log.
	//
	// In a production VCV Rack module in the library, logging to Rack's log is disallowed.
	// The logging is necessary only when authoring your theme and SVG.
	// Once your theme is correctly applying to the SVG, you do not need this logging
	// because it's useless to anyone other than someone modifying the SVG or theme.
	//
		themes.setLog([](svg_theme::Severity severity, svg_theme::ErrorCode code, std::string info)->void {
			DEBUG("Theme %s (%d): %s", SeverityName(severity), code, info.c_str());
		});
	}

    // get and set the theme to be persisted
    void setTheme(std::string theme) { this->theme = theme; }
    std::string getTheme() { return theme; }

    // access to the themes engine for the ModuleWidget
    svg_theme::SvgThemes& getThemes() { return themes; }

    // initThemes must be called to load themes.
    // It can be safely called multiple times and it only loads the first time.
    // In other words, it is "lazy" or just-in-time initialization.
    bool initThemes()
    {
        return themes.isLoaded()
            ? true
            : themes.load(themeFilename);
    }
    // Standard Rack persistence so that the selected theme is remembered with the patch.
    void dataFromJson(json_t* root) override {
        json_t* j = json_object_get(root, "theme");
        if (j) {
            theme = json_string_value(j);
        }
    }
    json_t* dataToJson() override {
        json_t* root = json_object();
        json_object_set_new(root, "theme", json_stringn(theme.c_str(), theme.size()));
        return root;
    }
    
};

struct PachdeThemedModuleWidget : ModuleWidget, svg_theme::IThemeHolder {
    PachdeThemedModule* my_module = nullptr;
    std::string panelFilename;
    bool setMyTheme = false;
    
    PachdeThemedModuleWidget(PachdeThemedModule* module, std::string panelFile) {
		panelFilename = asset::plugin(pluginInstance, panelFile);
		setPanel(createPanel(panelFilename));
		my_module = module ? module : nullptr;
		if (my_module) {
			if (isDefaultTheme()) {
				my_module->theme = my_module->defaultTheme;
			} else {
				// only initialize themes and modify the svg when the current theme is not the default theme
				//DEBUG("constructing");
				if (my_module->initThemes()) {
					auto theme = my_module->getTheme();
					if (theme.length() == 0) {
						//DEBUG("%li has no theme in constructor", my_module->id);
					} else {
						//DEBUG("%li has theme %s in constructor", my_module->id, theme.c_str());
						setTheme(theme);
					}
				}
			}
		}
	}

	void step() override {
		if (setMyTheme) {
			if (my_module) {
				//DEBUG("setting theme in step");
				auto theme = my_module->getTheme();
				if (theme.length() == 0) {
					//DEBUG("%li has no theme in step", my_module->id);
				} else {
					setTheme(theme);
				}
				setMyTheme = false;
			}
		}
		ModuleWidget::step();
	}

    // true when the default theme is the current theme
    bool isDefaultTheme() {
        if (!my_module) return true;
        auto theme = my_module->getTheme();
        return theme.empty() || 0 == theme.compare(my_module->defaultTheme);
    } 

    // IThemeHolder used by the menu helper
    std::string getTheme() override {
        return isDefaultTheme() ? my_module->defaultTheme : my_module->getTheme(); 
    } 

    // IThemeHolder used by the menu helper, and also whenever 
    // we want to apply a new theme
    void setTheme(std::string theme) override {
        if (!my_module) return;
        auto panel = dynamic_cast<rack::app::SvgPanel*>(getPanel());
        if (!panel) return;
		//DEBUG("Setting theme %s in %li", theme.c_str(), my_module->id);

        my_module->initThemes(); // load themes as necessary
        auto themes = my_module->getThemes();
        auto svg_theme = themes.getTheme(theme);
        if (!svg_theme) return;

        // For demo purposes, we are using a stock Rack SVGPanel
        // which does not implement IApplyTheme.so here we do it manually.
        // This shows how to apply themeing without implementing IApplyTheme
        // and using ApplyChildrenTheme.
        std::shared_ptr<Svg> newSvg = panel->svg;
        if (themes.applyTheme(svg_theme, panelFilename, newSvg)) {
			//DEBUG("Applied theme %s %s", theme.c_str(), svg_theme->name.c_str());
			panel->setBackground(newSvg);
			//DEBUG("set panel background"); 
            // The SVG was changed, so we need to tell the widget to redraw
            EventContext ctx;
            DirtyEvent dirt;
            dirt.context = &ctx;
            panel->onDirty(dirt);
        }
        // The preferred procedure is to subclass any widget you want to theme,
        // implementing IApplyTheme (which is quite simple to do in most cases),
        // and use this helper to apply the theme to the widget hierarchy.
        ApplyChildrenTheme(this, themes, svg_theme);

        // Let the module know what the new theme is so that it will be remembered.
        my_module->setTheme(theme);
    } 

    void appendContextMenu(Menu *menu) override {
        if (!my_module) return;
        if (!my_module->initThemes()) return;
        auto themes = my_module->getThemes();
        if (!themes.isLoaded()) return; // Can't load themes, so no menu to display

        // Good practice to separate your module's menus from the Rack menus
        menu->addChild(new MenuSeparator);

        // add the "Theme" menu
        //svg_theme::AppendThemeMenu(menu, this, themes);
        menu->addChild(createSubmenuItem("Themes", "", [=](Menu* menu) {//menu->addChild(createMenuItem("Use Left (not implemented)", "", [=]() {} ));
																	    svg_theme::AppendThemeMenu(menu, this, my_module->themes); } ));
        
    } 
};
