#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

// Replace with your network credentials
const char* wifi_ssid     = "your-wifi-ssid";
const char* wifi_password = "your-wifi-password";
const char* wifi_hostname = "nodemcu-whr930";
const char* ota_password  = "your-ota-password";

const char* mqtt_server   = "your-mqtt-server-ip";
const int   mqtt_port     = 1883;
const char* mqtt_username = "your-mqtt-username";
const char* mqtt_password = "your-mqtt-password";

const char* mqtt_topic_base = "house/ventilation/whr930";
const char* mqtt_logtopic = "house/ventilation/whr930/log";

const char* mqtt_set_ventilation_topic = "house/ventilation/whr930/setventilation";
const char* mqtt_set_temperature_topic = "house/ventilation/whr930/settemperature";
const char* mqtt_get_update_topic = "house/ventilation/whr930/update";

//useful for debugging, outputs info to a separate mqtt topic
const bool outputMqttLog = true;

// instead of passing array pointers between functions we just define this in the global scope
#define MAXDATASIZE 256
char data[MAXDATASIZE];
int data_length = 0;

// log message to sprintf to
char log_msg[256];

// mqtt topic to sprintf and then publish to
char mqtt_topic[256];

// mqtt
WiFiClient mqtt_wifi_client;
PubSubClient mqtt_client(mqtt_wifi_client);

void send_command(byte* command, int length)
{  
  log_message("sending command");
  sprintf(log_msg, "Data length   : %d", length); log_message(log_msg);
  sprintf(log_msg, "Ack           : %02X %02X", command[0], command[1]); log_message(log_msg);
  sprintf(log_msg, "Start         : %02X %02X", command[2], command[3]); log_message(log_msg);
  sprintf(log_msg, "Command       : %02X %02X", command[4], command[5]); log_message(log_msg);
  sprintf(log_msg, "Nr data bytes : %02X (integer %d)", command[6], command[6]); log_message(log_msg);

  int bytesSent = Serial.write(command, length);
  sprintf(log_msg, "sent bytes    : %d", bytesSent); log_message(log_msg);

  // wait until the serial buffer is filled with the replies
  delay(1000);

  // read the serial
  readSerial();
  
  sprintf(log_msg, "received size : %d", data_length); log_message(log_msg);
}

// Callback function that is called when a message has been pushed to one of your topics.
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  for (int i=0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  msg[length] = '\0';

  if (strcmp(topic, mqtt_set_ventilation_topic) == 0)
  {
    String ventilation_string(msg);
    int ventilation = ventilation_string.toInt() + 1;
    int checksum = (0 + 153 + 1 + ventilation + 173) % 256;

    sprintf(log_msg, "set ventilation to %d", ventilation - 1); log_message(log_msg);
    byte command[] = {0x07, 0xF0, 0x00, 0x99, 0x01, ventilation, checksum, 0x07, 0x0F};
    send_command(command, sizeof(command));
  }
  if (strcmp(topic, mqtt_set_temperature_topic) == 0)
  {
    String temperature_string(msg);
    int temperature = temperature_string.toInt();

    temperature = (temperature + 20) * 2;
    int checksum = (0 + 211 + 1 + temperature + 173) % 256;
    
    sprintf(log_msg, "set temperature to %d", temperature); log_message(log_msg);
    byte command[] = {0x07, 0xF0, 0x00, 0xD3, 0x01, temperature, checksum, 0x07, 0x0F};
    send_command(command, sizeof(command));
  }

  if (strcmp(topic, mqtt_get_update_topic) == 0)
  {    
    log_message("Updating..");

    update_everything();
  }
}

void update_everything()
{
  get_filter_status();
  get_temperatures();
  get_ventilation_status();
  get_fan_status();
  get_valve_status();
  get_bypass_control();
}

void get_filter_status() {  
    byte command[] = {0x07, 0xF0, 0x00, 0xD9, 0x00, 0x86, 0x07, 0x0F};
    send_command(command, sizeof(command));

    int filter_state = (int)(data[18]);
    
    char* filter_state_string;
    if (filter_state == 0) {
      filter_state_string = "Ok";
    } else if (filter_state == 1) {
      filter_state_string = "Full";
    } else {
      filter_state_string = "Unknown"; 
    }
    sprintf(log_msg, "received filter state : %d (%s)", filter_state, filter_state_string); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "filter_state"); mqtt_client.publish(mqtt_topic, filter_state_string);
}

void get_temperatures() {  
    byte command[] = {0x07, 0xF0, 0x00, 0xD1, 0x00, 0x7E, 0x07, 0x0F};
    send_command(command, sizeof(command));
    
    float ComfortTemp = (float)data[6] / 2.0 - 20;
    float OutsideAirTemp = (float)data[7] / 2.0 - 20;
    float SupplyAirTemp = (float)data[8] / 2.0 - 20;
    float ReturnAirTemp = (float)data[9] / 2.0 - 20;
    float ExhaustAirTemp = (float)data[10] / 2.0 - 20;
    
    sprintf(log_msg, "received temperatures (comfort, outside, supply, return, exhaust): %.2f, %.2f, %.2f, %.2f, %.2f", ComfortTemp, OutsideAirTemp, SupplyAirTemp, ReturnAirTemp, ExhaustAirTemp); log_message(log_msg);

    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "comfort_temp"); mqtt_client.publish(mqtt_topic, String(ComfortTemp).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "outside_air_temp"); mqtt_client.publish(mqtt_topic, String(OutsideAirTemp).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "supply_air_temp"); mqtt_client.publish(mqtt_topic, String(SupplyAirTemp).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "return_air_temp"); mqtt_client.publish(mqtt_topic, String(ReturnAirTemp).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "exhaust_air_temp"); mqtt_client.publish(mqtt_topic, String(ExhaustAirTemp).c_str());
}

void get_ventilation_status() {  
    byte command[] = {0x07, 0xF0, 0x00, 0xCD, 0x00, 0x7A, 0x07, 0x0F};
    send_command(command, sizeof(command));

    
    float ReturnAirLevel = (float)data[12];
    float SupplyAirLevel = (float)data[13];
    int FanLevel = (int)data[14] - 1;
    int IntakeFanActive = (int)data[15];
     
    sprintf(log_msg, "received ventilation status (return air level, supply air level, fan level, intake fan active): %.2f, %.2f, %d, %d", ReturnAirLevel, SupplyAirLevel, FanLevel, IntakeFanActive); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "return_air_level"); mqtt_client.publish(mqtt_topic, String(ReturnAirLevel).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "supply_air_level"); mqtt_client.publish(mqtt_topic, String(SupplyAirLevel).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "ventilation_level"); mqtt_client.publish(mqtt_topic, String(FanLevel).c_str());

    char* IntakeFanActive_string;
    if (IntakeFanActive == 1) {
      IntakeFanActive_string = "Yes";
    } else if (IntakeFanActive == 0) {
      IntakeFanActive_string = "No";
    } else {
      IntakeFanActive_string = "Unknown"; 
    }
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "intake_fan_active");  mqtt_client.publish(mqtt_topic, IntakeFanActive_string);
}

void get_fan_status() {  
    byte command[] = {0x07, 0xF0, 0x00, 0x0B, 0x00, 0xB8, 0x07, 0x0F};
    send_command(command, sizeof(command));

    float IntakeFanSpeed = (int)data[6];
    float ExhaustFanSpeed = (int)data[7];

    sprintf(log_msg, "received fan speeds (intake, exhaust): %.2f, %.2f", IntakeFanSpeed, ExhaustFanSpeed); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "intake_fan_speed"); mqtt_client.publish(mqtt_topic, String(IntakeFanSpeed).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "exhaust_fan_speed"); mqtt_client.publish(mqtt_topic, String(ExhaustFanSpeed).c_str());
}

void get_valve_status() {  
    byte command[] = {0x07, 0xF0, 0x00, 0x0D, 0x00, 0xBA, 0x07, 0x0F};
    send_command(command, sizeof(command));

    int ByPass = (int)data[7];
    int PreHeating = (int)data[8];
    int ByPassMotorCurrent = (int)data[9];
    int PreHeatingMotorCurrent = (int)data[10];
 
    sprintf(log_msg, "received fan status (bypass, preheating, bypassmotorcurrent, preheatingmotorcurrent): %d, %d, %d, %d", ByPass, PreHeating, ByPassMotorCurrent, PreHeatingMotorCurrent); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "valve_bypass_percentage"); mqtt_client.publish(mqtt_topic, String(ByPass).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "valve_preheating"); mqtt_client.publish(mqtt_topic, String(PreHeating).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "bypass_motor_current"); mqtt_client.publish(mqtt_topic, String(ByPassMotorCurrent).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "preheating_motor_current"); mqtt_client.publish(mqtt_topic, String(PreHeatingMotorCurrent).c_str());
}

void get_bypass_control() {
    byte command[] = {0x07, 0xF0, 0x00, 0xDF, 0x00, 0x8C, 0x07, 0x0F};
    send_command(command, sizeof(command));

    int ByPassFactor  = (int)data[9];
    int ByPassStep  = (int)data[10];
    int ByPassCorrection  = (int)data[11];

    char* summerModeString;
    if ((int)data[13] == 1) {
      summerModeString = "Yes";
    } else {
      summerModeString = "No";
    }
    sprintf(log_msg, "received bypass control (bypassfactor, bypass step, bypass correction, summer mode): %d, %d, %d, %s", ByPassFactor, ByPassStep, ByPassCorrection, summerModeString); log_message(log_msg);
    
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "bypass_factor"); mqtt_client.publish(mqtt_topic, String(ByPassFactor).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "bypass_step"); mqtt_client.publish(mqtt_topic, String(ByPassStep).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "bypass_correction"); mqtt_client.publish(mqtt_topic, String(ByPassCorrection).c_str());
    sprintf(mqtt_topic, "%s/%s", mqtt_topic_base, "summermode"); mqtt_client.publish(mqtt_topic, String(summerModeString).c_str());
}
void setup() {  
  Serial.begin(9600);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.hostname(wifi_hostname);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(wifi_hostname);

  // Set authentication
  ArduinoOTA.setPassword(ota_password);

  ArduinoOTA.onStart([]() {
  });
  ArduinoOTA.onEnd([]() {
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

  });
  ArduinoOTA.onError([](ota_error_t error) {

  });
  ArduinoOTA.begin();

  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(mqtt_callback);
}

void loop() {
  // Handle OTA first.
  ArduinoOTA.handle();

  if (!mqtt_client.connected())
  {
    mqtt_reconnect();
  }
  mqtt_client.loop();

  update_everything();
  delay(5000);
}

void mqtt_reconnect()
{
  // Loop until we're reconnected
  while (!mqtt_client.connected()) 
  {
    if (mqtt_client.connect(wifi_hostname, mqtt_username, mqtt_password))
    {
      mqtt_client.subscribe(mqtt_set_ventilation_topic);
      mqtt_client.subscribe(mqtt_set_temperature_topic);
      mqtt_client.subscribe(mqtt_get_update_topic);
    }
    else
    {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void log_message(char* string)
{
  if (outputMqttLog)
  {
    mqtt_client.publish(mqtt_logtopic, string); 
  } 
}

void readSerial()
{

  if (Serial.available() > 0) {
    log_message("serial available");
    
    int index = 0;
    while (Serial.available()){
      byte iByte = Serial.read();
        
      sprintf(log_msg, "%02X", iByte); log_message(log_msg);

      // reset the internal counter to zero when we encounter a start of a message
      if (index > 0 && iByte == 0xF0 && data[index-1] == 0x07){
          log_message("start of msg");
          index = 0;
      } else {
          index += 1;
      }
  
      data[index] = iByte;
      
      if (iByte == 0xF3 && data[index-1] == 0x07)
      {
        log_message("Got an ACK!");
        index = 0;
        data_length = 0;
      }
      if (iByte == 0x0F && data[index-1] == 0x07)
      {
          // call process function
          sprintf(log_msg, "end of msg of length %d", index+1); log_message(log_msg);
          data_length = index + 1;          
      }
    }
  }
}
