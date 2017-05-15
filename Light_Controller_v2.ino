#include <DmxSimple.h>


byte led1 = 0;
byte led2 = 0;
byte lcd = B00000000;
byte lcd_pos = 0;

byte master = 0;

byte ui[31];
byte ui_index = 0;

byte test = 4;

//SMOKE
byte smoke = 0;
long smokeTimeout = 0;

byte DmxChannels = 0;


void write_o(byte bit0 = 0, byte bit1 = 0, byte lcdD = 0, byte lcd_data = 0) {

  lcd_data = !!lcd_data;

  for (int i = 0; i < (!!lcdD) + 1; i++) {

    byte buff = 0;

    if (lcdD) {

      buff |= !!(lcdD & (B10000000 >> (4 * i)));
      buff <<= 1;
      buff |= !!(lcdD & (B01000000 >> (4 * i)));
      buff <<= 1;
      buff |= !!(lcdD & (B00100000 >> (4 * i)));
      buff <<= 1;
      buff |= !!(lcdD & (B00010000 >> (4 * i)));
      buff <<= 1;

      buff |= 1;
      buff <<= 1; //ENABLE HIGH

      buff |= lcd_data; //DATA 1 or INST 0
      buff <<= 1;


    }

    buff |= bit1;
    buff <<= 1;

    buff |= bit0;



    reg_write(buff);

    if (lcdD) {
      reg_write(buff & B11110111);
    }
  }

  //

}

void reg_write(byte msg) {


  for (int i = 7; i >= 0; i--) {

    PORTB &= B11111000;

    PORTB |= (B00000001 & (msg >> i)) ;//<< 4; //DATA

    PORTB |= B00000010;//clk

  }

  PORTB |= B00000100; //LATCH



}

void lcd_print(String str) {

  for (int i = 0; i < str.length(); i++) {

    if (lcd_pos & B00001111) {

    }

    byte b = str.charAt(i) | B10000000;
    ui[ui_index++] = b;


  }

}

// the setup routine runs once when you press reset:
void setup() {

  Serial.begin(31250);

  //DMX INIT
  DmxSimple.usePin(4);

  DmxChannels = 4 * 4;

  DmxSimple.maxChannel(DmxChannels);
  for (byte i = 0; i < DmxChannels / 4; i++) {
    DmxSimple.write((i * 4) + 1, 128);
    DmxSimple.write((i * 4) + 2, 255);
  }
  // initialize the digital pin as an output.
  pinMode(10, OUTPUT);  //LATCH
  pinMode(9, OUTPUT); //CLOCK
  pinMode(8, OUTPUT); //DATA

  //INIT 4-BIT LCD
  reg_write(B00111000);
  reg_write(B00110000);
  reg_write(B00111000);
  reg_write(B00110000);
  reg_write(B00111000);
  reg_write(B00110000);//Writes B00110000 x3 - Makes sure its in 8-Bit mode
  reg_write(B00101000);
  reg_write(B00100000);//4-Bit mode

  write_o(0, 0, B00101000, 0); //4Bit,2Lines,5x8pixels
  write_o(0, 0, B00000110, 0); //Sets movement.
  write_o(0, 0, B00010000, 0);
  write_o(1, 0, B00001111, 0); //Turns on Display, Cursor and Blink
  write_o(0, 1, B00000001, 0); //Clears display

  write_o(0, 1, 48, 1);
  write_o(0, 1, 49, 1);
  write_o(0, 1, 50, 1);
  write_o(0, 1, 51, 1);



}

// the loop routine runs over and over again forever:
void loop() {

  long currentTime = millis();

  //MIDI INPUT
  while (Serial.available()) {

    byte midi = Serial.read();

    // lcd_print(String(midi));

    while (!Serial.available()) {}

    if (midi == 192) {

      midi = Serial.read();
      //lcd_print(String(midi));

      switch (midi) {

        case 1 :
          digitalWrite(7, 1 & ++smoke);
          smokeTimeout = currentTime + 2000;
          lcd_print("Smoke");
          break;

        case 2:

          for (byte i = 0; i < DmxChannels / 4; i++) {
            DmxSimple.write((i * 4) + 1, 255);
            DmxSimple.write((i * 4) + 2, 255);
            DmxSimple.write((i * 4) + 3, 0);
            DmxSimple.write((i * 4) + 4, 0);
          }
          lcd_print("Red");
          break;
        case 3:
          for (byte i = 0; i < DmxChannels / 4; i++) {
            DmxSimple.write((i * 4) + 1, 255);
            DmxSimple.write((i * 4) + 2, 0);
            DmxSimple.write((i * 4) + 3, 255);
            DmxSimple.write((i * 4) + 4, 0);
          }
          lcd_print("Green");
          break;
        case 4:
          for (byte i = 0; i < DmxChannels / 4; i++) {
            DmxSimple.write((i * 4) + 1, 255);
            DmxSimple.write((i * 4) + 2, 0);
            DmxSimple.write((i * 4) + 3, 0);
            DmxSimple.write((i * 4) + 4, 255);
          }
          lcd_print("Blue");
          break;

      }

      Serial.flush();

    }
  }

  if ((1 & smoke) && currentTime > smokeTimeout) {

    digitalWrite(7, LOW);
    smoke++;

  }

  led1 = random() & 1;
  led2 = random() & 1;

  write_o(led1, led2);

  ui_index = 0;

  while (ui[0] != 0) {

    write_o(led1, led2, (ui[0] & B01111111), (ui[0] & B10000000));

    ui[0] = ui[++ui_index];
    ui[ui_index] = 0;

    if (ui[0] != 0) {
      delay(.5);
    }

  }

  //delay(1000);



}



