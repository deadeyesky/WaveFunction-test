/*
* This is the script for the 2020 science fair for analyzing
* the propogation of wave energy throughout a material by
* applying acoustic sound to it and then measuring the
* returning energy with a pair of microphones.
*/

// Import libraries
#include "SD.h"
#include "SPI.h"

// Define signal generator pins
#define W_CLK 6       // Pin 6 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 5       // Pin 5 - connect to freq update pin (FQ)
#define DATA 4        // Pin 4 - connect to serial data load pin (DATA)
#define RESET 3       // Pin 3 - connect to reset pin (RST).

// Set default pin values for the generator pins
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW);}

#define MAXIMUM_FREQUENCY 5000
#define MINIMUM_FREQUENCY 0
#define SAMPLES 1000

#define FASTADC 1

// Defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define CHIP_SELECT 13

String command;  // Used for the serial input function
String sensor_read; // Used as an object to append the sensor readings
String read_string;

char filepath1[] = "spectral_test.csv";

void display_info () {
  // Set the serial monitor baudrate
  Serial.begin(38400);
  Serial.println(F("-Arduino Nano Frequency Generator-"));
  Serial.println(F("Signal Generator: AD9850-----------"));
  Serial.println(F("Chip: AD9850 8RSZ 1432-------------"));
  Serial.println(F("Clock Speed: 125,000,000 MHz (5V)--"));
  Serial.println(F("Square Wave Mode?: No--------------"));
  Serial.println(F("Sine Wave Mode?: Yes---------------"));
  Serial.println(F("Word Load Clock Pin: 6 (PWM)-------"));
  Serial.println(F("Frequency Update Pin: 5 (PWM)------"));
  Serial.println(F("Data Pin: 4 (non-PWM)--------------"));
  Serial.println(F("Reset Pin: 3 (PWM, but not used)---"));
  Serial.println(F("Analog Measurement prescale: 64----"));
  Serial.println(F("SD card Chip Select Pin: 13--------\n"));
  delay(1500);
  // Wait until the SD card initializes
  while (!Serial) {}
  Serial.print(F("Initializing SD card... "));

  // If the SD card doesn't start up:
  if (!SD.begin(CHIP_SELECT)) {
    //SD.initErrorHalt();
    Serial.println(F("initialization failed!"));
    Serial.println(F("Either there is no SD card, or it's corrupted, or isn't connected."));
    //while (1);
  }

  Serial.println(F("initialization done.\n"));
  delay(1000);
  Serial.println(F("Frequency Range is from 0 to 10 Mhz."));
  Serial.println(F("The human ear can only hear noises from 80 Hz"));
  Serial.println(F("to 20,000 Hz. Anything outside these cannot  "));
  Serial.println(F("be heard.\n"));
  delay(1000);
  Serial.println(F("-------------Commands-------------"));
  Serial.println(F("full:      runs full spectral test"));
  Serial.println(F("test <in>: tests defined frequency"));
  Serial.println(F("runtest:   outputs frequency range"));
  Serial.println(F("off:  shuts off the speaker output"));
  Serial.println(F("<int>:   outputs entered frequency\n"));
  delay(1500);
}

// transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte (byte data) {
  for (byte i = 0; i < 8; i++, data >>= 1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency (double frequency) {
  int32_t freq = frequency * 4294967295 / 125000000;  // Note 125 MHz clock on 9850
  for (byte b = 0; b < 4; b++, freq >>= 8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Should see output
}

void setup () {
  // Run Setup function
  sendFrequency(0);
  display_info();

  // Configure data pins for output
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);

  // Port manipulation alternative:
  //DDRC = B01111000

  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);  // This pulse enables serial mode - Datasheet page 12 figure 10

  #if FASTADC
    // Set prescale to 64
    sbi(ADCSRA, ADPS2) ;
    sbi(ADCSRA, ADPS1) ;
    cbi(ADCSRA, ADPS0) ;
  #endif
}

void loop () {
  // Initialize the serial read
  if (Serial.available()) {
    command = Serial.readStringUntil('\n');
    // The following command creates file that appends a full spectrum of frequncies automatically
    if (command.equals("full")) {
      Serial.println("Starting full spectral test.");

      // Start the frequency read from high to low frequency
      for (int j = MINIMUM_FREQUENCY; j < MAXIMUM_FREQUENCY; j+=10) {
        // Initializes the file with the name as the specific frequency being tested
        File dataFile = SD.open((String(j) + ".csv").c_str(), FILE_WRITE);
        // Sets the frequency value of the AD9850
        sendFrequency(float(j));
        delay(10);

        if (dataFile) {
          Serial.print("Sampling...");
          // This for loop obtains a specified number of samples
          for (int i = SAMPLES; i >= 0; i--) {
            dataFile.println(String(analogRead(A0)) + "," + String(analogRead(A1)));  // Inserts the values in the file with a tab delimiter
          }
          dataFile.close();                             // Closes the file after the writing is done
          Serial.print(" Finished reading " + String(j));// Debugger for signifying the completion of the file write
          Serial.println(" Hz.");
        }
        // If the file isn't open, the serial monitor shows an error:
        //else {
        //  Serial.println("!Error opening spectral_test.csv!");
        //}
      }
      sendFrequency(0);
    }

    else if (command.equals("runtest")) {
      Serial.print(F("Running test... "));
      for (int j = MINIMUM_FREQUENCY; j < MAXIMUM_FREQUENCY; j+=10) {
        sendFrequency(j);
        delay(1);
      }
      sendFrequency(0);
      Serial.println(F("done."));
    }

    // The following code is used for testing a specific frequency.
    else if (command.startsWith("test")) {
      String num = command.substring(5);          // Creates a string that contains the values starting from the 5th place holder
      Serial.println("Sampling at " + num);
      File dataFile = SD.open((num + ".csv").c_str(), FILE_WRITE);    // Starts file with the name of the frequency being sampled
      sendFrequency(num.toFloat());    // Sets the AD9850 to generate the selected frequency
      delay(100);     // Small delay for giving the speaker time to operate before sampling takes place
      if (dataFile) {
        // Program samples for a defined amount of times
        for (int i = SAMPLES; i >= 0; i--) {
          dataFile.println(String(analogRead(A0)) + "," + String(analogRead(A1)));  // Inserts the values in the file with a tab delimiter
        }
        dataFile.close();                               // Closes the file after the writing is done
        sendFrequency(0);                               // Sets the AD9850 to 0 to turn off noise
        Serial.println("Finished. Saved as " + num + ".csv");  // Debugger for signifying the completion of the file write
      }
    }

    // Shuts off the signal generator when either 0 or "off" are typed
    else if (command.equals("off") || command.equals("0")) {
      sendFrequency(0);
      Serial.println(F("AD9850 is deactivated."));
    }

    // Any number that is entered in will just activate the speaker to vibrate at a specific frequency
    else {
      Serial.print("Frequency is set to " + command);
      Serial.println(F(" Hz."));
      if (command.toInt() >= 1000) {
        // Gradually increase the frequency
        for (int i = 0; i < command.toInt(); i+=10) {
          sendFrequency(i);
          delayMicroseconds(100);
        }
      }

      // Play frequency as usual.
      else {
        sendFrequency(command.toDouble());
      }
    }
  }
}
