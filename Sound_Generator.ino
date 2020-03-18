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

#define MAXIMUM_FREQUENCY 3000
#define MINIMUM_FREQUENCY 150
#define SAMPLES 10000

#define FASTADC 1

// Defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

const uint8_t CHIP_SELECT = 13;

int sensor1 = 0;
int sensor2 = 0;
int freq_num = 0;
uint32_t lastSample = 0;

String command;  // Used for the serial input function
String sensor_read; // Used as an object to append the sensor readings
String read_string;

char filepath1[] = "spectral_test.txt";

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
  Serial.println("Analog Measurement prescale: 64----");
  Serial.println("SD card Chip Select Pin: 13--------");
  Serial.println("");
  delay(1500);
  // Wait until the SD card initializes
  while (!Serial) {}
  Serial.print("Initializing SD card... ");

  // If the SD card doesn't start up:
  if (!SD.begin(CHIP_SELECT)) {
    //SD.initErrorHalt();
    Serial.println("initialization failed!");
    //while (1);
    return;
  }
  delay(1000);

  Serial.println("initialization done.");
  Serial.println("");
  Serial.println("Frequency Range is from 0 to 10 Mhz.");
  Serial.println("The human ear can only hear noises from 80 Hz");
  Serial.println("to 20,000 Hz. Anything outside these cannot  ");
  Serial.println("be heard.");
  Serial.println("");
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
    if (command.equals("full test")) {
      Serial.println("Starting full spectral test.");
      File dataFile = SD.open(filepath1, FILE_WRITE);

      // When the data file is available, the program will write to it
      if (dataFile) {
        if (SD.exists(filepath1)) {
          // The program deletes the pre-existing file to make room for the new one
          Serial.println("File alread exists. Overwriting file.");
          SD.remove(filepath1);
          delay(100);
        }

        // Start the frequency read from high to low frequency
        for (int j = MAXIMUM_FREQUENCY; j >= MINIMUM_FREQUENCY; j--) {
          // Prints the frequency being tested and writes to the AD9850 the frequency value
          sendFrequency(float(j));

          // This for loop obtains a specified number of samples
          for (int i = SAMPLES; i >= 0; i--) {
            dataFile.println(analogRead(A0) + "\t" + analogRead(A1));  // Inserts the values in the file with a tab delimiter
          }
          dataFile.println("");                       // Creates a new line when the frequency changes
        }
        dataFile.close();                             // Closes the file after the writing is done
        Serial.println("Finished");                      // Debugger for signifying the completion of the file write
      }
      // If the file isn't open, the serial monitor shows an error:
      else {
        Serial.println("error opening spectral_test.txt");
      }
    }

    // The following code is used for testing a specific frequency.
    else if (command.startsWith("test")) {
      String num = command.substring(5);          // Creates a string that contains the values starting from the 5th place holder
      Serial.println("Sampling at " + num);
      File dataFile = SD.open((num + ".txt").c_str(), FILE_WRITE);    // Starts file with the name of the frequency being sampled
      sendFrequency(num.toFloat());                   // Sets the AD9850 to generate the selected frequency

      // Program samples for a defined amount of times
      for (int i = SAMPLES; i >= 0; i--) {
        dataFile.println(analogRead(A0) + "\t" + analogRead(A1));  // Inserts the values in the file with a tab delimiter
      }
      dataFile.close();                               // Closes the file after the writing is done
      sendFrequency(0);                               // Sets the AD9850 to 0 to turn off noise
      Serial.println("Finished");                     // Debugger for signifying the completion of the file write
    }

    // Shuts off the signal generator when either 0 or "off" are typed
    else if (command.equals("off") || command.equals("0")) {
      sendFrequency(0);
      Serial.println("AD9850 is deactivated.");
    }

    // Any number that is entered in will just activate the speaker to vibrate at a specific frequency
    else {
      Serial.println("Frequency is set to " + command);
      sendFrequency(command.toFloat());
    }
  }
}
