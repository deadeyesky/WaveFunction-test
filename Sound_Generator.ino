/*
* This is the script for the 2020 science fair for analyzing
* the propogation of wave energy throughout a material by
* applying acoustic sound to it and then measuring the 
* returning energy with a pair of microphones.
*/

#define W_CLK 6       // Pin 6 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 5       // Pin 5 - connect to freq update pin (FQ)
#define DATA 4       // Pin 4 - connect to serial data load pin (DATA)
#define RESET 3      // Pin 3 - connect to reset pin (RST).

#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }

String input;  // Used for the serial input function

void initialize_serial_message () {
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
  Serial.println("");
  Serial.println("Enter a frequency.");
  Serial.println("");
}

// transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte(byte data) {
  for (int i = 0; i < 8; i++, data >>= 1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency(double frequency) {
  int32_t freq = frequency * 4294967295 / 125000000;  // note 125 MHz clock on 9850
  for (int b = 0; b < 4; b++, freq >>= 8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
}

void setup() {
  // Run Setup function
  initialize_serial_message();
  
  // Configure data pins for output
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);

  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);  // This pulse enables serial mode - Datasheet page 12 figure 10
}

void loop() {
  if(Serial.available()){
    input = Serial.readStringUntil('\n');
    Serial.println("Frequency set to " + input + "Hz");
    
    // Frequency number gets applied to the AD9850 from the Nano
    sendFrequency(input.toFloat());
  }
}
