/**
 * Example for the ESP32 HTTP(S) Webserver: https://github.com/fhessel/esp32_https_server
 * Docs: https://fhessel.github.io/esp32_https_server/index.html
 *
 * IMPORTANT NOTE:
 * This example is a bit more complex than the other ones, so be careful to
 * follow all steps.
 *
 * Make sure to check out the more basic examples like Static-Page to understand
 * the fundamental principles of the API before proceeding with this sketch.
 *
 * To run this script, you need to
 *  1) Enter your WiFi SSID and PSK below this comment
 *  2) Install the SPIFFS File uploader into your Arduino IDE to be able to
 *     upload static data to the webserver.
 *     Follow the instructions at:
 *     https://github.com/me-no-dev/arduino-esp32fs-plugin
 *  3) Upload the static files from the data/ directory of the example to your
 *     module's SPIFFs by using "ESP32 Sketch Data Upload" from the tools menu.
 *     If you face any problems, read the description of the libraray mentioned
 *     above.
 *     Note: If mounting SPIFFS fails, the script will wait for a serial connection
 *     (open your serial monitor!) and ask if it should format the SPIFFS partition.
 *     You may need this before uploading the data
 *     Note: Make sure to select a partition layout that allows for SPIFFS in the
 *     boards menu
 *  4) Have the ArduinoJSON library installed and available. (Tested with Version 6.17.2)
 *     You'll find it at:
 *     https://arduinojson.org/
 *
 * This script will install an HTTPS Server on your ESP32 with the following
 * functionalities:
 *  - Serve static files from the SPIFFS's data/public directory
 *  - Provide a REST API at /api to receive the asynchronous http requests
 *    - /api/uptime provides access to the current system uptime
 *    - /api/events allows to register or delete events to turn PINs on/off
 *      at certain times.
 *  - Use Arduino JSON for body parsing and generation of responses.
 *  - The certificate is generated on first run and stored to the SPIFFS in
 *    the cert directory (so that the client cannot retrieve the private key)
 */
// (ESP32 Wireless Mode) IP Search APP: Angry IP Scanner
// --- Unity can only send String instead of Int: comment out the validation --- */


/**
 * Things to fix/TODO:
 * - GET http://xxxx/api/events still doesn't return added events w/ larger time
 * - separate Arduino code for maintainess
 */

#include <Arduino.h>


// TODO: make turnOff happens immediately
// TODO: Configure your WiFi in http_functions.cpp

//#define WIFI_SSID "jamie"
//#define WIFI_PSK "jamierosie"

//String HOSTNAME = "Left_1";


// Include certificate data within the same folder of this file
#include "cert.h"
#include "private_key.h"

// We will use wifi
#include <WiFi.h>

// We will use SPIFFS and FS
#include <SPIFFS.h>
#include <FS.h>

// We use JSON as data format. Make sure to have the lib available
#include <ArduinoJson.h>

// Working with c++ strings
#include <string>

// Define the name of the directory for public files in the SPIFFS parition
#define DIR_PUBLIC "/public"
//#define DIR_PUBLIC "/"

// We need to specify some content-type mapping, so the resources get delivered with the
// right content type and are displayed correctly in the browser
char contentTypes[][2][32] = {
  {".html", "text/html"},
  {".css",  "text/css"},
  {".js",   "application/javascript"},
  {".json", "application/json"},
  {".png",  "image/png"},
  {".jpg",  "image/jpg"},
  {"", ""}
};

/* ------------------------------------------- */
// Includes for setting up the HTTPS/HTTP server
#include <HTTPSServer.hpp>  // HTTPS: only Firefox
#include <HTTPServer.hpp>  // HTTP: Chrome,..all browsers
/* ------------------------------ */

// Define the certificate data for the server (Certificate and private key)
#include <SSLCert.hpp>

// Includes to define request handler callbacks
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Required do define ResourceNodes
#include <ResourceNode.hpp>

// Easier access to the classes of the server. The HTTPS Server comes in a separate namespace.
using namespace httpsserver;

// Create an SSL certificate object from the files included above
// (see extras/README.md on how to create it)
SSLCert cert = SSLCert(
  example_crt_DER, // DER-formatted certificate data
  example_crt_DER_len, // length of the certificate data
  example_key_DER, // private key for that certificate
  example_key_DER_len // Length of the private key
);

// Create an SSL-enabled server that uses the certificate
// The contstructor takes some more parameters, but we go for default values here.
//HTTPSServer secureServer = HTTPSServer(&cert);

/* ------------------------------ */
// We just create a reference to the server here. We cannot call the constructor unless
// we have initialized the SPIFFS and read or created the certificate
//HTTPSServer * secureServer; // HTTPS
HTTPServer * secureServer;  // HTTP
/* ------------------------------------------- */

// Declare some handler functions for the various URLs on the server
// The signature is always the same for those functions. They get two parameters,
// which are pointers to the request data (read request body, headers, ...) and
// to the response data (write response, set status code, ...)
void connectToWiFi();
// void setup_routing(HTTPServer * secureServer);
void mountSPIFF();
void setup_task();

void handleSPIFFS(HTTPRequest * req, HTTPResponse * res);
void handleGetUptime(HTTPRequest * req, HTTPResponse * res);
void handleGetEvents(HTTPRequest * req, HTTPResponse * res);
void handlePostEvent(HTTPRequest * req, HTTPResponse * res);
void handleDeleteEvent(HTTPRequest * req, HTTPResponse * res);

// We use the following struct to store GPIO events:
#define MAX_EVENTS 50
struct {
  // is this event used (events that have been run will be set to false)
  bool active;
  // when should it be run?
  unsigned long time;  // 14400000 ms == 4 hours
  // which GPIO should be changed?
  int gpio;
  // int gpio[] = {}; // Array to store pins when running iterateSingle pattern?
  // and to which state? analog or digital?
  int intensity;
  // which mode? mode 1: real time
  int mode;
  // duration/frequency
  int duration;
  // how many rounds for the specific mode to run?
  int round;
} events[MAX_EVENTS];

// ************************** Vibration Motor **************************
#include <analogWrite.h>
const int rows = 4;
const int columns = 5;

void logParams(int duration, int intensity);
void turnAllMotorsOff(int motors[][columns]);
void heartBeatAll(int motors[][columns], int intensity, int duration, int round);
void iterateElements(int motors[][columns], int intensity, int duration, int round);
// void iterateSingle(int pins[], int intensity, int duration, int round);
void iterateSingle(int pin, int intensity, int duration, int round);
void touchEachOther(int motors[][columns], int intensity, int duration, int round);
void spread(int motors[][columns], int intensity, int duration, int round);
void growSingle(int pin, int intensity, int duration, int round);
void goUpAll(int motors[][columns], int intensity, int duration, int round);
void goUpDownAll(int motors[][columns], int intensity, int duration, int round);
void rainingHeavily(int motors[][columns], int intensity, int duration, int round);
void crushingStuff(int motors[][columns], int intensity, int duration, int round);


/********* for testing ***********/
void heartBeatTest(int motors[][columns], int intensity, int duration);
/**************************************/

// Left-Hand
int motorsArr[rows][columns] = {
                                  {19, 26, 27, 14, 23}, // thumb~little
                                  {5, 13, 33, 12, 22},
                                  {21, 17, 16, 18, 4}, // dw1(thumb), dw2, up1, dw3, up2
                                  {32, 25, 15, -1, -1} // back1(pinky)~3
                                };

// Right-Hand
// int motorsArr[rows][columns] = {
//                                   {23, 12, 33, 13, 19}, // thumb~little
//                                   {22, 14, 27, 26, 5},
//                                   {4, 18, 32, 21, 17},  // dwPalm1(thumb), dw2, up1, up2, dw3
//                                   {16, 15, 25, -1, -1} // back1(pinky)~3
//                                 };

//const int motorAmplitude = 100; // 0-255

/*---------------------------------------------*/
void setup() {
  // For logging
  Serial.begin(115200);

  // setup_task();

  // Set the pins that we will use as output pins
  for (int i=0; i<rows; i++) {
    for (int j=0; j<columns; j++) {
      pinMode(motorsArr[i][j], OUTPUT);
    }
  }

  mountSPIFF();

  // Initialize event structure:
  for(int i = 0; i < MAX_EVENTS; i++) {
    // events[i] = {false, 0, 0, 0, 0, 0, 0};
    // is this event used (events that have been run will be set to false)
    events[i].active = false;
    // when should it be run?
    events[i].time = 0;  // 14400000 ms == 4 hours
    // which GPIO should be changed?
    events[i].gpio = 0;
    // and to which state? analog or digital?
    events[i].intensity = 0;
    // which mode? mode 1: real time
    events[i].mode = 0;
    // duration/frequecy
    events[i].duration = 0;
    // how many rounds for the specific mode to run?
    events[i].round = 0;
  }

  connectToWiFi();
  // setup_routing(secureServer);

/* ------------------------------ */
// Create the server with the certificate we loaded before
// Use the & operator to store the memory address of the variable called cert, and assign it to the pointer.
//  secureServer = new HTTPSServer(&cert); // HTTPS
  secureServer = new HTTPServer(); // HTTP
/* ------------------------------ */

  // We register the SPIFFS handler as the default node, so every request that does
  // not hit any other node will be redirected to the file system.
  ResourceNode * spiffsNode = new ResourceNode("", "", &handleSPIFFS);
  secureServer->setDefaultNode(spiffsNode);

  // Add a handler that serves the current system uptime at GET /api/uptime
  ResourceNode * uptimeNode = new ResourceNode("/api/uptime", "GET", &handleGetUptime);
  secureServer->registerNode(uptimeNode);

  // Add the handler nodes that deal with modifying the events:
  ResourceNode * getEventsNode = new ResourceNode("/api/events", "GET", &handleGetEvents);
  secureServer->registerNode(getEventsNode);
  ResourceNode * postEventNode = new ResourceNode("/api/events", "POST", &handlePostEvent);
  secureServer->registerNode(postEventNode);
  ResourceNode * deleteEventNode = new ResourceNode("/api/events/*", "DELETE", &handleDeleteEvent);
  secureServer->registerNode(deleteEventNode);

  Serial.println("Starting server...");
  secureServer->start();
  if (secureServer->isRunning()) {
    Serial.println("Server ready.");
  }

  // randomSeed(10);
}

void loop() {
  // Serial.print("loop() running on core ");
  // Serial.println(xPortGetCoreID());

  // This call will let the server do its work
  secureServer->loop();

  // Here we handle the events
  // turnAllMotorsOff(); ?????
  unsigned long now = millis() / 1000;
  for (int i = 0; i < MAX_EVENTS; i++) {
    // Only handle active events:
    if (events[i].active) {
      // Only if the counter has recently been exceeded
      // if (events[i].time < now) {
/* ------------------------------------------------------------- */
//        if (events[i].gpio == vmotor) {
//          analogWrite(vmotor, (events[i].intensity == 1) ? motorAmplitude : 0);
//        } else {
//          // Apply the state change
//          digitalWrite(events[i].gpio, events[i].intensity);
//        }
/* ---------------------------- Haptic Pattern Modes --------------------------------- */
        if (events[i].mode == 0) {
          // digitalWrite(events[i].gpio, events[i].intensity);
          // this for loop works correctly with an array of any type or size
          // for (byte j = 0; j < (sizeof(events[i].gpio) / sizeof(events[i].gpio[0])); j++) {
          //   analogWrite(events[i].gpio[j], events[i].intensity); // forever
          // }

          analogWrite(events[i].gpio, events[i].intensity); // forever
        }
        // else if (events[i].mode == 1) {
        //   while (1) {
        //     heartBeatAll(motorsArr, events[i].intensity, events[i].duration);
        //   }
        //   // while (1) {
        //   //  iterateSingle(events[i].gpio, events[i].intensity, events[i].duration, events[i].round);
        //   // }
        //   // while (1) {
        //     analogWrite(events[i].gpio, random(100, events[i].intensity));
        //     delay(random(20, events[i].duration)); // random

        //     analogWrite(events[i].gpio, 0);
        //     delay(random(20, events[i].duration)); // random
        //   // }
        // }
        else {
          switch (events[i].mode) // switch-case: only runs 1 time???
          {
            case 1:
              // intensity=[180, 120] duration=120
              heartBeatAll(motorsArr, events[i].intensity, events[i].duration, events[i].round);
              break;
            case 2:
              // intensity=[180, 90] duration=180
              iterateElements(motorsArr, events[i].intensity, events[i].duration, events[i].round);
              break;
            case 3:
              // intensity=[180, 120] duration=120
              touchEachOther(motorsArr, events[i].intensity, events[i].duration, events[i].round);
              break;
            case 4:
              // intensity=[200, 120] duration=120
              spread(motorsArr, events[i].intensity, events[i].duration, events[i].round);
              break;
            case 5:
              // gpio=12 round=10 intensity=[250, 180] duration=120
              iterateSingle(events[i].gpio, events[i].intensity, events[i].duration, events[i].round);
              break;
            case 6:
              // intensity=[200, 120] duration=120
              growSingle(events[i].gpio, events[i].intensity, events[i].duration, events[i].round);
              break;
            case 7:
              // intensity=[160, 120], duration=5-10 (micro seconds), round=2
              // Magnetic water
              goUpAll(motorsArr, events[i].intensity, events[i].duration, events[i].round);
              break;
            case 8:
              // intensity=[160, 120], duration=5-10 (micro seconds), round=2
              // Magnetic water
              goUpDownAll(motorsArr, events[i].intensity, events[i].duration, events[i].round);
              break;
            case 9:
              turnAllMotorsOff(motorsArr);
              break;
            case 10:
              rainingHeavily(motorsArr, events[i].intensity, events[i].duration, events[i].round);
            case 11:
              crushingStuff(motorsArr, events[i].intensity, events[i].duration, events[i].round);
            default:
              break;
          }
        }

        // Deactivate the event so it doesn't fire again
        events[i].active = false;
      // }
    }
  }

  // Other code would go here...
  delay(1);
}

/**
 * This handler function will try to load the requested resource from SPIFFS's /public folder.
 * If the method is not GET, it will throw 405, if the file is not found, it will throw 404.
 */
void handleSPIFFS(HTTPRequest * req, HTTPResponse * res) {
  // We only handle GET here
  if (req->getMethod() == "GET") {
    // Redirect / to /index.html
    std::string reqFile = req->getRequestString()=="/" ? "/index.html" : req->getRequestString();

    // Try to open the file
    std::string filename = std::string(DIR_PUBLIC) + reqFile;

    // Check if the file exists
    if (!SPIFFS.exists(filename.c_str())) {
      // Send "404 Not Found" as response, as the file doesn't seem to exist
      res->setStatusCode(404);
      res->setStatusText("Not found");
      res->println("404 Not Found");
      return;
    }

    File file = SPIFFS.open(filename.c_str());

    // Set length
    res->setHeader("Content-Length", httpsserver::intToString(file.size()));

    // Content-Type is guessed using the definition of the contentTypes-table defined above
    int cTypeIdx = 0;
    do {
      if(reqFile.rfind(contentTypes[cTypeIdx][0])!=std::string::npos) {
        res->setHeader("Content-Type", contentTypes[cTypeIdx][1]);
        break;
      }
      cTypeIdx+=1;
    } while(strlen(contentTypes[cTypeIdx][0])>0);

    // Read the file and write it to the response
    uint8_t buffer[256];
    size_t length = 0;
    do {
      length = file.read(buffer, 256);
      res->write(buffer, length);
    } while (length > 0);

    file.close();
  } else {
    // If there's any body, discard it
    req->discardRequestBody();
    // Send "405 Method not allowed" as response
    res->setStatusCode(405);
    res->setStatusText("Method not allowed");
    res->println("405 Method not allowed");
  }
}

/**
 * This function will return the uptime in seconds as JSON object:
 * {"uptime": 42}
 */
void handleGetUptime(HTTPRequest * req, HTTPResponse * res) {
  // Create a buffer of the size of a JSON object with n members
  // (pretty simple, we have just one key here)
  StaticJsonDocument<JSON_OBJECT_SIZE(1)> doc;

  // Set the uptime key to the uptime in seconds
  doc["uptime"] = millis()/1000;
  // Set the content type of the response
  res->setHeader("Content-Type", "application/json");
  // Serialize the object and send the result to Serial.
  // Remember to use *, as we only have a pointer to the HTTPResponse here:
  serializeJson(doc, *res); // output: {"uptime": 42}
}

/**
 * This handler will return a JSON array of currently active events for GET /api/events
 * [{"gpio":27,"intensity":1,"time":42,"id":0}, ...]
 */
void handleGetEvents(HTTPRequest * req, HTTPResponse * res) {
  // We need to calculate the capacity of the json buffer
  int activeEvents = 0;
  for(int i = 0; i < MAX_EVENTS; i++) {
    if (events[i].active) activeEvents++;
  }

  // For each active event, we need 1 array element with 4 objects
  const size_t capacity = JSON_ARRAY_SIZE(activeEvents) + activeEvents * JSON_OBJECT_SIZE(4);
//  const size_t capacity = 1024;

  // DynamicJsonDocument is created on the heap instead of the stack
  DynamicJsonDocument doc(capacity);
  JsonArray arr = doc.to<JsonArray>();
  for(int i = 0; i < MAX_EVENTS; i++) {
    if (events[i].active) {
      JsonObject eventObj = arr.createNestedObject();
      eventObj["gpio"] = events[i].gpio;
      // eventObj["gpios"] = events[i].gpios;
      eventObj["intensity"] = events[i].intensity;
      eventObj["time"] = events[i].time;
      eventObj["mode"] = events[i].mode;
      eventObj["duration"] = events[i].duration;
      eventObj["round"] = events[i].round;
      // Add the index to allow delete and post to identify the element
      eventObj["id"] = i;
    }
  }

  // Print to response
  res->setHeader("Content-Type", "application/json");
  serializeJson(arr, *res); // output: [{"gpio":27,"intensity":1,"time":42,"id":0}, ...]
}

void handlePostEvent(HTTPRequest * req, HTTPResponse * res) {
  // We expect an object with 8 elements and add some buffer
  const size_t capacity = JSON_OBJECT_SIZE(8) + 180;
  DynamicJsonDocument reqDoc(capacity);

  // Create buffer to read request
  char * buffer = new char[capacity + 1];
  memset(buffer, 0, capacity+1);

  // Try to read request into buffer
  size_t idx = 0;
  // while "not everything read" or "buffer is full"
  while (!req->requestComplete() && idx < capacity) {
    idx += req->readChars(buffer + idx, capacity-idx);
  }

  // If the request is still not read completely, we cannot process it.
  if (!req->requestComplete()) {
    res->setStatusCode(413);
    res->setStatusText("Request entity too large");
    res->println("413 Request entity too large");
    // Clean up
    delete[] buffer;
    return;
  }

  // Parse the object
  // Extract JSON to an object
  DeserializationError error = deserializeJson(reqDoc, buffer);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
/* --------------------------------------------------------------- */
  // Check input data types
  bool dataValid = true;
  // reqDoc["time"].is<long>()
  if (!reqDoc["time"].is<int>() || !reqDoc["gpio"].is<int>() || !reqDoc["intensity"].is<int>()) {
    dataValid = false;
  }

  // Check actual values
  // unsigned long eTime = 0;
  unsigned long eTime = 0;
  int eGpio = 0;
  // int eGpios = {};
  // int eIntensity = LOW; // digital
  int eIntensity = 0; // analog
  int eMode = 0; // haptic pattern mode
  int eDuration = 0;
  int eRounds = 0;

  if (dataValid) {
    eTime = reqDoc["time"];
//    if (eTime < millis()/1000) dataValid = false;

    eGpio = reqDoc["gpio"];
    // eGpios = reqDoc["gpios"];
    // if (!(eGpio == GPIO_1 || eGpio == GPIO_2 || eGpio == GPIO_3 || eGpio == GPIO_4 || eGpio == GPIO_5)) dataValid = false;
//    for (int i = 0; i < rows; i++) {
//      for (int j = 0; j < columns; j++) {
//        if (eGpio != motors[i][j]) dataValid = false;
//      }
//    }

    eIntensity = reqDoc["intensity"];
    // if (eIntensity != HIGH && eIntensity != LOW) dataValid = false;
    if (eIntensity < 0 || eIntensity > 255) dataValid = false;

    eMode = reqDoc["mode"];
    eDuration = reqDoc["duration"];
    eRounds = reqDoc["round"];
    // if (eMode > 2 || eDuration < 0 || eRounds < 0) dataValid = false;
     if (eDuration < 0 || eRounds < 0) dataValid = false;
  }

  Serial.println(reqDoc["intensity"].as<int>());
  Serial.println("eIntensity: ");
  Serial.println(eIntensity);
  Serial.print("eMode: ");
  Serial.println(eMode);
  Serial.println("before 404");

  // Clean up, we don't need the buffer any longer
  delete[] buffer;

  // If something failed: 400
  /* ------------------------------------------- */
  dataValid = true;
  /* ------------------------------------------- */
  // ??? command line POST not working but browser UI works!
  if (!dataValid) {
    res->setStatusCode(400);
    res->setStatusText("Bad Request");
    res->println("400 Bad Request");

    res->print("dataValid: ");
    res->println(dataValid);
    res->print("eTime: ");
//    res->println(eTime); // 0
    res->println(reqDoc["time"].as<long>());
    res->print("eGpio: "); // 0
//    res->println(eGpio);
    res->println(reqDoc["gpio"].as<int>());
    // res->print("eGpios: "); // 0
    // for(JsonVariant v : reqDoc["gpios"]) {
    //  res->println(v.as<int>());
    // }
    // res->println(reqDoc["gpios"].as<int>());
    res->print("eIntensity: ");
//    res->println(eIntensity); // 0
    res->println(reqDoc["intensity"].as<int>());
    res->print("eMode: ");
    res->println(reqDoc["mode"].as<int>());
    res->print("eDuration: ");
    res->println(reqDoc["duration"].as<int>());
    res->print("eRounds: ");
    res->println(reqDoc["round"].as<int>());
    return;
  }

   Serial.println(reqDoc["intensity"].as<int>());
   Serial.print("eIntensity: ");
   Serial.println(eIntensity);

  // Try to find an inactive event in the list to write the data to
  int eventID = -1;
  for(int i = 0; i < MAX_EVENTS && eventID == -1; i++) {
    if (!events[i].active) {
      eventID = i;
//      events[i].gpio = eGpio;  // 0, not working w/ httpPie POST command
      events[i].gpio = reqDoc["gpio"].as<int>(); // works w/ httpPie POST command
      // events[i].gpios = reqDoc["gpios"];
      // copyArray(reqDoc["gpios"].as<JsonArray(), events[i].gpios);
      //   copyArray(reqDoc["gpios"], events[i].gpios);
//      events[i].time = eTime;  // 0
      events[i].time = reqDoc["time"].as<long>();
//      events[i].intensity = eIntensity;  // 0
      events[i].intensity = reqDoc["intensity"].as<int>();
      events[i].mode = reqDoc["mode"].as<int>();
      events[i].duration = reqDoc["duration"].as<int>();
      events[i].round = reqDoc["round"].as<int>();
      events[i].active = true;
    }
  }

  // Check if we could store the event
  if (eventID > -1) {
    // Create a buffer for the response
    StaticJsonDocument<JSON_OBJECT_SIZE(8)> resDoc;

    // Set the uptime key to the uptime in seconds
    // ??? always returns 0
//    resDoc["gpio"] = events[eventID].gpio;
//    resDoc["intensity"] = events[eventID].intensity;
//    resDoc["time"] = events[eventID].time;
//    resDoc["id"] = eventID;

    resDoc["gpio"] = reqDoc["gpio"].as<int>();
    // resDoc["gpios"] = reqDoc["gpios"].as<int>();
    resDoc["intensity"] = reqDoc["intensity"].as<int>();
    resDoc["time"] = reqDoc["time"].as<long>();
    resDoc["mode"] = reqDoc["mode"].as<int>();
    resDoc["duration"] = reqDoc["duration"].as<int>();
    resDoc["round"] = reqDoc["round"].as<int>();
    resDoc["id"] = eventID;

    // Write the response
    res->setHeader("Content-Type", "application/json");
    serializeJson(resDoc, *res); // output: {"gpio": 2, "intensity": "HIGH", "time": 42, "id": 1}

  } else {
    // We could not store the event, no free slot.
    res->setStatusCode(507);
    res->setStatusText("Insufficient storage");
    res->println("507 Insufficient storage");

  }
}

/**
 * This handler will delete an event (meaning: deactive the event)
 */
void handleDeleteEvent(HTTPRequest * req, HTTPResponse * res) {
  // Access the parameter from the URL. See Parameters example for more details on this
  ResourceParameters * params = req->getParams();
  size_t eid = std::atoi(params->getPathParameter(0).c_str());

  if (eid < MAX_EVENTS) {
    // Set the inactive flag
    events[eid].active = false;
    // And return a successful response without body
    res->setStatusCode(204);
    res->setStatusText("No Content");
  } else {
    // Send error message
    res->setStatusCode(400);
    res->setStatusText("Bad Request");
    res->println("400 Bad Request");
  }
}
/*---------------------------------------------*/