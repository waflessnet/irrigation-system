#include <Arduino.h>
#include <FS.h>

/*#define MYFS SPIFFS*/
#define MYFS LittleFS

#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

/**
 *  @Autor: Francisco Cespedes A.
 *  @Description: SISTEMA DE RIEGO
 *  Chipset: ESP8266
 *  Board: NodeMCU v1.0 / V2
 *  email : ncwapuntes@gmail.com
 */

#define RELAY_1 D4
#define RELAY_2 D7
#define LED_GREEN D5 // for check relay 1  status | on is enable
#define LED_YELLOW D6 // for check relay 2  status | on is enable
#define LED_RED D8 // get data internet
#define ENABLE  "1"
#define DISABLE  "0"
#define N_RELAY_1  "1"
#define N_RELAY_2  "2"


const char *wifi_name = "HUAWEI-B2368-8A60FE";
const char *wifi_password = "HV34MRJ3";
//----------------------

const char *PARAM_RELAY_CHANGE_STATUS = "state";
const char *PARAM_RELAY_NAME = "relay";
/**
 * set status global for used in web page
 * two relay default disable.
 */
String status_relay_1 = DISABLE;
String status_relay_2 = DISABLE;

AsyncWebServer server(80);

// TaskHandle_t Core0TaskHnd;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void wifi_init() {
    delay(50);
    Serial.println("init connection wifi:");
    WiFi.begin(wifi_name, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("\n connection wifi ok");
    Serial.println("init server: HOME GARDEN IRRIGATION SYSTEM");
    Serial.print("IP local server: ");
    Serial.print(WiFi.localIP());
    Serial.println("");
}

void data_process_led_red() {
    digitalWrite(LED_RED, HIGH);
    delay(1500);
    digitalWrite(LED_RED, LOW);

}

void set_state_led_green(String state) {
    if (state == ENABLE) {
        digitalWrite(LED_GREEN, HIGH);
    } else {
        digitalWrite(LED_GREEN, LOW);
    }
}

void set_state_led_yellow(String state) {
    if (state == ENABLE) {
        digitalWrite(LED_YELLOW, HIGH);
    } else {
        digitalWrite(LED_YELLOW, LOW);
    }
}

/**
 * relay 1 is led green
 * relay 2 is led yellow
 * @param relay
 * @param state
 */
void change_state_relay(String relay, String state) {
    if (relay == N_RELAY_1) {
        set_state_led_green(state);
        if (state == ENABLE) {
            digitalWrite(RELAY_1, LOW); // SET ON RELAY
            status_relay_1 = ENABLE;
        } else {
            digitalWrite(RELAY_1, HIGH); // SET ON RELAY
            status_relay_1 = DISABLE;
        }
    }
    if (relay == N_RELAY_2) {
        set_state_led_yellow(state);
        if (state == ENABLE) {
            digitalWrite(RELAY_2, LOW); // SET ON RELAY
            status_relay_2 = ENABLE;
        } else {
            digitalWrite(RELAY_2, HIGH); // SET ON RELAY
            status_relay_2 = DISABLE;
        }
    }

}

/**
 * Server
 *
 */
void server_init(AsyncWebServer &_server) {

    /**
     * server api
     */
    _server.on("/work", HTTP_GET, [](AsyncWebServerRequest *request) {
        data_process_led_red();
        String pageHTML = "It's Work";
        request->send(200, "text/html", pageHTML);
    });
    // Send a GET request to <IP>/relay/set?relay=<*relay>&status=<status>
    _server.on("/relay/set", HTTP_GET, [](AsyncWebServerRequest *request) {
        data_process_led_red();
        String name_relay;
        String state_relay;

        if (request->hasParam(PARAM_RELAY_CHANGE_STATUS) && request->hasParam(PARAM_RELAY_NAME)) {
            state_relay = request->getParam(PARAM_RELAY_CHANGE_STATUS)->value();
            name_relay = request->getParam(PARAM_RELAY_NAME)->value();
            Serial.println("name relay: " + name_relay + " status to assing:" + state_relay);
            change_state_relay(name_relay, state_relay);

            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        } else {
            String not_valid = "Not valid request";
            AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", not_valid);
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        }

    });

    _server.on("/relay/get", HTTP_GET, [](AsyncWebServerRequest *request) {
        data_process_led_red();
        String name_relay;
        String state_relay;
        if (request->hasParam(PARAM_RELAY_NAME)) {
            name_relay = request->getParam(PARAM_RELAY_NAME)->value();
            Serial.println("Get status relay number: " + name_relay);


            if (name_relay == N_RELAY_1) {
                String res = "{\"state\":" + status_relay_1 +"}";
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", res);
                response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response);
            } else if (name_relay == N_RELAY_2) {
                String res = "{\"state\":" + status_relay_2 +"}";
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", res);
                response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response);
            } else {
                AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "Not valid relay");
                response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response);
            }

        } else {
            name_relay = "not valid request";
            request->send(400, "text/plain", "Nok");
        }

    });

    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        data_process_led_red();
        Serial.println("request to /");
        AsyncWebServerResponse *response = request->beginResponse(MYFS, "/index.html", "text/html");
        // response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });


    _server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        data_process_led_red();
        Serial.println("request to /index.html");
        AsyncWebServerResponse *response = request->beginResponse(MYFS, "/index.html", "text/html");
        // response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
    _server.onNotFound(notFound);
    _server.begin();
}

void init_message() {
    Serial.println(
            "\n"
            "#   _____  ______  ______ _____  ______ _______ _______ _____  _____  __   _      _______ __   __ _______ _______ _______ _______\n"
            "#     |   |_____/ |_____/   |   |  ____ |_____|    |      |   |     | | \\  |      |______   \\_/   |______    |    |______ |  |  |\n"
            "#   __|__ |    \\_ |    \\_ __|__ |_____| |     |    |    __|__ |_____| |  \\_|      ______|    |    ______|    |    |______ |  |  |\n"
            "#                                                                                                                                "
    );
}

void setup() {
    // init GIO
    Serial.begin(9600);


    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);

    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    // DEFAULT RELE OFF
    digitalWrite(RELAY_1, HIGH);
    digitalWrite(RELAY_2, HIGH);
    // DEFAULT LED OFF
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    wifi_init(); //CALL CONFIG WIFI
    if (!MYFS.begin()) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    init_message();
    server_init(server); // call init config server.
}

void loop() {
    // all set in setup, for loop is not necessary

}



