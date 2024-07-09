#include <FastLED.h>

#define LED_DT1 4
#define LED_DT2 5
#define LED_COUNT 59
#define BTN_PIN 0
#define INTER_PIN 3

int led_pin = 13;
int buttonState = 0;

//переменные аппаратного прерывания
volatile uint32_t debounce;
volatile byte mode = 0;
volatile bool submode = false;
volatile byte effectSpeed = 1;
volatile int brightValue = 0;

//переменные эффектов
byte loadDelay = 100; // 0-255
byte flashDelay = 100; // 0-255
byte breathMultip[] = {4, 8, 10}; // 0-32767
float flashMultip[] = {0.9850, 0.9200, 0.8000}; // 0-32767
int bpms[] = {120, 175, 450}; // 1-1000
byte bpmsSize = 3; // количество bpms и breathMultip

bool goesUP = false; // бул для затухания
uint32_t tmr1; //таймер для вспышек
double logBase = 0.95; //коэффициент логарифма затухания вспышки
byte currentLed = 0;

CRGB leds[LED_COUNT];
int ledsX[LED_COUNT][3];

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LED_DT1>(leds, LED_COUNT);
  FastLED.addLeds<NEOPIXEL, LED_DT2>(leds, LED_COUNT);
  pinMode(BTN_PIN, INPUT);
  pinMode(INTER_PIN, INPUT);
  delay(250);
  attachInterrupt(0, swap, CHANGE);

  //рассчитать bpm в миллисекунды
  for (int i = 0; i < bpmsSize; i++)
  {
    bpms[i] = 60000 / bpms[i];
  }
  for (int i = 0; i < bpmsSize; i++)
  {
    Serial.println(bpms[i]);
  }
  one_color_all(0, 0, 0);
  Serial.print("Ready!");
}

void loop() {
  digitalWrite(led_pin, HIGH);
  buttonState = analogRead(BTN_PIN);
  Serial.println(buttonState);
  switch (mode) {
    case 1: one_color();  break;
    case 2: breathing(); break;
    case 3: flash(); break;
    case 4: strobe(); break;
    case 5: one_color_all(0, 0, 0); colorWipe(0xff, 0xff, 0xff); colorWipe(0x00, 0x00, 0x00); break;
  }
}

void swap() {
  digitalWrite(led_pin, LOW);
  if (millis() - debounce >= 250 && digitalRead(2)) {
    digitalWrite(led_pin, LOW);
    debounce = millis();
 //   buttonState = analogRead(BTN_PIN);
  //  Serial.println(buttonState);
    if (buttonState >= 850) {                      //загрузка  (постепенно загорается от плеча полностью и оттуда же так же гаснет)
      mode = 5;
    }
    if (buttonState >= 590 && buttonState < 850) { //страб 3 скорости (режим эпилепсии)
      if (mode == 4) {
        effectSpeed = ++effectSpeed;
        if (effectSpeed >= 4) {
          effectSpeed = 1;
        }
      }
      mode = 4;
    }
    if (buttonState >= 420 && buttonState < 590) { //вспышка 3 скорости (мнгровенно загорается и медленно гаснет)
      if (mode == 3) {
        effectSpeed = ++effectSpeed;
        if (effectSpeed >= 4) {
          effectSpeed = 1;
        }
      }
      mode = 3;
    }
    if (buttonState >= 270 && buttonState < 420) { //дыхание 3 скорости
      if (mode == 2) {
        effectSpeed = ++effectSpeed;
        if (effectSpeed >= 4) {
          effectSpeed = 1;
        }
      }
      mode = 2;
    }
    if (buttonState >= 120 && buttonState < 270) { //статика и выкл
      if (mode != 1) {
        submode = false;
      }
      mode = 1;
      if (submode) {
        submode = false;
      }
      else {
        submode = true;
      }
    }
  }
}
void one_color() {
  if (submode) {
    one_color_all(255, 255, 255);
  }
  else {
    one_color_all(0, 0, 0);
  }
}

void one_color_all(int cred, int cgrn, int cblu) {
  for (int i = 0 ; i < LED_COUNT; i++ ) {
    leds[i].setRGB( cred, cgrn, cblu);
  }
  FastLED.show();
}

void breathing() {
  if (brightValue >= 255) {
    brightValue = 255;
  }
  for (int i = 0 ; i < LED_COUNT; i++ ) {
    leds[i].setRGB(brightValue, brightValue, brightValue);
  }
  FastLED.show();
  if (goesUP == true) {
    brightValue = brightValue + breathMultip[effectSpeed - 1];
  } else {
    brightValue = brightValue - breathMultip[effectSpeed - 1];
  }
  if (brightValue <= 0) {
    brightValue = 0;
    delay(10);
    goesUP = true;
  } else if (brightValue >= 255) {
    brightValue = 255;
    goesUP = false;
  }
  delay(20);
}

void flash() {
  if (brightValue >= 255) {
    brightValue = 255;
  }
  brightValue = brightValue * flashMultip[effectSpeed - 1];
  //Serial.println(brightValue);
  for (int i = 0 ; i < LED_COUNT; i++ ) {
    leds[i].setRGB(brightValue, brightValue, brightValue);
  }
  FastLED.show();
  delay(10);
  if (brightValue <= 0) {
    delay(100);
    brightValue = 255;
  }
}

void strobe() {
  if (millis() - tmr1 >= bpms[effectSpeed - 1]) {
    tmr1 = millis();
    for (int i = 0 ; i < LED_COUNT; i++ ) {
      leds[i].setRGB(255, 255, 255);
    }
    FastLED.show();
    for (int i = 0 ; i < LED_COUNT; i++ ) {
      leds[i].setRGB(0, 0, 0);
    }
    FastLED.show();
  }
}

void colorWipe(byte red, byte green, byte blue) {
  for (uint16_t i = 0; i < LED_COUNT && mode == 5; i++) {
    setPixel(i, red, green, blue);
    FastLED.show();
      buttonState = analogRead(BTN_PIN);
  Serial.println(buttonState);
    delay(35);
  }
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}
