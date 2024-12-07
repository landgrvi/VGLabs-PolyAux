#include "plugin.hpp"

Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelBaseLoop8);
	p->addModel(modelLoop8);
	p->addModel(modelOuts8);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
