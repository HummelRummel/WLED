#pragma once

#include "wled.h"

/*
 *
 * This Mod enables the HummelRummel interactive toy functionality. It extends WLED with some flow effects, adds support for gyro sensor and modifies the button behaviour.
 * Currently implemented:
 *   - TBD respond to status request on mqtt
 *   - TBD Button handler replacement to get rid of the resets, when pressed for longer than 10sec
 *   - TBD Changed switch button handling to directly send the on/off signals of the button
 * 
 */

#define HUMMELRUMMEL_USERMOD "HummelRummelUsermod"

// currently just a copy of wled001/json.cpp

#define JSON_PATH_STATE      1
#define JSON_PATH_INFO       2
#define JSON_PATH_STATE_INFO 3
#define JSON_PATH_NODES      4
#define JSON_PATH_PALETTES   5
#define JSON_PATH_FXDATA     6
#define JSON_PATH_NETWORKS   7
#define JSON_PATH_EFFECTS    8

//class name. Use something descriptive and leave the ": public Usermod" part :)
class HummelRummelUsermod : public Usermod {

  private:
    // Private class members. You can declare variables and functions only accessible to your usermod here
    bool initDone = false;
    bool enabled = false;
    unsigned long lastTime = 0;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    bool testBool = false;
    unsigned long testULong = 42424242;
    float testFloat = 42.42;
    String testString = "Forty-Two";

    // These config variables have defaults set inside readFromConfig()
    int testInt;
    long testLong;
    int8_t testPins[2];

    // // string that are used multiple time (this will save some flash memory)
    // static const char _name[];
    // static const char _enabled[];


    // any private methods should go here (non-inline method should be defined out of class)
    void publishStateViaMQTT(); 
    void publishInfoViaMQTT(); 
    void publishPalettesViaMQTT(); 
    void publishEffectsViaMQTT(); 
    void publishNetworksViaMQTT(); 
    void publishErrorViaMQTT(const char* err); 


  public:

    // non WLED related methods, may be used for data exchange between usermods (non-inline methods should be defined out of class)

    // /**
    //  * Enable/Disable the usermod
    //  */
    // inline void enable(bool enable) { 
    //       Serial.println("MOA12");

    //   enabled = enable; }

    // /**
    //  * Get usermod enabled/disabled state
    //  */
    // inline bool isEnabled() { return enabled; }

    // in such case add the following to another usermod:
    //  in private vars:
    //   #ifdef USERMOD_EXAMPLE
    //   MyExampleUsermod* UM;
    //   #endif
    //  in setup()
    //   #ifdef USERMOD_EXAMPLE
    //   UM = (MyExampleUsermod*) usermods.lookup(USERMOD_ID_EXAMPLE);
    //   #endif
    //  somewhere in loop() or other member method
    //   #ifdef USERMOD_EXAMPLE
    //   if (UM != nullptr) isExampleEnabled = UM->isEnabled();
    //   if (!isExampleEnabled) UM->enable(true);
    //   #endif


    // methods called by WLED (can be inlined as they are called only once but if you call them explicitly define them out of class)

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * readFromConfig() is called prior to setup()
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      // do your set-up here
      initDone = true;
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
            // Serial.println("MOA23");
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
      //      Serial.println("MOA26");

      // if usermod is disabled or called during strip updating just exit
      // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
      // if (!enabled || strip.isUpdating()) return;

      // do your magic here
      if (millis() - lastTime > 1000) {
        //Serial.println("I'm alive!");
            Serial.print("MOA13 ");
        Serial.print(millis());
        Serial.println();
        lastTime = millis();
      }
    }


    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root)
    {
      // if "u" object does not exist yet wee need to create it
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      //this code adds "u":{"ExampleUsermod":[20," lux"]} to the info object
      //int reading = 20;
      //JsonArray lightArr = user.createNestedArray(HUMMELRUMMEL_USERMOD)); //name
      //lightArr.add(reading); //value
      //lightArr.add(F(" lux")); //unit

      // if you are implementing a sensor usermod, you may publish sensor data
      //JsonObject sensor = root[F("sensor")];
      //if (sensor.isNull()) sensor = root.createNestedObject(F("sensor"));
      //temp = sensor.createNestedArray(F("light"));
      //temp.add(reading);
      //temp.add(F("lux"));
    }


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      // if (!initDone || !enabled) return;  // prevent crash on boot applyPreset()

      // JsonObject usermod = root[HUMMELRUMMEL_USERMOD];
      // if (usermod.isNull()) usermod = root.createNestedObject(HUMMELRUMMEL_USERMOD);

      // //usermod["user0"] = userVar0;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      // if (!initDone) return;  // prevent crash on boot applyPreset()

      // JsonObject usermod = root[HUMMELRUMMEL_USERMOD];
      // if (!usermod.isNull()) {
      //   // expect JSON usermod data in usermod name object: {"ExampleUsermod:{"user0":10}"}
      //   userVar0 = usermod["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
      // }
      // // you can as well check WLED state JSON keys
      // //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
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
      JsonObject top = root.createNestedObject("HummelRummelUsermod");
      top["enabled"] = enabled;
      //save these vars persistently whenever settings are saved
      top["great"] = userVar0;
      top["testBool"] = testBool;
      top["testInt"] = testInt;
      top["testLong"] = testLong;
      top["testULong"] = testULong;
      top["testFloat"] = testFloat;
      top["testString"] = testString;
      JsonArray pinArray = top.createNestedArray("pin");
      pinArray.add(testPins[0]);
      pinArray.add(testPins[1]); 
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

      JsonObject top = root[HUMMELRUMMEL_USERMOD];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["enabled"], enabled);
      configComplete &= getJsonValue(top["great"], userVar0);
      configComplete &= getJsonValue(top["testBool"], testBool);
      configComplete &= getJsonValue(top["testULong"], testULong);
      configComplete &= getJsonValue(top["testFloat"], testFloat);
      configComplete &= getJsonValue(top["testString"], testString);

      // A 3-argument getJsonValue() assigns the 3rd argument as a default value if the Json value is missing
      configComplete &= getJsonValue(top["testInt"], testInt, 42);  
      configComplete &= getJsonValue(top["testLong"], testLong, -42424242);

      // "pin" fields have special handling in settings page (or some_pin as well)
      configComplete &= getJsonValue(top["pin"][0], testPins[0], -1);
      configComplete &= getJsonValue(top["pin"][1], testPins[1], -1);

      return configComplete;
    }


    /*
     * appendConfigData() is called when user enters usermod settings page
     * it may add additional metadata for certain entry fields (adding drop down is possible)
     * be careful not to add too much as oappend() buffer is limited to 3k
     */
    void appendConfigData()
    {
      oappend(SET_F("addInfo('")); oappend(String(HUMMELRUMMEL_USERMOD).c_str()); oappend(SET_F(":great")); oappend(SET_F("',1,'<i>(this is a great config value)</i>');"));
      oappend(SET_F("addInfo('")); oappend(String(HUMMELRUMMEL_USERMOD).c_str()); oappend(SET_F(":testString")); oappend(SET_F("',1,'enter any string you want');"));
      oappend(SET_F("dd=addDropdown('")); oappend(String(HUMMELRUMMEL_USERMOD).c_str()); oappend(SET_F("','testInt');"));
      oappend(SET_F("addOption(dd,'Nothing',0);"));
      oappend(SET_F("addOption(dd,'Everything',42);"));
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


    /**
     * handleButton() can be used to override default button behaviour. Returning true
     * will prevent button working in a default way.
     * Replicating button.cpp
     */
    bool handleButton(uint8_t b) {
      yield();
      // ignore certain button types as they may have other consequences
      if (!enabled
       || buttonType[b] == BTN_TYPE_NONE
       || buttonType[b] == BTN_TYPE_RESERVED
       || buttonType[b] == BTN_TYPE_PIR_SENSOR
       || buttonType[b] == BTN_TYPE_ANALOG
       || buttonType[b] == BTN_TYPE_ANALOG_INVERTED) {
        return false;
      }
    // Serial.println("MOA11");

      bool handled = false;
      // do your button handling here
      return handled;
    }
  

#ifndef WLED_DISABLE_MQTT
    /**
     * handling of MQTT message
     * topic only contains stripped topic (part after /wled/MAC)
     */
    bool onMqttMessage(char* topic, char* payload) {
      if (!WLED_MQTT_CONNECTED) {
        Serial.println("receive message via mqtt, but no mqtt connection");
        return false;
      }
      Serial.println("MOA9");

      // check if we received a command
      if (strlen(topic) == 5 && strncmp_P(topic, PSTR("/json"), 5) == 0) {
            Serial.println("MOA10");

       String action = payload;
       if (action == "s") {
         publishStateViaMQTT();
         return true;
       } else if (action == "i") {
         publishInfoViaMQTT();
         return true;
       } else if (action == "p") {
         publishPalettesViaMQTT();
         return true;
       } else if (action == "e") {
         publishEffectsViaMQTT();
         return true;
       } else if (action == "n") {
         publishNetworksViaMQTT();
         return true;
       } else {
         publishErrorViaMQTT("unknown action");
         return true;
       }
      }
      return false;
    }

    /**
     * onMqttConnect() is called when MQTT connection is established
     */
    void onMqttConnect(bool sessionPresent) {
      Serial.println("MOA22");
      if (mqttDeviceTopic[0] == 0)
        return;


      String topic = String(mqttDeviceTopic) + "/json";
      Serial.println(topic);

      mqtt->subscribe(topic.c_str(), 0);

        //Check if MQTT Connected, otherwise it will crash the 8266
      if (WLED_MQTT_CONNECTED) {
        char ip[16] = "";
        IPAddress localIP = Network.localIP();
        sprintf(ip, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

        char subuf[64];
        strcpy(subuf, mqttDeviceTopic);
        strcat_P(subuf, PSTR("/ip"));
        mqtt->publish(subuf, 0, false, ip);
      }

      // do any MQTT related initialisation here
      //publishMqtt("I am alive!");
    }
#endif


    /**
     * onStateChanged() is used to detect WLED state change
     * @mode parameter is CALL_MODE_... parameter used for notifications
     */
    void onStateChange(uint8_t mode) {
      // do something if WLED state changed (color, brightness, effect, preset, etc)
    }


    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_HUMMELRUMMEL;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};


// add more strings here to reduce flash memory usage
// const char HummelRummelUsermod::_name[]    PROGMEM = "HummelRummelUsermod";
// const char HummelRummelUsermod::_enabled[] PROGMEM = "enabled";


// // implementation of non-inline member methods

void HummelRummelUsermod::publishStateViaMQTT()
{
#ifndef WLED_DISABLE_MQTT
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED) {

    // This seems to be the max size for the payload, not sure where it comes from
    char payload[1024];

    String topic = String(mqttDeviceTopic) + "/state";

    // get the lock for json
    if (!requestJSONBufferLock(21)) {
      return;
    }

    // clearup the doc and then fill it
    doc.clear();
    JsonObject state = doc.createNestedObject("state");
    serializeState(state);

    // calculate the needed buffer and in case it's enoufh allocate for the json
    size_t len = measureJson(doc)+1;
    if (len > sizeof(payload)) {
      releaseJSONBufferLock();
      publishErrorViaMQTT("payload too big");
      return;
    }

    // serialise and send it
    size_t n = serializeJson(doc, payload, sizeof(payload));
    uint16_t m = mqtt->publish(topic.c_str(), 0, false, payload, n);

    if (m == 0) {
      publishErrorViaMQTT("failed to publish");
    }

    // finally we can release everything
    releaseJSONBufferLock();
  }
#endif
}

void HummelRummelUsermod::publishInfoViaMQTT()
{
#ifndef WLED_DISABLE_MQTT
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED) {

    // This seems to be the max size for the payload, not sure where it comes from
    char payload[1024];

    String topic = String(mqttDeviceTopic) + "/info";

    // get the lock for json
    if (!requestJSONBufferLock(22)) {
      return;
    }

    // clearup the doc and then fill it
    doc.clear();
    JsonObject info = doc.createNestedObject("info");
    serializeInfo(info);

    // calculate the needed buffer and in case it's enoufh allocate for the json
    size_t len = measureJson(doc)+1;
    if (len > sizeof(payload)) {
      releaseJSONBufferLock();
      publishErrorViaMQTT("payload too big");
      return;
    }

    // serialise and send it
    size_t n = serializeJson(doc, payload, sizeof(payload));
    uint16_t m = mqtt->publish(topic.c_str(), 0, false, payload, n);

    // finally we can release everything
    releaseJSONBufferLock();
    if (m == 0) {
      publishErrorViaMQTT("failed to publish");
    }
  }
#endif
}


void HummelRummelUsermod::publishPalettesViaMQTT()
{
#ifndef WLED_DISABLE_MQTT
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED) {

    // This seems to be the max size for the payload, not sure where it comes from
    char payload[1024];

    String topic = String(mqttDeviceTopic) + "/pal";
   
    // get the lock for json
    if (!requestJSONBufferLock(23)) {
      return;
    }

    // clearup the doc and then fill it
    doc.clear();
    doc["palettes"] = serialized((const __FlashStringHelper*)JSON_palette_names);

    // send it
    size_t n = serializeJson(doc, payload, sizeof(payload));
    uint16_t m = mqtt->publish(topic.c_str(), 0, false, payload, n);

    if (m == 0) {
      publishErrorViaMQTT("failed to publish");
    }
  }
#endif
}


void HummelRummelUsermod::publishEffectsViaMQTT()
{
#ifndef WLED_DISABLE_MQTT
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED) {

    // This seems to be the max size for the payload, not sure where it comes from
    char payload[1024];

    String topic = String(mqttDeviceTopic) + "/eff";

    // get the lock for json
    if (!requestJSONBufferLock(24)) {
      return;
    }

    // clearup the doc and then fill it
    doc.clear();
    JsonArray effect = doc.createNestedArray("effect");
    serializeModeNames(effect);

    // calculate the needed buffer and in case it's enoufh allocate for the json
    size_t len = measureJson(doc)+1;
    if (len > sizeof(payload)) {
      releaseJSONBufferLock();
      publishErrorViaMQTT("payload too big");
      return;
    }

    // serialise and send it
    size_t n = serializeJson(doc, payload, sizeof(payload));
    uint16_t m = mqtt->publish(topic.c_str(), 0, false, payload, n);

    // finally we can release everything
    releaseJSONBufferLock();
    if (m == 0) {
      publishErrorViaMQTT("failed to publish");
    }
  }
#endif
}


// void HummelRummelUsermod::publishEffectsViaMQTT()
// {
// #ifndef WLED_DISABLE_MQTT
//   //Check if MQTT Connected, otherwise it will crash the 8266
//   if (WLED_MQTT_CONNECTED) {
//     // This seems to be the max size for the payload, not sure where it comes from
//     char payload[1024];

//     String topic = String(mqttDeviceTopic) + "/eff";

//     // get the lock for json
//     if (!requestJSONBufferLock(21)) {
//       return;
//     }

//     // clearup the doc and then fill it
//     doc.clear();
//     JsonVariant _root = doc.to<JsonObject>();
//     serializeModeNames(_root);

//     // calcula.tote the needed buffer and in case it's enoufh allocate for the json
//     size_t len = measureJson(doc)+1;
//     if (len > sizeof(payload)) {
//       releaseJSONBufferLock();
//       publishErrorViaMQTT("payload too big");
//       return;
//     }

//     // serialise and send it
//     size_t n = serializeJson(doc, payload, sizeof(payload));
//     uint16_t m = mqtt->publish(topic.c_str(), 0, false, payload, n);

//     // finally we can release everything
//     releaseJSONBufferLock();
//     if (m == 0) {
//       publishErrorViaMQTT("failed to publish");
//     }
//   }
// #endif
// }

void HummelRummelUsermod::publishNetworksViaMQTT()
{
#ifndef WLED_DISABLE_MQTT
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED) {
    // This seems to be the max size for the payload, not sure where it comes from
    char payload[1024];

    String topic = String(mqttDeviceTopic) + "/net";

    // get the lock for json
    if (!requestJSONBufferLock(25)) {
      return;
    }

    // clearup the doc and then fill it
    doc.clear();
    JsonObject net = doc.createNestedObject("networks");
    serializeNetworks(net);

    // calculate the needed buffer and in case it's enoufh allocate for the json
    size_t len = measureJson(doc)+1;
    if (len > sizeof(payload)) {
      releaseJSONBufferLock();
      publishErrorViaMQTT("payload too big");
      return;
    }

    // serialise and send it
    size_t n = serializeJson(doc, payload, sizeof(payload));
    uint16_t m = mqtt->publish(topic.c_str(), 0, false, payload, n);

    // finally we can release everything
    releaseJSONBufferLock();
    if (m == 0) {
      publishErrorViaMQTT("failed to publish");
    }
  }
#endif
}

void HummelRummelUsermod::publishErrorViaMQTT(const char* error)
{
#ifndef WLED_DISABLE_MQTT
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED) {
    Serial.println("MOA1");
    char subuf[64];
    strcpy(subuf, mqttDeviceTopic);
    strcat_P(subuf, PSTR("/err"));
    mqtt->publish(subuf, 0, false, error);
  }
#endif
}
