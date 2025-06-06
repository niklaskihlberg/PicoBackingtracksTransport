#include "main.h"
#include "tusb.h"
// #include "tusb_config.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
// #include "pico/stdio.h"

#include "pico/time.h"
#include "bsp/board.h"
// #include "class/midi/midi_device.h"
#include <stdio.h>
#include <math.h>



// #include <stdio.h>





uint32_t now = 0;

void kolla_klockan() {
  now = to_ms_since_boot(get_absolute_time()); // Uppdatera nuvarande tid
};

void setup() {

  board_init(); // Initiera GPIO

  tud_init(0); // Initiera Tiny USB Device

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  stdio_usb_init(); // Initiera USB-serial för debug

  gpio_init(BTN_LOP_PIN); gpio_set_dir(BTN_LOP_PIN, GPIO_IN); gpio_pull_up(BTN_LOP_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_CUE_PIN); gpio_set_dir(BTN_CUE_PIN, GPIO_IN); gpio_pull_up(BTN_CUE_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_BAR_PIN); gpio_set_dir(BTN_BAR_PIN, GPIO_IN); gpio_pull_up(BTN_BAR_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_BIT_PIN); gpio_set_dir(BTN_BIT_PIN, GPIO_IN); gpio_pull_up(BTN_BIT_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_FWD_PIN); gpio_set_dir(BTN_FWD_PIN, GPIO_IN); gpio_pull_up(BTN_FWD_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_RWD_PIN); gpio_set_dir(BTN_RWD_PIN, GPIO_IN); gpio_pull_up(BTN_RWD_PIN); // Konfigurera knappar som ingångar

  gpio_init(LED_LOP_PIN); gpio_set_dir(LED_LOP_PIN, GPIO_OUT); gpio_put(LED_LOP_PIN, 0); // Initiera LED-pinnar som utgångar
  gpio_init(LED_RWD_R_PIN); gpio_set_dir(LED_RWD_R_PIN, GPIO_OUT); gpio_put(LED_RWD_R_PIN, 0); // Initiera LED-pinnar som utgångar
  gpio_init(LED_RWD_G_PIN); gpio_set_dir(LED_RWD_G_PIN, GPIO_OUT); gpio_put(LED_RWD_G_PIN, 0); // Initiera LED-pinnar som utgångar
  gpio_init(LED_RWD_B_PIN); gpio_set_dir(LED_RWD_B_PIN, GPIO_OUT); gpio_put(LED_RWD_B_PIN, 0); // Initiera LED-pinnar som utgångar
  gpio_init(LED_FWD_R_PIN); gpio_set_dir(LED_FWD_R_PIN, GPIO_OUT); gpio_put(LED_FWD_R_PIN, 0); // Initiera LED-pinnar som utgångar
  gpio_init(LED_FWD_G_PIN); gpio_set_dir(LED_FWD_G_PIN, GPIO_OUT); gpio_put(LED_FWD_G_PIN, 0); // Initiera LED-pinnar som utgångar
  gpio_init(LED_FWD_B_PIN); gpio_set_dir(LED_FWD_B_PIN, GPIO_OUT); gpio_put(LED_FWD_B_PIN, 0); // Initiera LED-pinnar som utgångar
  gpio_init(LED_KLK_PIN); gpio_set_dir(LED_KLK_PIN, GPIO_OUT);  gpio_put(LED_KLK_PIN, 0); // Initiera LED-pinnar som utgångar

  gpio_set_function(LED_RWD_R_PIN, GPIO_FUNC_PWM); // Aktivera PWM för RWD_R_PIN
  gpio_set_function(LED_RWD_G_PIN, GPIO_FUNC_PWM); // Aktivera PWM för RWD_G_PIN
  gpio_set_function(LED_RWD_B_PIN, GPIO_FUNC_PWM); // Aktivera PWM för RWD_B_PIN
  uint slice_rwd_r = pwm_gpio_to_slice_num(LED_RWD_R_PIN); // Hämta slice för RWD_R_PIN
  uint slice_rwd_g = pwm_gpio_to_slice_num(LED_RWD_G_PIN); // Hämta slice för RWD_G_PIN
  uint slice_rwd_b = pwm_gpio_to_slice_num(LED_RWD_B_PIN); // Hämta slice för RWD_B_PIN
  pwm_set_enabled(slice_rwd_r, true); // Aktivera PWM för RWD_R_PIN
  pwm_set_enabled(slice_rwd_g, true); // Aktivera PWM för RWD_G_PIN
  pwm_set_enabled(slice_rwd_b, true); // Aktivera PWM för RWD_B_PIN
  gpio_set_function(LED_FWD_R_PIN, GPIO_FUNC_PWM); // Aktivera PWM för FWD_R_PIN
  gpio_set_function(LED_FWD_G_PIN, GPIO_FUNC_PWM); // Aktivera PWM för FWD_G_PIN
  gpio_set_function(LED_FWD_B_PIN, GPIO_FUNC_PWM); // Aktivera PWM för FWD_B_PIN
  uint slice_fwd_r = pwm_gpio_to_slice_num(LED_FWD_R_PIN); // Hämta slice för FWD_R_PIN
  uint slice_fwd_g = pwm_gpio_to_slice_num(LED_FWD_G_PIN); // Hämta slice för FWD_G_PIN
  uint slice_fwd_b = pwm_gpio_to_slice_num(LED_FWD_B_PIN); // Hämta slice för FWD_B_PIN
  pwm_set_enabled(slice_fwd_r, true); // Aktivera PWM för FWD_R_PIN
  pwm_set_enabled(slice_fwd_g, true); // Aktivera PWM för FWD_G_PIN
  pwm_set_enabled(slice_fwd_b, true); // Aktivera PWM för FWD_B_PIN
  gpio_set_function(LED_KLK_PIN, GPIO_FUNC_PWM); // Aktivera PWM för LED_KLK_PIN
  uint slice_klk = pwm_gpio_to_slice_num(LED_KLK_PIN); // Hämta slice för LED_KLK_PIN
  pwm_set_enabled(slice_klk, true); // Aktivera PWM för LED_KLK_PIN
  gpio_set_function(LED_LOP_PIN, GPIO_FUNC_PWM); // Aktivera PWM för LED_LOP_PIN
  uint slice_lop = pwm_gpio_to_slice_num(LED_LOP_PIN); // Hämta slice för LED_LOP_PIN
  pwm_set_enabled(slice_lop, true); // Aktivera PWM för LED_LOP_PIN
}

int main() {
  setup();
  while (1) {
    
    kolla_klockan(); // Uppdatera nuvarande tid
    tud_task();   // TinyUSB, gör din grej!
    read_midi(); // Got midi?
    update_buttons(); // Push it!
    update_leds(); // Let there be!

    // Extra: säkerställ att watchdog eller sleep inte stoppar PWM
    sleep_ms(1); // Om du vill sänka CPU-belastning

  }
  return 0;
}

// Dummy-implementationer för TinyUSB MIDI-symboler när bara en stream används
// extern "C" uint32_t tud_midi_n_stream_write(uint8_t instance, uint8_t cable_num, const uint8_t* buffer, uint32_t bufsize) {
//   (void)instance;
//   return tud_midi_stream_write(cable_num, buffer, bufsize);
// }
// extern "C" uint32_t tud_midi_n_available(uint8_t instance, uint8_t cable_num) {
//   (void)instance; (void)cable_num;
//   return 0;
// }
// extern "C" uint32_t tud_midi_n_stream_read(uint8_t instance, uint8_t cable_num, void* buffer, uint32_t bufsize) {
//   (void)instance; (void)cable_num; (void)buffer; (void)bufsize;
//   return 0;
// }


