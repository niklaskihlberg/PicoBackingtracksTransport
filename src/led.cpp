#include "main.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include <math.h>

enum COLOR { // Färgkonstanter för RWD/FWD LED och PWM-nivåer
  BLACK = 0,   // 0% PWM
  WHITE = 1, // 100% PWM
  GRAY = 2,  // 80% PWM (PWM_GRAY)
  GREEN = 3, // 100% PWM för grön (Cue)
  BLUE = 4,  // 100% PWM för blå (Bar)
  RED = 5    // 100% PWM för röd (Bit)
};

enum DIRECTION {
  RWD = 0,
  FWD = 1
};

enum BRIGHTNESS {
  ZERO = 0,  // 0% PWM
  DIM  = 80, // 20% PWM (80% av 4096)
  FULL = 255 // 100% PWM
};


void getLedPin(uint8_t DIRECTION, int &R, int &G, int &B) { // Hjälpfunktion för att hämta RGB-pinnar för RGB-Leddar
  switch (DIRECTION) {
    case DIRECTION::RWD: R = LED_RWD_R_PIN; G = LED_RWD_G_PIN; B = LED_RWD_B_PIN; break;
    case DIRECTION::FWD: R = LED_FWD_R_PIN; G = LED_FWD_G_PIN; B = LED_FWD_B_PIN; break;
  }
}

void setLedRgb(uint8_t DIRECTION, COLOR COLOR) { // Ändra typ till LedColor
  int R, G, B;
  getLedPin(DIRECTION, R, G, B);
  switch (COLOR) {
    case COLOR::WHITE:          pwm_set_gpio_level(R, BRIGHTNESS::FULL); pwm_set_gpio_level(G, BRIGHTNESS::FULL); pwm_set_gpio_level(B, BRIGHTNESS::FULL);  break;
    case COLOR::GRAY:           pwm_set_gpio_level(R, BRIGHTNESS::DIM);  pwm_set_gpio_level(G, BRIGHTNESS::DIM);  pwm_set_gpio_level(B, BRIGHTNESS::DIM);   break;
    case COLOR::GREEN:          pwm_set_gpio_level(R, BRIGHTNESS::ZERO); pwm_set_gpio_level(G, BRIGHTNESS::FULL); pwm_set_gpio_level(B, BRIGHTNESS::ZERO);  break;
    case COLOR::BLUE:           pwm_set_gpio_level(R, BRIGHTNESS::ZERO); pwm_set_gpio_level(G, BRIGHTNESS::ZERO); pwm_set_gpio_level(B, BRIGHTNESS::FULL);  break;
    case COLOR::RED:            pwm_set_gpio_level(R, BRIGHTNESS::FULL); pwm_set_gpio_level(G, BRIGHTNESS::ZERO); pwm_set_gpio_level(B, BRIGHTNESS::ZERO);  break;
    case COLOR::BLACK: default: pwm_set_gpio_level(R, BRIGHTNESS::ZERO); pwm_set_gpio_level(G, BRIGHTNESS::ZERO); pwm_set_gpio_level(B, BRIGHTNESS::ZERO);  break;
  }
}

void update_ancora_led() {
  switch (durata) { // Uppdatera RWD LED baserat på "durata"
    case DURATA::IDL: setLedRgb(RWD, btnRwd.stt ? COLOR::GRAY  : COLOR::BLACK); break;
    case DURATA::CUE: setLedRgb(RWD, btnRwd.stt ? COLOR::WHITE : COLOR::GREEN); break;
    case DURATA::BAR: setLedRgb(RWD, btnRwd.stt ? COLOR::WHITE : COLOR::BLUE);  break;
    case DURATA::BIT: setLedRgb(RWD, btnRwd.stt ? COLOR::WHITE : COLOR::RED);   break;
  }
}

void update_avanti_led() {
  switch (durata) { // Uppdatera FWD LED baserat på "durata"
    case DURATA::IDL: setLedRgb(FWD, btnFwd.stt ? COLOR::GRAY  : COLOR::BLACK); break;
    case DURATA::CUE: setLedRgb(FWD, btnFwd.stt ? COLOR::WHITE : COLOR::GREEN); break;
    case DURATA::BAR: setLedRgb(FWD, btnFwd.stt ? COLOR::WHITE : COLOR::BLUE);  break;
    case DURATA::BIT: setLedRgb(FWD, btnFwd.stt ? COLOR::WHITE : COLOR::RED);   break;
  }
}

void set_loop_led_off() {
  pwm_set_gpio_level(LED_LOP_PIN, BRIGHTNESS::ZERO); // Stäng av LOOP LED
}

void set_loop_led_sequence() { // "Pulse-effekt" för LOOP-LED
    
  if (mspb == 0) return; // skydd mot delning med noll

  float phase = (now - the_latest_beat) / (float)mspb;
  if (phase > 1.0f) phase = 1.0f; // clamp

  // Fade: max vid beat, min halvvägs, max igen vid nästa beat
  float min_brightness = 0.3f; // 30% av max
  float brightness = min_brightness + (1.0f - min_brightness) * 0.5f * (1.0f + cosf(M_PI * phase));
  uint16_t pwm_value = (uint16_t)(FULL * brightness);

  pwm_set_gpio_level(LED_LOP_PIN, pwm_value);

}

void update_loop_led() {
  AllInfinito ? set_loop_led_sequence() : set_loop_led_off(); // Sätt LOOP LED baserat på AllInfinito
}

void set_durata_led(btn *btn) { // Uppdatera alla LED:ar baserat på "durata"
  if (btn->stt) pwm_set_gpio_level(btn->pin, BRIGHTNESS::FULL);
  else pwm_set_gpio_level(btn->pin, BRIGHTNESS::ZERO);
}

void update_durata_leds() { // Uppdatera alla LED:ar baserat på "durata"
  set_durata_led(&btnCue);
  set_durata_led(&btnBar);
  set_durata_led(&btnBit);
}

void update_leds() {
  update_ancora_led();
  update_avanti_led();
  update_loop_led();
  update_durata_leds();
}
