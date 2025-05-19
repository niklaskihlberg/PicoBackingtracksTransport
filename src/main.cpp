#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "tusb_config.h"

// Definiera pinnar för knapparna och LED:en
#define BTN_LOP_PIN 2
#define BTN_CUE_PIN 3
#define BTN_BAR_PIN 4
#define BTN_BIT_PIN 5
#define BTN_RWD_PIN 7
#define BTN_FWD_PIN 6
#define LED_LOP_PIN 15

#define DEBOUNCE_MS 30

bool LoopLED = false;

bool Button_Lop_Pressed = false;
bool Button_Cue_Pressed = false;
bool Button_Bar_Pressed = false;
bool Button_Bit_Pressed = false;
bool Button_Rwd_Pressed = false;
bool Button_Fwd_Pressed = false;

bool Cue_Rwd_State = false;
bool Bar_Rwd_State = false;
bool Bit_Rwd_State = false;
bool Cue_Fwd_State = false;
bool Bar_Fwd_State = false;
bool Bit_Fwd_State = false;


uint8_t Cue_Rwd_Note = 1;
uint8_t Bar_Rwd_Note = 2;
uint8_t Bit_Rwd_Note = 3;
uint8_t Cue_Fwd_Note = 4;
uint8_t Bar_Fwd_Note = 5;
uint8_t Bit_Fwd_Note = 6;

uint8_t Lop_CC = 100;

uint32_t DEBNC_Lop = 0;
uint32_t DEBNC_Cue = 0;
uint32_t DEBNC_Bar = 0;
uint32_t DEBNC_Bit = 0;
uint32_t DEBNC_Rwd = 0;
uint32_t DEBNC_Fwd = 0;

void check_noteOff(bool &active_state, uint8_t note) {
    if (active_state) {
        uint8_t midi[3] = {0x80, note, 0}; // Skapa MIDI-meddelande // Note Off, Note, Velocity
        tud_midi_stream_write(0, midi, 3); // Skicka MIDI
        active_state = false; // Återställ tillståndet
    }
}

void check_buttons() {

    // RWD
    if (Button_Cue_Pressed && Button_Rwd_Pressed && !Cue_Rwd_State) {
        uint8_t note_on[3] = {0x90, Cue_Rwd_Note, 127}; // Note On, Note, Velocity
        tud_midi_stream_write(0, note_on, 3);
        Cue_Rwd_State = true;
    }

    if (Button_Bar_Pressed && Button_Rwd_Pressed && !Bar_Rwd_State) {
        uint8_t note_on[3] = {0x90, Bar_Rwd_Note, 127}; // Note On, Note, Velocity
        tud_midi_stream_write(0, note_on, 3);
        Bar_Rwd_State = true;
    }

    if (Button_Bit_Pressed && Button_Rwd_Pressed && !Bit_Rwd_State) {
        uint8_t note_on[3] = {0x90, Bit_Rwd_Note, 127}; // Note On, Note, Velocity
        tud_midi_stream_write(0, note_on, 3);
        Bit_Rwd_State = true;
    }

    // FWD
    if (Button_Cue_Pressed && Button_Fwd_Pressed && !Cue_Fwd_State) {
        uint8_t note_on[3] = {0x90, Cue_Fwd_Note, 127}; // Note On, Note, Velocity
        tud_midi_stream_write(0, note_on, 3);
        Cue_Fwd_State = true;
    }

    if (Button_Bar_Pressed && Button_Fwd_Pressed && !Bar_Fwd_State) {
        uint8_t note_on[3] = {0x90, Bar_Fwd_Note, 127}; // Note On, Note, Velocity
        tud_midi_stream_write(0, note_on, 3);
        Bar_Fwd_State = true;
    }

    if (Button_Bit_Pressed && Button_Fwd_Pressed && !Bit_Fwd_State) {
        uint8_t note_on[3] = {0x90, Bit_Fwd_Note, 127}; // Note On, Note, Velocity
        tud_midi_stream_write(0, note_on, 3);
        Bit_Fwd_State = true;
    }


}

void setup() {
  board_init();
  tusb_init();

  // Konfigurera knappar som ingångar
  gpio_init(BTN_LOP_PIN);
  gpio_set_dir(BTN_LOP_PIN, GPIO_IN);
  gpio_pull_up(BTN_LOP_PIN);

  gpio_init(BTN_CUE_PIN);
  gpio_set_dir(BTN_CUE_PIN, GPIO_IN);
  gpio_pull_up(BTN_CUE_PIN);

  gpio_init(BTN_BAR_PIN);
  gpio_set_dir(BTN_BAR_PIN, GPIO_IN);
  gpio_pull_up(BTN_BAR_PIN);

  gpio_init(BTN_BIT_PIN);
  gpio_set_dir(BTN_BIT_PIN, GPIO_IN);
  gpio_pull_up(BTN_BIT_PIN);

  gpio_init(BTN_FWD_PIN);
  gpio_set_dir(BTN_FWD_PIN, GPIO_IN);
  gpio_pull_up(BTN_FWD_PIN);

  gpio_init(BTN_RWD_PIN);
  gpio_set_dir(BTN_RWD_PIN, GPIO_IN);
  gpio_pull_up(BTN_RWD_PIN);

  // Konfigurera LED som utgång
  gpio_init(LED_LOP_PIN);
  gpio_set_dir(LED_LOP_PIN, GPIO_OUT);
  gpio_put(LED_LOP_PIN, 0);
}

void loop() {
    tud_task(); // Hantera USB-tasks

    if (tud_midi_available()) {
        uint8_t midi[4];
        uint32_t bytes_read = tud_midi_stream_read(midi, sizeof(midi));
        if (bytes_read > 0) {
            if ((midi[0] & 0xF0) == 0xB0 && midi[1] == 64) {
                if (midi[2] > 0) LoopLED = true;
                else LoopLED = false;
                gpio_put(LED_LOP_PIN, LoopLED);
            }
        }
    }

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // LOOP BUTTON
    if (gpio_get(BTN_LOP_PIN)) {
        if (!Button_Lop_Pressed && (now - DEBNC_Lop > DEBOUNCE_MS)) {
            Button_Lop_Pressed = true;
            DEBNC_Lop = now;
            uint8_t Lop_Val = LoopLED ? 127 : 0;
            uint8_t midi[3] = {0xB0, Lop_CC, Lop_Val};
            tud_midi_stream_write(0, midi, 3);
            LoopLED = !LoopLED;
            gpio_put(LED_LOP_PIN, LoopLED);
        }
    } else if (Button_Lop_Pressed && (now - DEBNC_Lop > DEBOUNCE_MS)) {
        Button_Lop_Pressed = false;
        DEBNC_Lop = now;
    }

    // CUE
    if (gpio_get(BTN_CUE_PIN)) {
        if (!Button_Cue_Pressed && (now - DEBNC_Cue > DEBOUNCE_MS)) {
            Button_Cue_Pressed = true;
            DEBNC_Cue = now;
            check_buttons();
        }
    } else if (Button_Cue_Pressed && (now - DEBNC_Cue > DEBOUNCE_MS)) {
        Button_Cue_Pressed = false;
        DEBNC_Cue = now;
        check_noteOff(Cue_Rwd_State, Cue_Rwd_Note);
        check_noteOff(Cue_Fwd_State, Cue_Fwd_Note);
    }

    // BAR
    if (gpio_get(BTN_BAR_PIN)) {
        if (!Button_Bar_Pressed && (now - DEBNC_Bar > DEBOUNCE_MS)) {
            Button_Bar_Pressed = true;
            DEBNC_Bar = now;
            check_buttons();
        }
    } else if (Button_Bar_Pressed && (now - DEBNC_Bar > DEBOUNCE_MS)) {
        Button_Bar_Pressed = false;
        DEBNC_Bar = now;
        check_noteOff(Bar_Rwd_State, Bar_Rwd_Note);
        check_noteOff(Bar_Fwd_State, Bar_Fwd_Note);
    }

    // BIT
    if (gpio_get(BTN_BIT_PIN)) {
        if (!Button_Bit_Pressed && (now - DEBNC_Bit > DEBOUNCE_MS)) {
            Button_Bit_Pressed = true;
            DEBNC_Bit = now;
            check_buttons();
        }
    } else if (Button_Bit_Pressed && (now - DEBNC_Bit > DEBOUNCE_MS)) {
        Button_Bit_Pressed = false;
        DEBNC_Bit = now;
        check_noteOff(Bit_Rwd_State, Bit_Rwd_Note);
        check_noteOff(Bit_Fwd_State, Bit_Fwd_Note);
    }

    // RWD
    if (gpio_get(BTN_FWD_PIN)) {
        if (!Button_Rwd_Pressed && (now - DEBNC_Rwd > DEBOUNCE_MS)) {
            Button_Rwd_Pressed = true;
            DEBNC_Rwd = now;
            check_buttons();
        }
    } else if (Button_Rwd_Pressed && (now - DEBNC_Rwd > DEBOUNCE_MS)) {
        Button_Rwd_Pressed = false;
        DEBNC_Rwd = now;
        check_noteOff(Cue_Rwd_State, Cue_Rwd_Note);
        check_noteOff(Bar_Rwd_State, Bar_Rwd_Note);
        check_noteOff(Bit_Rwd_State, Bit_Rwd_Note);
    }

    // FWD
    if (gpio_get(BTN_RWD_PIN)) {
        if (!Button_Fwd_Pressed && (now - DEBNC_Fwd > DEBOUNCE_MS)) {
            Button_Fwd_Pressed = true;
            DEBNC_Fwd = now;
            check_buttons();
        }
    } else if (Button_Fwd_Pressed && (now - DEBNC_Fwd > DEBOUNCE_MS)) {
        Button_Fwd_Pressed = false;
        DEBNC_Fwd = now;
        check_noteOff(Cue_Fwd_State, Cue_Fwd_Note);
        check_noteOff(Bar_Fwd_State, Bar_Fwd_Note);
        check_noteOff(Bit_Fwd_State, Bit_Fwd_Note);
    }
}

int main() {
  setup();
  while (1) {
    loop();
  }
  return 0;
}
