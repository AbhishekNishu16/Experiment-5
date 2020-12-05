//including all libraries
#include <WiFiNINA.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>
#include <ArduinoJson.h>
#include <SparkFunLSM6DS3.h>
#include "Wire.h"

//Connecting wifi
char ssid[] = "The name of your WIFI";
char pass[] = "Your WIFI password";

int status = WL_IDLE_STATUS;

char pubkey[] = "pub-c-8ae9b73c-60d6-46b3-b5d0-75230f65c3fa";
char subkey[] = "sub-c-005dbc58-2f12-11eb-9713-12bae088af96";

const char* myID = "Grace";

char publishChannel[] = "Grace";
char readChannel[] = "Nishu";

// JSON variables
StaticJsonDocument<200> dataToSend; // The JSON from the outgoing message
StaticJsonDocument<200> inMessage; // JSON object for receiving the incoming values
//create the names of the parameters you will use in your message
String JsonParamName1 = "publisher";
String JsonParamName2 = "daylight";

int serverCheckRate = 1000; 
unsigned long lastCheck;

LSM6DS3 myIMU(I2C_MODE, 0x6A);


//light sensor
int sensor1Pin=A1;
int sensor1Val;

//leds
int nishuledPins[]={11,12,10,9,8,7,6};
//int graceledPins[]={11,12,10,9};
int led1Val;
int brightness;
int ledTotal = sizeof(nishuledPins) / sizeof(int);

int nishuDaylight=0;
int graceDaylight=0;

const char* inMessagePublisher; 


void setup() {
 for(int i=0;i<ledTotal;i++){
 pinMode(nishuledPins[i],OUTPUT);
 //pinMode(graceledPins[i],OUTPUT);
 Serial.begin(9600);
}

connectToPubNub();
//myIMU.begin();
}

void loop() {

graceDaylight=sensor1Val;
sendReceiveMessages(serverCheckRate);

nightNishu(nishuDaylight);
//nightGrace(graceDaylight);
}

void connectToPubNub()
{
    // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to the network, SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    Serial.print("*");

    // wait 2 seconds for connection:
    delay(2000);
  }

  // once you are connected :
  Serial.println();
  Serial.print("You're connected to ");
  Serial.println(ssid);
  
  PubNub.begin(pubkey, subkey);
  Serial.println("Connected to PubNub Server");
  
}


void sendReceiveMessages(int pollingRate)
{
    //connect, publish new messages, and check for incoming
    if((millis()-lastCheck)>=pollingRate)
    {
      //publish data
      sendMessage(publishChannel); // publish this value to PubNub

      //check for new incoming messages
      readMessage(readChannel);
      
      //save the time so timer works
      lastCheck=millis();
    }  
}


void sendMessage(char channel[]) 
{

      Serial.print("Sending Message to ");
      Serial.print(channel);
      Serial.println(" Channel");
  
  char msg[64]; 
  
  dataToSend[JsonParamName1] = myID; 
  dataToSend[JsonParamName2] = graceDaylight;

  serializeJson(dataToSend, msg);
  Serial.println(msg);
  
  WiFiClient* client = PubNub.publish(channel, msg); 
  if (!client) 
  {
    Serial.println("publishing error"); 
  }
  else
  {
  Serial.print("   ***SUCCESS"); 
  }

}

void readMessage(char channel[]) 
{
  String msg;
    auto inputClient = PubNub.history(channel,1);
    if (!inputClient) 
    {
        Serial.println("message error");
        delay(1000);
        return;
    }
    HistoryCracker getMessage(inputClient);
    while (!getMessage.finished()) 
    {
        getMessage.get(msg);
        //basic error check to make sure the message has content
        if (msg.length() > 0) 
        {
          Serial.print("**Received Message on ");
          Serial.print(channel);
          Serial.println(" Channel");
          Serial.println(msg);
          //parse the incoming text into the JSON object

          deserializeJson(inMessage, msg); // parse the  JSON value received

           inMessagePublisher = inMessage[JsonParamName1]; 
           nishuDaylight = inMessage[JsonParamName2]; 

        
        }
    }
    inputClient->stop();
  
}


void nightNishu(int inputValue)
{ 
int minDaylight = 0;
int maxDaylight = 255;

int minSensorVal = 0;
int maxSensorVal = 1023;

sensor1Val=analogRead(sensor1Pin);

int brightness = map(inputValue,minSensorVal,maxSensorVal,maxDaylight,minDaylight);

for(int i=0;i<ledTotal;i++)
  {
//brightness=map(sensor1Val,1000,0,0,255);
analogWrite(nishuledPins[i],brightness);
}

//Serial.print("nishu's sensor value: ");
//Serial.println(sensor1Val);

}

//void nightGrace(int inputValue)
//{ 
//int minDaylight = 0;
//int maxDaylight = 255;
//
//int minSensorVal = 0;
//int maxSensorVal = 300;
//
//sensor1Val=analogRead(sensor1Pin);
//
//int brightness = map(inputValue,minSensorVal,maxSensorVal,maxDaylight,minDaylight);
//
//for(int i=0;i<ledTotal;i++)
//  {
////brightness=map(sensor1Val,1000,0,0,255);
//analogWrite(graceledPins[i],brightness);
//}
//
////Serial.print("grace's sensor value:                  ");
////Serial.println(sensor1Val);
//
//}
