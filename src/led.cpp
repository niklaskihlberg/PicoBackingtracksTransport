#include "main.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include <math.h>

void getLedPin(uint8_t led, int &r, int &g, int &b) { // Hjälpfunktion för att hämta RGB-pinnar för RGB-Leddar
  switch (led) {
    case RWD: r = LED_RWD_R_PIN; g = LED_RWD_G_PIN; b = LED_RWD_B_PIN; break;
    case FWD: r = LED_FWD_R_PIN; g = LED_FWD_G_PIN; b = LED_FWD_B_PIN; break;
  }
}

void setLedRgb(uint8_t led, uint8_t color) {
  int r, g, b;
  getLedPin(led, r, g, b);
  switch (color) {
    case WHITE:        pwm_set_gpio_level(r, PWM_FULL); pwm_set_gpio_level(g, PWM_FULL); pwm_set_gpio_level(b, PWM_FULL); break;
    case GRAY:         pwm_set_gpio_level(r, PWM_DIM); pwm_set_gpio_level(g, PWM_DIM); pwm_set_gpio_level(b, PWM_DIM); break;
    case GREEN:        pwm_set_gpio_level(r, PWM_OFF);  pwm_set_gpio_level(g, PWM_FULL); pwm_set_gpio_level(b, PWM_OFF);  break;
    case BLUE:         pwm_set_gpio_level(r, PWM_OFF);  pwm_set_gpio_level(g, PWM_OFF);  pwm_set_gpio_level(b, PWM_FULL); break;
    case RED:          pwm_set_gpio_level(r, PWM_FULL); pwm_set_gpio_level(g, PWM_OFF);  pwm_set_gpio_level(b, PWM_OFF);  break;
    case OFF: default: pwm_set_gpio_level(r, PWM_OFF);  pwm_set_gpio_level(g, PWM_OFF);  pwm_set_gpio_level(b, PWM_OFF);  break;
  }
}

static const uint8_t colors[4][2] = { // För varje durata: { färg om tryckt, färg om släppt }
  { GRAY, OFF },    // OFF
  { WHITE, GREEN }, // CUE
  { WHITE, BLUE },  // BAR
  { WHITE, RED }    // BIT
};

void update_ancora_led() {
  uint8_t idx = (durata <= BIT) ? durata : 0;
  setLedRgb(RWD, btnRwd.stt ? colors[idx][0] : colors[idx][1]);
}

void update_avanti_led() {
  uint8_t idx = (durata <= BIT) ? durata : 0;
  setLedRgb(FWD, btnRwd.stt ? colors[idx][0] : colors[idx][1]);
}

void update_loop_led() {
  gpio_put(LED_LOP_PIN, AllInfinito ? PWM_FULL : PWM_OFF); // Sätt LOOP LED baserat på AllInfinito
}

// Mjuk fade för LOOP-LED. Anropa denna regelbundet (t.ex. varje 10–20 ms)
void update_loop_led_task() {
  static uint16_t current_level = 0; // nuvarande PWM-nivå
  const uint16_t target = AllInfinito ? PWM_FULL : PWM_OFF;
  const uint16_t step = 32; // fade-hastighet (justera vid behov)

  if (current_level < target) {
    current_level = (current_level + step > target) ? target : current_level + step;
  } else
  if (current_level > target) {
    current_level = (current_level < step + target) ? target : current_level - step;
  }
  pwm_set_gpio_level(LED_LOP_PIN, current_level);
}

void update_leds() {
  update_ancora_led();
  update_avanti_led();
  update_loop_led();
}
