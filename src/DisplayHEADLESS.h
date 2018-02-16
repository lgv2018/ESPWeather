#ifndef MY_DISPLAY_HEADLESS_H
#define MY_DISPLAY_HEADLESS_H

#include "Config.h"
#include "DisplayBase.h"

class Display : public DisplayBase
{
public:
	Display(ConfigurationBase& config) : DisplayBase() {}

	virtual void begin() {}
	virtual void loop(long now) {}

	virtual void publish_telemetry(const String& name) {}
	virtual void publish_name(const String& name) {}
	virtual void publish_status(const String& str) {}
	virtual void log(const String& str) {}

};

#endif
