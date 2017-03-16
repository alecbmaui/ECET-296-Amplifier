#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

int CS = 10;
int volAddress = (B00010011);
volatile int volume = 0;
int old_volume = 0;
volatile int l_oldVol = 0;
volatile int r_oldVol = 0;
const int max_vol = 20;
const int min_vol = 0;
bool mute = false;
bool stby = false;
bool zero = false;
int muteSwitch = 8;
int mutePin = 5;
int stbyPin = 4;
int stbySwitch = 9;
int volArray [] = {0, 3, 8, 15, 23, 32, 42, 53, 65, 77, 90, 104, 119, 134, 149, 166, 182, 200, 218, 236, 255};
volatile int lVol = 0;
volatile int rVol = 0;
bool l = false;
bool r = false;
int buttonCount = 0;   // counter for the number of button presses
int button = 0;         // current state of the button
int lastButton = 0;
char bluetooth;
int language = 0;

void setup() {

  pinMode(CS, OUTPUT);
  // initialize SPI:
  SPI.begin();
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.noCursor();
  Serial.begin(115200);
  attachInterrupt(0, cw, FALLING);
  attachInterrupt(1, ccw, FALLING);
  volumeSet (0, 0);
  lcdPrintVol(buttonCount, volume);
  digitalWrite(stbyPin, HIGH);
  digitalWrite(mutePin, HIGH);

}
void loop() {

  if (stby == true && digitalRead(stbySwitch) == LOW) {
  }
  else if (stby == true && digitalRead(stbySwitch) == HIGH && mute == false) {
    digitalWrite(stbyPin, HIGH);
    digitalWrite(mutePin, HIGH);
    lcdPrintVol(buttonCount, volume);
    stby = false;
  }
  else if (stby == true && digitalRead(stbySwitch) == HIGH && mute == true) {
    digitalWrite(stbyPin, HIGH);
    lcdMute(language);
    stby = false;
  }
  if (mute == true && digitalRead(muteSwitch) == LOW && stby == false) {
    button = digitalRead(6);
    //If button state changes, a variable is incremented between 0, 1, & 2
    if (button != lastButton) {
      if (button == HIGH && language != 1) {
        language++;
        lcdMute(language);
      }
      else if (button == HIGH && language == 1) {
        language = 0;
        lcdMute(language);
      }
      lastButton = button;
      delay (50);
    }
  }
  else if (mute == true && digitalRead(muteSwitch) == HIGH && stby == false) {
    digitalWrite(mutePin, HIGH);
    lcdPrintVol(buttonCount, volume);
    mute = false;
  }
  if (mute == false && stby == false) {
    //Checks for input from Bluetooth controls cominf from Serial
    if (Serial.available() > 0) {
      bluetooth = Serial.read();
      Serial.println(bluetooth);
      if (bluetooth == 'U' && volume != max_vol) {
        volume++;
        l_oldVol++;
        r_oldVol++;
      }
      else if (bluetooth == 'D' && volume != min_vol) {
        volume--;
        l_oldVol--;
        r_oldVol--;
      }
      else if (bluetooth == 'B') {
        if (buttonCount != 2) {
          buttonCount++;
        }
        else if (buttonCount == 2) {
          buttonCount = 0;
        }
      }
      else {

      }
    }
    button = digitalRead(6);
    //If button state changes, a variable is incremented between 0, 1, & 2
    if (button != lastButton) {
      if (button == HIGH && buttonCount != 2) {
        buttonCount++;
      }
      else if (button == HIGH && buttonCount == 2) {
        buttonCount = 0;
      }
      lastButton = button;
      delay (50);
    }
    if (volume != old_volume && r == false && l == false)
    {
      lVol = l_oldVol;
      rVol = r_oldVol;
      old_volume = volume;
      lcdPrintVol(buttonCount, volume);
      volumeSet(lVol, rVol);
    }

    if (buttonCount == 1 && l == false) {
      lcdPrintVol(buttonCount, lVol);
      l = true;
    }
    if (buttonCount == 1 && l == true) {
      
      if (l_oldVol != lVol)
      {
        lVol = l_oldVol;
        lcdPrintVol(buttonCount, lVol);
        leftVol(lVol);
        if (lVol > rVol) {
          volume = lVol;
          r_oldVol = rVol;
        }
        else {
          volume = rVol;
          r_oldVol = rVol;
        }
      }
    }
    else if (buttonCount != 1 && l == true) {
      lcdPrintVol(buttonCount, volume);
      l = false;
    }
    if (buttonCount == 2 && r == false) {
      lcdPrintVol(buttonCount, rVol);
      r = true;
    }
    if (buttonCount == 2 && r == true) {
      if (r_oldVol != rVol)
      {
        rVol = r_oldVol;
        lcdPrintVol(buttonCount, rVol);
        rightVol(rVol);
        if (rVol > lVol) {
          volume = rVol;
          l_oldVol = lVol;
        }
        else {
          volume = lVol;
          l_oldVol = lVol;
        }
      }
    }

    else if (buttonCount != 2 && r == true) {
      lcdPrintVol(buttonCount, volume);
      r = false;
    }
  }
  if (digitalRead(muteSwitch) == LOW && mute == false && stby == false)
  {
    digitalWrite(mutePin, LOW);
    lcdMute(language);
    mute = true;
  }
  if (digitalRead(stbySwitch) == LOW && stby == false)
  {
    digitalWrite(mutePin, LOW);
    digitalWrite(stbyPin, LOW);
    lcdStby(language);
    stby = true;
  }
  //If volume is set to 0, Mute is activated in order to silence the audio system
  if (volume == 0) {
    digitalWrite(mutePin, LOW);
    zero = true;
  }
  //Pulls the mute pin High if the volume is set above 0, allowing audio through the output
  else if (volume != 0 && zero == true) {
    digitalWrite(mutePin, HIGH);
    zero = false;
  }
}

//Sets value for both addressable Digital Potentiometers of the MCP42050
void volumeSet(int lvalue, int rvalue) {
  // take the CS pin low to select the chip:
  digitalWrite(CS, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(B00010010);
  SPI.transfer(volArray [lvalue]);
  // take the CS pin high to de-select the chip:
  digitalWrite(CS, HIGH);
  // take the CS pin low to select the chip:
  digitalWrite(CS, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(B00010001);
  SPI.transfer(volArray [rvalue]);
  // take the CS pin high to de-select the chip:
  digitalWrite(CS, HIGH);
}

//Sets value for Digital Potentiometer, corresponding to the Left channel
void leftVol(int value) {
  // take the CS pin low to select the chip:
  digitalWrite(CS, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(B00010010);
  SPI.transfer(volArray [value]);
  // take the CS pin high to de-select the chip:
  digitalWrite(CS, HIGH);
}

//Sets value for Digital Potentiometer, corresponding to the Right channel
void rightVol(int value) {
  // take the CS pin low to select the chip:
  digitalWrite(CS, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(B00010001);
  SPI.transfer(volArray [value]);
  // take the CS pin high to de-select the chip:
  digitalWrite(CS, HIGH);
}

//Clockwise turn/Incrementing function for Rotary Encoder
void cw()
{
  int b = digitalRead(3);
  if (digitalRead(8) == HIGH && b == HIGH && volume != max_vol) {
    volume++;
    l_oldVol++;
    r_oldVol++;
  }
  if (volume > max_vol)
    volume = max_vol;
  if (l_oldVol > max_vol)
    l_oldVol = max_vol;
  if (r_oldVol > max_vol)
    r_oldVol = max_vol;
}

//Counter-clockwise turn/Decrementing function for Rotary Encoder
void ccw()
{
  int a = digitalRead(2);
  if (digitalRead(8) == HIGH && a == HIGH && volume != min_vol) {
    volume--;
    l_oldVol--;
    r_oldVol--;
  }
  if (volume < min_vol)
    volume = min_vol;
  if (l_oldVol < min_vol)
    l_oldVol = min_vol;
  if (r_oldVol < min_vol)
    r_oldVol = min_vol;
}

//Given a mode number and a volume, outputs the properly formatted LCD display
void lcdPrintVol(int mode, int vol) {
  lcd.clear();
  if (mode == 0) {
    lcd.print("Volume:       ");
  }
  else if (mode == 1) {
    lcd.print("Left:         ");
  }
  else if (mode == 2) {
    lcd.print("Right:        ");
  }
  if (vol < 10) {
    lcd.print("0");
  }
  lcd.print(vol);
}

void lcdMute(int lang){
  lcd.clear();
  if (lang == 0){
    lcd.println("MUTE            ");
  }
  else if (lang == 1){
    lcd.println("ミュート            ");
  }
}

void lcdStby(int lang){
  lcd.clear();
  if (lang == 0){
    lcd.println("STBY            ");
  }
  else if (lang == 1){
    lcd.println("待機する            ");
  }
}

void checkVol (int a, int b){
   if (a > b) {
          volume = a;
        }
        else {
          volume = b;
        }
}
