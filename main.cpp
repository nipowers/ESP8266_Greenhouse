// Adafruit IO Publish Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

// Modified by Nicholas Powers for Arrow Electronics to function as a simple
// automated green house. Still needs a lot of polish but it functions.

// Use with Adafruit config.h to setup wifi and Adafruit.IO connection
// https://github.com/adafruit/Adafruit_IO_Arduino/blob/master/examples/adafruitio_00_publish/config.h

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include "DHT.h"
#include <Arduino.h>
#include <Servo.h>

/************************ Example Starts Here *******************************/

void handleMessage_fan(AdafruitIO_Data *data);
void handleMessage_hatch(AdafruitIO_Data *data);

//Setup DHT temp and humidity sensor on pin 2 and as type DHT22
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Create servo object
Servo myservo;

//Set transistor control pin to pin 15
int transistorPin = 15;

// this int will hold the current count for our sketch
int count = 0;
bool open = false;
int pos = 10;

bool fan_toggle;
// set up the 'counter' feed
AdafruitIO_Feed *counter = io.feed("counter");
// set up 'temperature' feed
AdafruitIO_Feed *temp = io.feed("temperature");
// set up 'humidity' feed
AdafruitIO_Feed *humid = io.feed("humidity");
// set up 'light_level' feed
AdafruitIO_Feed *light_level = io.feed("light_level");

// input feeds for control
AdafruitIO_Feed *fan = io.feed("fan");
AdafruitIO_Feed *hatch = io.feed("hatch");

void setup() {
  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  //while(! Serial);

  Serial.println("Connecting to Adafruit IO");
  Serial.println("This is a test of the DHT22 with servo and fan");

  //Attach servo to GPIO 16
  myservo.attach(16);

  // connect to io.adafruit.com
  io.connect();
  //Start DHT process
  dht.begin();
  //Setup analog input
  pinMode(A0, INPUT);
  pinMode(transistorPin, OUTPUT);

  myservo.write(pos);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());


  //setup input handlers
  fan->onMessage(handleMessage_fan);
  hatch->onMessage(handleMessage_hatch);
}

void handleMessage_fan(AdafruitIO_Data *data) {

  Serial.print("received <- ");

  fan_toggle = data->value();
  Serial.print("Fan Toggle: ");
  Serial.println(fan_toggle);
  // write the current state to the led


}

void handleMessage_hatch(AdafruitIO_Data *data) {

  Serial.print("received <- ");

  if(data->toPinLevel() == HIGH)
    Serial.println("HIGH");
  else
    Serial.println("LOW");
}

void loop() {
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  // save count to the 'counter' feed on Adafruit IO
  Serial.print("sending -> ");
  Serial.println(count);
  counter->save(count);

  // increment the count by 1
  count++;


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  //Read analog input
  int light = analogRead(A0);
  int light_adjust = light/10;

  Serial.println("........................................");
  Serial.print("Humidity:    ");
  Serial.print(h);
  Serial.println(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.println(" *F\t");
  Serial.print("Heat index:  ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F ");
  Serial.print("Light Level: ");
  Serial.println(light);
  Serial.print("Hatch Open?  ");
  Serial.println(open);

  temp->save(f);
  humid->save(h);
  light_level->save(light_adjust);

  //Open hatch and turn on fan if temp or humidity pass threshold
  if((f>80 and !open))// or (!open and fan_toggle))
  {
    Serial.println("Going from 10 to 100");

    for(pos = 10; pos <= 100; pos += 1) // goes from 0 degrees to 180 degrees
    {                                  // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }

    open = true;                       // set open to true to indicate hatch is open

    Serial.println("Turning on fan");
    digitalWrite(transistorPin, HIGH);
  }

  //Close hatch and turn off fan if temp or humidity below threshold
  if((f<77 and open))// or (open and !fan_toggle))
  {
    Serial.println("Going from 10 to 100");

    for(pos = 100; pos >= 10; pos -= 1) // goes from 0 degrees to 180 degrees
    {                                  // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }

    open = false;                       // set open to true to indicate hatch is open

    Serial.println("Turning off fan");
    digitalWrite(transistorPin, LOW);
  }



  // wait five seconds (1000 milliseconds == 1 second)
  Serial.println();
  delay(5000);
}
