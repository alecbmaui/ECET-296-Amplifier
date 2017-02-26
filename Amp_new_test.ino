#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

int CS = 10;
int volAddress = (B00010011);
volatile int volume = 0;
int old_volume = 0;
int l_oldVol = 0;
int r_oldVol = 0;
const int max_vol = 20;
const int min_vol = 0;
bool mute = false;
bool stby = false;
bool zero = false;
int mutePin;
int stbyPin;
volatile int volSelect = 0;
int volArray [] = {0, 3, 8, 15, 23, 32, 42, 53, 65, 77, 90, 104, 119, 134, 149, 166, 182, 200, 218, 236, 255};
volatile int lVol = 0;
volatile int rVol = 0;
bool l = false;
bool r = false;
int buttonCount = 0;   // counter for the number of button presses
int button = 0;         // current state of the button
int lastButton = 0;

void setup() {

  pinMode(CS, OUTPUT);
  // initialize SPI:
  SPI.begin();
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  Serial.begin(9600);
  attachInterrupt(0, cw, FALLING);
  attachInterrupt(1, ccw, FALLING);
  volumeSet (0, 0);
  lcd.clear();
  lcd.print("Volume:       ");
  lcd.println(volume);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);

}

void loop() {

 if (stby == true && digitalRead(9) == LOW) {
  }
  else if (stby == true && digitalRead(9) == HIGH && mute == false) {
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    lcd.clear();
    lcd.print("Volume:       ");
    lcd.println(volume);
    stby = false;
  }
  else if (stby == true && digitalRead(9) == HIGH && mute == true) {
    digitalWrite(4, HIGH);
    lcd.clear();
    lcd.println("MUTE");
    stby = false;
  }
  if (mute == true && digitalRead(8) == LOW && stby == false) {
  }
  else if (mute == true && digitalRead(8) == HIGH && stby == false) {
    digitalWrite(5, HIGH);
    lcd.clear();
    lcd.print("Volume:       ");
    lcd.println(volume);
    mute = false;
  }
  if (mute == false && stby == false) {
    button = digitalRead(6);
    if (button != lastButton) {
      if (button == HIGH && buttonCount != 2) {
        buttonCount++;
        if (buttonCount == 1){
          
        }
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
      Serial.println(volume);
      old_volume = volume;
      lcd.clear();
      lcd.print("Volume:       ");
      lcd.println(volume);
      // int volSet = 12.75 * volume;
      volumeSet(lVol, rVol);
    }
   
    if (buttonCount == 1 && l == false){
       lcd.clear();
      lcd.print("Left:       ");
      lcd.println(lVol);
      l = true;
    }
    if (buttonCount == 1 && l == true) {
     
      if (l_oldVol != lVol)
      {
        lVol = l_oldVol;
        lcd.clear();
        lcd.print("Left:       ");
        lcd.println(lVol);
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
      lcd.clear();
      lcd.print("Volume:       ");
      lcd.println(volume);
      l = false;
    }

    if (buttonCount == 2 && r == false){
       lcd.clear();
      lcd.print("Right:       ");
      lcd.println(rVol);
      r = true;
    }
    if (buttonCount == 2 && r == true) {

      if (r_oldVol != rVol)
      {
        rVol = r_oldVol;
        lcd.clear();
        lcd.print("Right:       ");
        lcd.println(rVol);
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
      lcd.clear();
      lcd.print("Volume:       ");
      lcd.println(volume);
      r = false;
    }
  }
  if (digitalRead(8) == LOW && mute == false && stby == false)
  {
    digitalWrite(5, LOW);
    lcd.clear();
    lcd.println("MUTE");
    mute = true;
  }
  if (digitalRead(9) == LOW && stby == false)
  {
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    lcd.clear();
    lcd.println("STBY");
    stby = true;
  }
  if (volume == 0){
    digitalWrite(5, LOW);
    zero = true;
  }
  else if (volume != 0 && zero == true){
    digitalWrite(5, HIGH);
    zero = false;
  }
}

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

void leftVol(int value) {
  // take the CS pin low to select the chip:
  digitalWrite(CS, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(B00010010);
  SPI.transfer(volArray [value]);
  // take the CS pin high to de-select the chip:
  digitalWrite(CS, HIGH);
}

void rightVol(int value) {
  // take the CS pin low to select the chip:
  digitalWrite(CS, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(B00010001);
  SPI.transfer(volArray [value]);
  // take the CS pin high to de-select the chip:
  digitalWrite(CS, HIGH);
}

void cw()
{
  int b = digitalRead(3);
  if (digitalRead(8) == HIGH && b == HIGH) {

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


void ccw()
{
  int a = digitalRead(2);
  if (digitalRead(8) == HIGH && a == HIGH) {

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

