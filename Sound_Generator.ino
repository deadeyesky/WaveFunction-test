/*
* This is the script for the 2020 science fair for analyzing
* the propogation of wave energy throughout a material by
* applying acoustic sound to it and then measuring the
* returning energy with a pair of microphones.
*/

#include "SD.h"
#include"SPI.h"

#define W_CLK 6       // Pin 6 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 5       // Pin 5 - connect to freq update pin (FQ)
#define DATA 4       // Pin 4 - connect to serial data load pin (DATA)
#define RESET 3      // Pin 3 - connect to reset pin (RST).

#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW);}

#define MIC_1 A0
#define MIC_2 A1

#define MAXIMUM_FREQUENCY 10000
#define MINIMUM_FREQUENCY 150

int sensor1 = 0;
int sensor2 = 0;
int maxval1 = 0;
int maxval2 = 0;
uint32_t lastSample = 0;

String input;  // Used for the serial input function
File raw_data;

void display_info () {
  // Set the serial monitor baudrate
  Serial.begin(9600);
  Serial.println("--Arduino Nano Frequency Generator");
  Serial.println("Signal Generator: AD9850-----------");
  Serial.println("Chip: AD9850 8RSZ 1432-------------");
  Serial.println("Clock Speed: 125,000,000 MHz (5V)--");
  Serial.println("Square Wave Mode?: No--------------");
  Serial.println("Sine Wave Mode?: Yes---------------");
  Serial.println("Word Load Clock Pin: 6 (PWM)-------");
  Serial.println("Frequency Update Pin: 5 (PWM)------");
  Serial.println("Data Pin: 4 (non-PWM)--------------");
  Serial.println("Reset Pin: 3 (PWM, but not used)---");
  delay(1500);
  Serial.println("");
  Serial.println("Frequency Range is from 0 to 10 Mhz.");
  Serial.println("The human ear can only hear noises from 80 Hz");
  Serial.println("to 20,000 Hz. Anything outside these cannot  ");
  Serial.println("be heard.");
  delay(1500);
}

// transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte (byte data) {
  for (int i = 0; i < 8; i++, data >>= 1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency (double frequency) {
  int32_t freq = frequency * 4294967295 / 125000000;  // Note 125 MHz clock on 9850
  for (int b = 0; b < 4; b++, freq >>= 8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Should see output
}

void setup () {
  // Run Setup function
  display_info();

  // Configure data pins for output
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);

  // Microphone pins get set to input
  pinMode(MIC_1, INPUT);
  pinMode(MIC_2, INPUT);

  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);  // This pulse enables serial mode - Datasheet page 12 figure 10

  while (!Serial) {}

  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }

  Serial.println("initialization done.");
}

void fullRangeTest () {
  if(Serial.available()) {
    Serial.println("");
    raw_data = SD.open("data.txt", FILE_WRITE);
    raw_data.println("frequency\tsensor1\tsensor2");

    for (float i = MAXIMUM_FREQUENCY; i >= MINIMUM_FREQUENCY; i--) {
      sendFrequency(i);
      for (int j = 1000; j >= 0; j--) {
        sensor1 = analogRead(MIC_1);
        sensor2 = analogRead(MIC_2);
        delay(10);

        if (sensor1 > maxval1) {
          maxval1 = sensor1;
        }

        else if (sensor2 > maxval2) {
          maxval2 = sensor2;
        }
      }
      raw_data.println(i + "\t" + maxval1.toString() + "\t" + maxval2.toString());
      maxval1 = 0; maxval2 = 0;
    }
    raw_data.close();
    Serial.println("Finished");
  }
}

void loop () {
  if(Serial.available()) {
    input = Serial.readStringUntil('\n');
    Serial.println("Frequency set to " + input + "Hz");

    if (input == "test") {
      fullRangeTest();
    }
    // Frequency number gets applied to the AD9850 from the Nano
    sendFrequency(input.toFloat());
  }
}
