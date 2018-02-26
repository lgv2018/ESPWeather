/**
*  Copyright (C) 2018  foxis (Andrius Mikonis <andrius.mikonis@gmail.com>)
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/
#ifndef MY_TELEMETRY_H
#define MY_TELEMETRY_H

#include "Config.h"
#include "TelemetryBase.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <BMP280.h>

// These will come from EasyOTA
//#define GETTER(T, name) T name() { return _##name; }
//#define SETTER(T, name) T name(T name) { T pa##name = _##name; _##name = name; return pa##name; }

class Telemetry : public TelemetryBase
{
#ifndef ESP_WEATHER_NO_HUMIDITY
	DHT_Unified dht11;
#endif
	BMP280 bme;
	ConfigurationBase& config;
	unsigned long last_m1;
	bool _init;
	int _skip_readings;

public:
	Telemetry(ConfigurationBase& config) :
		TelemetryBase(),
		config(config)
#ifndef ESP_WEATHER_NO_HUMIDITY
		, dht11(02, DHT11)
#endif
	{

	}

	virtual void begin() {
		#ifndef ESP_WEATHER_NO_HUMIDITY
		dht11.begin();
		#endif
		bme.begin(SDA, SCL);
		bme.setOversampling(16);

		_battery = 0;
		_temperature = 0;
		_humidity = 0;
		_pressure = 0;
		_skip_readings = 5;
		_init = true;
		_send = false;
		bme_ready = 0;
		last_m1 = millis();
		bme_ready = last_m1 + bme.startMeasurment();
	}

	virtual void loop(unsigned long now) {
		// perform measurements every second
		if (now - last_m1 > 1000) {
			double temp, humi, psi;

			#ifndef ESP_WEATHER_NO_HUMIDITY
			sensors_event_t event;
			dht11.humidity().getEvent(&event);
			humi = event.relative_humidity;
			#endif

			for (int i = 0; i < 10; i++)
				_battery += analogRead(A0) / 1024.0;

			_humidity += humi;

			// start BMP280 measurements
			if (bme_ready == 0) {
				bme_ready = now + bme.startMeasurment();
			}

			// Read BMP280 measurements
			// NOTE: Will perform humidity and battery voltage measurements up til now
			if (now - bme_ready > 0) {
				bme.getTemperatureAndPressure(temp, psi);
				if (_skip_readings)
				{
					_send = false;
					_skip_readings--;
				} else if (_temperature == 0) {
					_pressure = psi * 2;
					_temperature = temp * 2;
					_send = true;
				} else {
					_pressure += psi;
					_temperature += temp;
					_send = true;
				}
				bme_ready = 0;
				last_m1 = now;
			} else {
				// retain messages until true measurement comes in
				_temperature *= 2;
				_pressure *= 2;
			}

			// first time measurements
			if (!_init)  {
				_pressure /= 2.0;
				_temperature /= 2.0;
				_humidity /= 2.0;
				_battery /= 2.0;
			} else {
				_init = false;
			}
		}
	}
};

#endif
