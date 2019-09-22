#include <SPI.h>
#include<WiFi.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include "portalhtml.h"
#include <string.h>
#include <stdlib.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <HTTPClient.h>
#include <pgmspace.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#define TOPIC_NAME "thing1"
#define IOT_TOPIC "$aws/things/" TOPIC_NAME "/shadow/update"
#define MAX_MESSAGE_LEN 31
#define WIFI_MESSAGE 256
#define CHANNEL 102
#define ARDUINO_RUNNING_CORE 0

//************** Auxillary functions******************//
WebServer server(80);
RF24 radio(25,26); // CE, CSN
StaticJsonDocument<200> doc;
HTTPClient http;
AWS_IOT awsClient; //Instance of AWS_IOT class

//handle for task
TaskHandle_t xHandle;

int gatewayConfig = 0;

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

//Interrupt Pin
volatile int intPin = 27;

//ledPin
int ledpin = 5;

//TX and RX address
const byte rxAddr[6] = "00002"; 
const byte txAddr[6] = "00001";

//WiFi Connection attempts and counter
int counter =0;

//Gateway name
String gate = "GateWay1";

//*********SSID and Pass for AP**************//
const char* ssidAPConfig = "adminesp";
const char* passAPConfig = "adminesp";

//*********AWS Credentials*************//
char HOST_ADDRESS[]="a15adm0vrco7b4-ats.iot.us-west-2.amazonaws.com";
char CLIENT_ID[]= "thing1Policy";


//**********softAPconfig Timer*************//
unsigned long APTimer = 0;
unsigned long APInterval = 90000;

//Sensor parameters
char rxBuff[MAX_MESSAGE_LEN] ="";
char ackbuff[MAX_MESSAGE_LEN] ="";
char ackPayload[] = "Success";
const char MESSAGE_BODY[] = "{\"state\":{\"desired\":{\"gate\":\"%s\",\"device\":%s,\"temp\":%d}}}";
char messagePayload[WIFI_MESSAGE];
//char ack[MAX_MESSAGE_LEN] ="";
bool isAPConnected = true;

//Subscribe Payload
char rcvdPayload[WIFI_MESSAGE];


void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
    strncpy(rcvdPayload,payLoad,payloadLen);
    Serial.println(rcvdPayload);    
}

void IRAM_ATTR ISR_int(){
     APTimer = millis();
     detachInterrupt(digitalPinToInterrupt(intPin));
     vTaskSuspend( xHandle );
     gatewayConfig=1;
  }

void setup() {
  Serial.begin(115200);
  Serial.print("Started");
  pinMode(intPin,INPUT);
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin,LOW);
  //attachInterrupt(digitalPinToInterrupt(intPin), ConfigInt, FALLING);
  EEPROM.begin(512);
  radio.begin();
  radio.setChannel(CHANNEL);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();        
  Serial.println(radio.isChipConnected() ? "connected" : "not connected");
  radio.setPALevel(RF24_PA_MIN);
  radio.openWritingPipe(txAddr);        // Both radios listen on the same pipes by default, and switch when writing
  radio.openReadingPipe(1,rxAddr);
  radio.startListening(); 
  reconnectWiFi();
  //delay(5000);
  attachInterrupt(digitalPinToInterrupt(intPin),ISR_int,CHANGE);
  xTaskCreatePinnedToCore(
    TaskSend
    ,  "TaskSendNRF"   // A name just for humans
    ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &xHandle 
    ,  0);
  //reconnectWiFi();
}
void loop() {

    if(gatewayConfig){
          Serial.println("interruptGenerated");
          handleWebform();
          vTaskResume(xHandle);
      }
      gatewayConfig=0;
  /*if(startConfig==1){
      Serial.println("interruptGenerated");
      handleWebform();
    }*/
    
}


void TaskSend(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
   
  for (;;) // A Task shall never return or exit.
  { 
          
   if ( radio.available()) {
    if(rxBuff!=""){
         strcpy(rxBuff,"");
         Serial.println("buffercleared");
      }
        radio.read(&rxBuff, sizeof(rxBuff));
        //Serial.println(sizeof(buff));
        Serial.println(rxBuff);
        Serial.println(sizeof(rxBuff));
        DeserializationError error = deserializeJson(doc,rxBuff);
     if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
     } 
        const char* deviceId= doc["d"];
        int sensor = doc["s"];
        String devCat = "DEVICE0" + String(deviceId);
        Serial.printf("sensor = %d\n",sensor);
        Serial.println(devCat);
        snprintf(messagePayload,WIFI_MESSAGE,MESSAGE_BODY,gate.c_str(),devCat.c_str(),sensor);
        Serial.println(messagePayload);
        if(sizeof(messagePayload)>0){
           if(WiFi.status()==WL_CONNECTED){
               int code = hornbill.publish(IOT_TOPIC,messagePayload);
               Serial.printf("message published %d",counter++);
        Serial.println(code); 
        switch(code){
              case 0:
                 Serial.println("SUCCESS");
                 Serial.println(messagePayload);
                 digitalWrite(ledpin,HIGH);
                 vTaskDelay(500);
                 digitalWrite(ledpin,LOW);
                 break;
              case -26:
                 Serial.println("MQTT_CONNECTION_ERROR");
                 reconnectMQTT();
                 break;
              case -27:
                 Serial.println("MQTT_CONNECT_TIMEOUT_ERROR");
                 reconnectMQTT();
                 break;
              case -28:
                 Serial.println("MQTT_REQUEST_TIMEOUT_ERROR");
                 reconnectMQTT();
                 break;
              case -36:
                 Serial.println("MQTT_CONNACK_UNKNOWN_ERROR");
                 break;
              case -43:
                  Serial.println("SHADOW_WAIT_FOR_PUBLISH");
                  break;
              case -6:
                 Serial.println("NETWORK_SSL_CONNECT_TIMEOUT_ERROR");
                 reconnectMQTT();
                 break;
              case -1:
                  Serial.println("FALIURE");
                  reconnectMQTT();
                  break;     
              case -3:
                  Serial.println("TCP_CONNECTION_ERROR");
                  reconnectMQTT();
                  break;       
    /** The TLS handshake failed */
              case -4:
                  Serial.println("SSL_CONNECTION_ERROR");
                  reconnectMQTT();
                  break;
  /** Error associated with setting up the parameters of a Socket */
              case -5:
                  Serial.println("TCP_SETUP_ERROR");
                  reconnectMQTT();
                  break;
  /** A timeout occurred while waiting for the TLS handshake to complete. */
              case -13:
                  Serial.println("NETWORK_DISCONNECTED_ERROR");
                  hornbill.connect(HOST_ADDRESS,CLIENT_ID);
                  break;
              default:
                  Serial.println("NETWORK_ERROR");
                  //hornbill.connect(HOST_ADDRESS,CLIENT_ID);    
          }
               }else{
                 reconnectWiFi();
                 reconnectMQTT()
              }
        }
    vTaskDelay(5);
  }
    vTaskDelay(1000);
  }
  
}


//****************************Connect to WiFi****************************//
void reconnectWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
        String string_Ssid="";
        String string_Password="";
        string_Ssid= read_string(22,0); 
        string_Password= read_string(52,22);        
        Serial.println("ssid: "+ string_Ssid);
        Serial.println("Password: "+string_Password);               
  delay(400);
  WiFi.begin(string_Ssid.c_str(),string_Password.c_str());
  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {   
      delay(500);
      Serial.print(".");
      if(count == 40){
          ESP.restart();
        }
        count++;
  }
  Serial.print("Connected to:\t");
  Serial.println(WiFi.localIP());
}

//****************************handleWebForm****************************//
void handleWebform(){ 
      WiFi.mode(WIFI_AP);
      delay(100);
      Serial.println(WiFi.softAP(ssidAPConfig,passAPConfig) ? "Configuring softAP" : "kya yaar not connected");
      WiFi.softAPConfig(apIP, apIP, netMsk);    
      delay(100);
      Serial.println(WiFi.softAPIP());
      server.begin();
      server.on("/", handle1);
      server.on("/node", handle2);
      server.on("/nodeConfig", handle3); 
      server.onNotFound(handleNotFound);
      while(millis()-APTimer<= APInterval) {
       server.handleClient();  
        }
       Serial.println("Server disconnected");
       Serial.println(WiFi.disconnect()? "disconnected":"not Disconnected");
       delay(100);  
       //reconnectWiFi();
       ESP.restart();
      }

//****************************Set SSID and Passkey****************************//
void handle1(){
   Serial.println("handle 1 begin");
  if(server.args()>0){
       if(server.hasArg("ssid")&&server.hasArg("passkey")&&server.hasArg("sensor_select")&&server.hasArg("sensor_list")){
          /*for (int i = 0 ; i < EEPROM.length() ; i++) {
            EEPROM.write(i, 0);
          }*/
          Serial.println("handle 1 over");
          ROMwrite(String(server.arg("ssid")),String(server.arg("passkey")),String(server.arg("sensor_select")),String(server.arg("sensor_list")));
          handle2();
        }
        else if(server.hasArg("node")){       
            int pos = 82;
            Serial.println(server.args());
            for(int i=0; i<=server.args();i++){
            Serial.println(String(server.argName(i))+'\t' + String(server.arg(i)));
            ROMwriteNode(String(server.arg(i)),pos);
            pos+=6;
        } 
            isAPConnected =false;  
         }    
    }else{
          int n = WiFi.scanNetworks();
          int indices[n];     
          if(n == 0){
              Serial.println("No networks found");
          }else{   
              for (int i = 0; i < n; i++) {
              indices[i] = i;
          }
         String webString = FPSTR(HTTPHEAD);
         webString+= FPSTR(HTTPBODYSTYLE);
         webString+= FPSTR(HTTPBODY);
         webString+= FPSTR(HTTPCONTENTSTYLE);
         webString+= FPSTR(HTTPDEVICE);
         webString.replace("{s}",gate);
         webString+= FPSTR(HTTPFORM);
         for(int i=0;i<n;i++){
           int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
           String item = FPSTR(HTTP_CONTENT1_START);
           String RssiQuality = String(quality);
           item.replace("{v}",WiFi.SSID(indices[i]));
           item.replace("{r}",RssiQuality); 
           webString+=item;
         }
         webString+= FPSTR(HTTPLABLE1);
         webString+= FPSTR(HTTPLABLE2);
         webString+= FPSTR(HTTPLABLE3);
         webString+= FPSTR(HTTPNODESEL);
         webString+= FPSTR(HHTTPDELAY);
         webString+= FPSTR(HTTPSUBMIT);
         webString+= FPSTR(HTTPCLOSEFORM);
         webString+= FPSTR(HTTPSCRIPT);
         webString+= FPSTR(HTTPCLOSE);
         //File file = SPIFFS.open("/AgNextCaptive.html", "r");
         //server.streamFile(file,"text/html");
         server.send(200,"text/html",webString);
         //file.close();
         Serial.println(millis()-APTimer);
         APTimer= millis();
      }
  }
}


//****************HANDLE NOT FOUND*********************//
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  server.send(404, "text/plain", message);
}
  

//----------Write to ROM-----------//
void ROMwrite(String s, String p, String id,String delays){
 s+=";";
 write_EEPROM(s,0);
 p+=";";
 write_EEPROM(p,22);
 id+=";";
 write_EEPROM(id,52);
 delays+=";";
 write_EEPROM(delays,62);
 EEPROM.commit();   
}

void ROMwriteNode(String node,int pos){
    node+=";";
    write_EEPROM(node,pos);
    EEPROM.commit();
  }


//***********Write to ROM**********//
void write_EEPROM(String x,int pos){
  for(int n=pos;n<x.length()+pos;n++){
  //write the ssid and password fetched from webpage to EEPROM
   EEPROM.write(n,x[n-pos]);
  }
}
  

//****************************EEPROM Read****************************//
String read_string(int l, int p){
  String temp;
  for (int n = p; n < l+p; ++n)
    {
   // read the saved password from EEPROM
     if(char(EEPROM.read(n))!=';'){
     
       temp += String(char(EEPROM.read(n)));
     }else n=l+p;
    }
  return temp;
}


int getRSSIasQuality(int RSSI) {
  int quality = 0;
  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}


//**************************MQTT Connect******************************//
void reconnectMQTT(){
  Serial.println("Maintain Connection");
  if(WiFi.status()==WL_CONNECTED){
    int connError = hornbill.connect(HOST_ADDRESS,CLIENT_ID);
    Serial.println(connError);
    if(connError== 0)
    {
        Serial.println("Connected to AWS");
        delay(1000);

        if(0==hornbill.subscribe(TOPIC_NAME,mySubCallBackHandler))
        {
            Serial.println("Subscribe Successfull");
        }
        else
        {
            Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
            while(1);
        }
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }
  }else{
       reconnectWiFi(); 
    }
}
