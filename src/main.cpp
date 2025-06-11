#include <stdio.h>
#include "main.h"
#include "tusb.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/stdio.h"

#include "pico/time.h"
#include "bsp/board.h"


void update_time() {
  now = to_ms_since_boot(get_absolute_time()); // Uppdatera nuvarande tid
};


void setup_btn_pin(uint8_t PIN) {
  gpio_init(PIN);
  gpio_set_dir(PIN, GPIO_IN);
  gpio_pull_up(PIN); // Aktivera pull-up motstånd
}

void setup_led_pin(uint8_t PIN) {
  gpio_init(PIN);
  gpio_set_dir(PIN, GPIO_OUT);
  gpio_put(PIN, 0);
  gpio_set_function(PIN, GPIO_FUNC_PWM);
  uint slice = pwm_gpio_to_slice_num(PIN);
  pwm_set_enabled(slice, true);
}

uint8_t BTN_PINS[6] = {
  BTN_LOP_PIN,   // Loop-knapp
  BTN_CUE_PIN,   // Cue-knapp
  BTN_BAR_PIN,   // Bar-knapp
  BTN_BIT_PIN,   // Bit-knapp
  BTN_RWD_PIN,   // Rewind/Ancora-knapp
  BTN_FWD_PIN    // Forward/Avanti-knapp
};

uint8_t LED_PINS[10] = {
  LED_LOP_PIN,   // Loop LED
  LED_RWD_R_PIN, // RWD Red LED
  LED_RWD_G_PIN, // RWD Green LED
  LED_RWD_B_PIN, // RWD Blue LED
  LED_FWD_R_PIN, // FWD Red LED
  LED_FWD_G_PIN, // FWD Green LED
  LED_FWD_B_PIN, // FWD Blue LED
  LED_CUE_PIN,   // Cue LED
  LED_BAR_PIN,   // Bar LED
  LED_BIT_PIN    // Bit LED
};


void init_AllInfinito() {
  sendMidiCC(0x64, 64); // Query AllInfinito state
  AllInfinito = false; // Initiera AllInfinito till false
  printf("\033[38;5;218m[MRX]\033[97m AllInfinito: \033[31mfalse\033[0m\n");
}

void setup() {

  board_init(); // Initiera GPIO
  tud_init(0); // Initiera Tiny USB Device

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  stdio_usb_init(); // Initiera USB-serial för debug

  for (int i = 0; i < 6; i++) setup_btn_pin(BTN_PINS[i]); // Initiera alla knappar
  for (int i = 0; i < 10; i++) setup_led_pin(LED_PINS[i]); // Initiera alla LED:ar

  init_AllInfinito(); // Initiera AllInfinito

}

int main() {
  setup();
  while (1) {
    tud_task();   // TinyUSB, gör din grej!
    read_midi(); // Got midi?
    update_time(); // Kolla klockan!
    update_buttons(); // Push it!
    update_leds(); // Light it up!
    // sleep_ms(1); // Om du vill sänka CPU-belastning
  }
  return 0;
}
