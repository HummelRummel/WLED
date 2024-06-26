#pragma once

#include "wled.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#define HW_PIN_SCL 5
#define HW_PIN_SDA 4

/*
 * This usermod adds support for the ADXL345 sensor.
 */

const int MODE_DISABLED         = 0;
const int MODE_BRIGHTNESS       = 1;
const int MODE_EFFECT_SPEED     = 2;
const int MODE_EFFECT_INTENSITY = 3;
const int MODE_EFFECT_FLOW_MODE = 4;
const int MODE_COLOR1_R         = 5;
const int MODE_COLOR1_G         = 6;
const int MODE_COLOR1_B         = 7;
const int MODE_COLOR1I2_R       = 8;
const int MODE_COLOR1I2_G       = 9;
const int MODE_COLOR1I2_B       = 10;

const int DEFAULT_SENSOR_STEPS = 8;
const float DEFAULT_STATIC_MAX = 10;
const float DEFAULT_FLOW_MIN = 12;


class ADXL345SensorUsermod : public Usermod {
  private:
    Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

      //Private class members. You can declare variables and functions only accessible to your usermod here
    unsigned long lastTimeMQTTUpdated = 0;
    unsigned long lastStateUpdated = 0;

    // config
    bool sensorEnabled = false;
    unsigned int updateInterval = 100;
    bool sendMQTTEvent = false;
    bool sendRawMQTTEvent = false;
    bool enableFlowSpeed = false;
    uint8 flowSpeed = 255;
    uint8 staticSpeed = 0;
    int modeX = 0;
    int modeY = 0;
    int modeZ = 0;
    int sensorSteps = DEFAULT_SENSOR_STEPS;

    bool sensorMissing = false;
    float minX = -DEFAULT_STATIC_MAX;
    float maxX = DEFAULT_STATIC_MAX;
    float minY = -DEFAULT_STATIC_MAX;
    float maxY = DEFAULT_STATIC_MAX;
    float minZ = -DEFAULT_STATIC_MAX;
    float maxZ = DEFAULT_STATIC_MAX;
    float flowMinX = -DEFAULT_FLOW_MIN;
    float flowMaxX = DEFAULT_FLOW_MIN;
    float flowMinY = -DEFAULT_FLOW_MIN;
    float flowMaxY = DEFAULT_FLOW_MIN;
    float flowMinZ = -DEFAULT_FLOW_MIN;
    float flowMaxZ = DEFAULT_FLOW_MIN;

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      PinOwner po = PinOwner::UM_ADXL345; // defaults to being pinowner for SCL/SDA pins
      PinManagerPinType pins[2] = { { HW_PIN_SCL, true }, { HW_PIN_SDA, true } };  // allocate pins
      if (!pinManager.allocateMultiplePins(pins, 2, po)) {
        Serial.println("Could not allocate pins for ADXL345, disabling it...");
        sensorMissing = true;
        sensorEnabled = false;
        return;
      }

      if(!accel.begin())
      {
         Serial.println("No ADXL345 sensor detected, disabling it");
         sensorMissing = true;
         sensorEnabled = false;
      // } else {
      //   accel.setRange(ADXL345_RANGE_4_G);
      }
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      //Serial.println("Connected to WiFi!");
    }


    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     *
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     *
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() {
        if (sensorMissing == false && sensorEnabled == true) {
          sensors_event_t event;
          accel.getEvent(&event);
          // if ( event.acceleration.x < minX ) {
          //   minX = event.acceleration.x;
          // }
          // if ( event.acceleration.x > maxX ) {
          //   maxX = event.acceleration.x;
          // }
          // if ( event.acceleration.y < minY ) {
          //   minY = event.acceleration.y;
          // }
          // if ( event.acceleration.y > maxY ) {
          //   maxY = event.acceleration.y;
          // }
          // if ( event.acceleration.z < minZ ) {
          //   minZ = event.acceleration.z;
          // }
          // if ( event.acceleration.z > maxZ ) {
          //   maxZ = event.acceleration.z;
          // }
          if (millis() - lastStateUpdated > updateInterval) {
            bool flowing = false;
            if ((event.acceleration.x > flowMaxX) || (event.acceleration.x < flowMinX)) {
              flowing = true;
            }
            if ((event.acceleration.y > flowMaxY) || (event.acceleration.y < flowMinY)) {
              flowing = true;
            }
            if ((event.acceleration.z > flowMaxZ) || (event.acceleration.z < flowMinZ)) {
              flowing = true;
            }
            bool flowSpeedUpdated = false;
            if (enableFlowSpeed == true) {
              if (flowing == true) {
                if (effectSpeed != flowSpeed) {
                  effectSpeed = flowSpeed;
                  flowSpeedUpdated = true;
                }
              } else {
                if (effectSpeed != staticSpeed) {
                  effectSpeed = staticSpeed;
                  flowSpeedUpdated = true;
                }
              }
            }

            if (event.acceleration.x > maxX) {
              event.acceleration.x = maxX;
            } else if (event.acceleration.x < minX) {
              event.acceleration.x = minX;
            }
            if (event.acceleration.y > maxY) {
              event.acceleration.y = maxY;
            } else if (event.acceleration.y < minY) {
              event.acceleration.y = minY;
            }
            if (event.acceleration.z > maxZ) {
              event.acceleration.z = maxZ;
            } else if (event.acceleration.z < minZ) {
              event.acceleration.z = minZ;
            }
            float scaledX = (1.0 / (maxX - minX)) * ((event.acceleration.x > maxX ? maxX : (event.acceleration.x < minX ? minX : event.acceleration.x)) - minX);
            int uint8ScaledX =  scaledX*255;
            if (uint8ScaledX != 255) {
              uint8ScaledX = (uint8ScaledX / sensorSteps) * sensorSteps;
            }
            float scaledY = (1.0 / (maxY - minY)) * ((event.acceleration.y > maxY ? maxY : (event.acceleration.y < minY ? minY : event.acceleration.y)) - minY);
            int uint8ScaledY =  uint8(scaledY*255);
            if (uint8ScaledY != 255) {
              uint8ScaledY = (uint8ScaledY / sensorSteps) * sensorSteps;
            }
            float scaledZ = (1.0 / (maxZ - minZ)) * ((event.acceleration.z > maxZ ? maxZ : (event.acceleration.z < minZ ? minZ : event.acceleration.z)) - minZ);
            int uint8ScaledZ =  uint8(scaledZ*255);
            if (uint8ScaledZ != 255) {
              uint8ScaledZ = (uint8ScaledZ / sensorSteps) * sensorSteps;
            }

            bool xUpdated = applyMode(modeX, uint8ScaledX, event.acceleration.x, minX, maxX);
            bool yUpdated = applyMode(modeY, uint8ScaledY, event.acceleration.y, minY, maxY);
            bool zUpdated = applyMode(modeZ, uint8ScaledZ, event.acceleration.z, minZ, maxZ);
            lastStateUpdated = millis();

            if(xUpdated || yUpdated || zUpdated || flowSpeedUpdated) {
              colorUpdated(CALL_MODE_INIT);
              updateInterfaces(CALL_MODE_INIT);
            }

            if (millis() - lastTimeMQTTUpdated > 1000) {
              if (sendRawMQTTEvent == true) {
                 if ((WLED_CONNECTED) && (WLED_MQTT_CONNECTED)) {
                   char topic[38];
                   strcpy(topic, mqttDeviceTopic);
                   strcat(topic, "/accel/raw");
                   char data[40];
                   strcpy(data,"X:");
                   strcat(data,String(event.acceleration.x).c_str());
                   strcat(data,",Y:");
                   strcat(data,String(event.acceleration.y).c_str());
                   strcat(data,",Z:");
                   strcat(data,String(event.acceleration.z).c_str());
                   mqtt->publish(topic,0,false,data);
                 }
              }

              if (sendMQTTEvent == true) {
                 if ((WLED_CONNECTED) && (WLED_MQTT_CONNECTED)) {
                   char topic[38];
                   strcpy(topic, mqttDeviceTopic);
                   strcat(topic, "/accel");
                   char data[80];
                   strcpy(data,"X:");
                   strcat(data,String(scaledX).c_str());
                   strcat(data,"/");
                   strcat(data,String(uint8ScaledX).c_str());
                   strcat(data,",Y:");
                   strcat(data,String(scaledY).c_str());
                   strcat(data,"/");
                   strcat(data,String(uint8ScaledY).c_str());
                   strcat(data,",Z:");
                   strcat(data,String(scaledZ).c_str());
                   strcat(data,"/");
                   strcat(data,String(uint8ScaledZ).c_str());
                   mqtt->publish(topic,0,false,data);
                 }
              }

              lastTimeMQTTUpdated = millis();

          }
    }
      }
    }

    bool applyMode(int mode, uint8 scaledValue, float rawValue, float min, float max){
        bool updated = false;
        if (mode == MODE_BRIGHTNESS) {
          if (bri != scaledValue) {
            bri = scaledValue;
            updated = true;
          }
        } else if (mode == MODE_EFFECT_SPEED) {
          if (effectSpeed != scaledValue) {
            effectSpeed = scaledValue;
            updated = true;
          }
        } else if (mode == MODE_EFFECT_INTENSITY) {
          if (effectIntensity != scaledValue) {
            effectIntensity = scaledValue;
            updated = true;
          }
        } else if (mode == MODE_EFFECT_FLOW_MODE) {
          if (rawValue > max || rawValue < min) {
            if (effectSpeed != 255) {
              effectSpeed = 255;
              updated = true;
            }
          } else {
            if (effectSpeed != 0) {
              effectSpeed = 0;
              updated = true;
            }          }
        } else if (mode == MODE_COLOR1_R) {
          col[0] = scaledValue;
          updated = true;
        } else if (mode == MODE_COLOR1_G) {
          col[1] = scaledValue;
          updated = true;
        } else if (mode == MODE_COLOR1_B) {
          col[2] = scaledValue;
          updated = true;
        }else if (mode == MODE_COLOR1I2_R) {
          col[0] = scaledValue;
          colSec[1] = scaledValue + 128;
          updated = true;
        } else if (mode == MODE_COLOR1I2_G) {
          col[1] = scaledValue;
          colSec[1] = scaledValue + 128;
          updated = true;
        } else if (mode == MODE_COLOR1I2_B) {
          col[2] = scaledValue;
          colSec[1] = scaledValue + 128;
          updated = true;
        }
        return updated;
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root)
    {
        //MOA TBD this should add the current values
      ////this code adds "u":{"AccelEnabled": true} to the info object
      //JsonObject user = root["u"];
      //if (user.isNull()) user = root.createNestedObject("u");

      //accelObj["AccelEnabled"] = sensorEnabled; //value
    }



    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      root["accel_sensor_enabled"] = sensorEnabled;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      sensorEnabled = root["accel_sensor_enabled"] | sensorEnabled; //if "accel_sensor_enabled" key exists in JSON, update, else keep old value
    }


    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     *
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     *
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     *
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     *
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("ADXL345SensorUsermod");
      top["sensor_enabled"] = sensorEnabled; //save these vars persistently whenever settings are saved
      top["update_interval"] = updateInterval; //save these vars persistently whenever settings are saved
      top["mode_x"] = modeX; //save these vars persistently whenever settings are saved
      top["mode_y"] = modeY; //save these vars persistently whenever settings are saved
      top["mode_z"] = modeZ; //save these vars persistently whenever settings are saved
      top["sensor_steps"] = sensorSteps; //save these vars persistently whenever settings are saved
      top["send_mqtt_event"] = sendMQTTEvent; //save these vars persistently whenever settings are saved
      top["send_raw_mqtt_event"] = sendRawMQTTEvent; //save these vars persistently whenever settings are saved
      top["min_x"] = minX; //save these vars persistently whenever settings are saved
      top["max_x"] = maxX; //save these vars persistently whenever settings are saved
      top["min_y"] = minY; //save these vars persistently whenever settings are saved
      top["max_y"] = maxY; //save these vars persistently whenever settings are saved
      top["min_z"] = minZ; //save these vars persistently whenever settings are saved
      top["max_z"] = maxZ; //save these vars persistently whenever settings are saved
      top["flow_min_x"] = flowMinX; //save these vars persistently whenever settings are saved
      top["flow_max_x"] = flowMaxX; //save these vars persistently whenever settings are saved
      top["flow_min_y"] = flowMinY; //save these vars persistently whenever settings are saved
      top["flow_max_y"] = flowMaxY; //save these vars persistently whenever settings are saved
      top["flow_min_z"] = flowMinZ; //save these vars persistently whenever settings are saved
      top["flow_max_z"] = flowMaxZ; //save these vars persistently whenever settings are saved
      top["enable_flow_speed"] = enableFlowSpeed;
      top["flow_speed"] = flowSpeed;
      top["static_speed"] = staticSpeed;
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     *
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     *
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     *
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     *
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root["ADXL345SensorUsermod"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["sensor_enabled"], sensorEnabled, false);
      configComplete &= getJsonValue(top["update_interval"], updateInterval, updateInterval);
      configComplete &= getJsonValue(top["mode_x"], modeX, 0);
      configComplete &= getJsonValue(top["mode_y"], modeY, 0);
      configComplete &= getJsonValue(top["mode_z"], modeZ, 0);
      configComplete &= getJsonValue(top["sensor_steps"], sensorSteps, DEFAULT_SENSOR_STEPS);
      configComplete &= getJsonValue(top["send_mqtt_event"], sendMQTTEvent, false);
      configComplete &= getJsonValue(top["send_raw_mqtt_event"], sendRawMQTTEvent, false);
      configComplete &= getJsonValue(top["min_x"], minX, minX);
      configComplete &= getJsonValue(top["max_x"], maxX, maxX);
      configComplete &= getJsonValue(top["min_y"], minY, minY);
      configComplete &= getJsonValue(top["max_y"], maxY, maxY);
      configComplete &= getJsonValue(top["min_z"], minZ, minZ);
      configComplete &= getJsonValue(top["max_z"], maxZ, maxZ);
      configComplete &= getJsonValue(top["flow_min_x"], flowMinX, flowMinX);
      configComplete &= getJsonValue(top["flow_max_x"], flowMaxX, flowMaxX);
      configComplete &= getJsonValue(top["flow_min_y"], flowMinY, flowMinY);
      configComplete &= getJsonValue(top["flow_max_y"], flowMaxY, flowMaxY);
      configComplete &= getJsonValue(top["flow_min_z"], flowMinZ, flowMinZ);
      configComplete &= getJsonValue(top["flow_max_z"], flowMaxZ, flowMaxZ);
      configComplete &= getJsonValue(top["enable_flow_speed"], enableFlowSpeed, enableFlowSpeed);
      configComplete &= getJsonValue(top["flow_speed"], flowSpeed, flowSpeed);
      configComplete &= getJsonValue(top["static_speed"], staticSpeed, staticSpeed);

      return configComplete;
    }


    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {

      //strip.setPixelColor(0, RGBW32(0,0,0,0)) // set the first pixel to black
    }


    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_ADXL345_SENSOR;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};
