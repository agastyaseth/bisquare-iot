# aws-mqtt-websockets
Implementation of a middleware to use AWS MQTT service through websockets. Aiming the esp8266 platform

## ChangeLog

* 1.0.1 - works with arduinoWebSockets v.2.0.5 and arduino/esp sdk 2.3.0
* 1.0.alpha - stable - works with arduinoWebSockets v.2.0.2 and arduino/esp sdk 2.1.0
* 0.3 - own impl of circular buffer
* 0.2 - auto reconnection
* 0.1 - has known limitation and it was not extensively tested

## Motivation

As we cannot use AWS MQTT service directly because of the lack of support for TLS 1.2, we need to use the websocket communication as a transport layer for MQTT through SSL (supported by esp8266)

This way we can change the state of your esp8266 devices in realtime, without using the AWS Restful API and busy-waiting inefficient approach.

## Dependencies

| Library                   | Link                                                            | Use                 |
|---------------------------|-----------------------------------------------------------------|---------------------|
|aws-sdk-arduino*            |https://github.com/svdgraaf/aws-sdk-arduino                      |aws signing functions|
|arduinoWebSockets          |https://github.com/Links2004/arduinoWebSockets                   |websocket comm impl  |
|Paho MQTT for Arduino      |https://projects.eclipse.org/projects/technology.paho/downloads  |mqtt comm impl       |

\* copy files from src/common folder to the library root folder and then copy the files from src/esp8266 folder in the library root folder (overwritting the ones from common folder) - details in the aws-sdk-arduino library's github page 

## Installation

1. Configure your arduino ide to compile and upload programs to ESP8266 (Arduino core for ESP8266 - details https://github.com/esp8266/Arduino )\*\*
2. Install all the dependencies as Arduino Libraries
3. Install aws-mqtt-websockets as Arduino Library as well
4. Configure the example file with your AWS credencials and endpoints
5. Compile, upload and run!

\** The library was tested with 2.3.0 stable version of Arduino core for ESP8266

## Limitation

* need to be stress tested
* may work in Arduino out of AWS environment (turn off SSL and set Path) 

## Usage

It is transparent. It is the same as the usage of Paho. There is just some changes in the connection step. See the example for details. Things you should edit in the example:
* ssid and password to connect to wifi
* domain/endpoint for your aws iot service
* region of your aws iot service
* aws user key \*\*
* aws user secret key

 \*\* It is a good practice creating a new user (and grant just iot services permission). Avoid use the key/secret key of your main aws console user
 
 ```
 //AWS IOT config, change these:
char wifi_ssid[]       = "your-ssid";
char wifi_password[]   = "your-password";
char aws_endpoint[]    = "your-endpoint.iot.eu-west-1.amazonaws.com";
char aws_key[]         = "your-iam-key";
char aws_secret[]      = "your-iam-secret-key";
char aws_region[]      = "eu-west-1";
const char* aws_topic  = "$aws/things/your-device/shadow/update";
int port = 443;

//MQTT config
const int maxMQTTpackageSize = 128;
const int maxMQTTMessageHandlers = 1;
 ```
