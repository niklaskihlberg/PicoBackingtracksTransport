#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "tusb_config.h"

// Definiera pinnar för knapparna och LED:en
#define BTN_LOP_PIN 2
#define BTN_CUE_PIN 3
#define BTN_BAR_PIN 4
#define BTN_BIT_PIN 5
#define BTN_FWD_PIN 6
#define BTN_RWD_PIN 7
#define LED_LOP_PIN 15

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

    if (tud_midi_available()) { // Hantera inkommande MIDI-meddelanden
        uint8_t midi[4]; // Skapa en buffert för att lagra inkommande MIDI-meddelanden
        uint32_t bytes_read = tud_midi_stream_read(midi, sizeof(midi)); // Läs inkommande MIDI-meddelanden
        if (bytes_read > 0) { // Om det finns inkommande MIDI-meddelanden
            if ((midi[0] & 0xF0) == 0xB0 && midi[1] == 64) { // Kontrollera om det är ett Control Change-meddelande för att styra Loop-LED:en // CC-meddelande för Loop-LED:en
                if (midi[2] > 0) LoopLED = true; // Om värdet är > 0, sätt på Loop-LED:en
                else LoopLED = false; // Annars, stäng av Loop-LED:en
                gpio_put(LED_LOP_PIN, LoopLED); // Sätt LED:ens tillstånd
            }
        }
    }

    // LOOP BUTTON
    if (!gpio_get(BTN_LOP_PIN)) { // Om knappen trycks ned
        if (!Button_Lop_Pressed) { // Skicka MIDI CC-meddelande baserat på LED:ens tillstånd
            Button_Lop_Pressed = true; // Ändra tillståndet för knappen
            uint8_t Lop_Val = LoopLED ? 127 : 0; // Om Loop-LED:en är på, sätt värdet till 127, annars 0
            uint8_t midi[3] = {0xB0, Lop_CC, Lop_Val}; // Skapa MIDI-meddelande // CC, Number, Value
            tud_midi_stream_write(0, midi, 3); // Skicka MIDI
            LoopLED = !LoopLED; // Växla LED-tillstånd
            gpio_put(LED_LOP_PIN, LoopLED); // Växla pinnens tillstånd
        }
    }
    else if (Button_Lop_Pressed) Button_Lop_Pressed = false; // Om knappen släpps, återställ tillståndet


    // CUE
    if (!gpio_get(BTN_CUE_PIN)) if (!Button_Cue_Pressed) { // Om knappen trycks ned
        Button_Cue_Pressed = true; // Ändra tillståndet för knappen
        check_buttons(); // Kolla om någon midi skall skickas
    }
    else if (Button_Cue_Pressed) { // Om knappen släpps
        Button_Cue_Pressed = false; // Ändra tillståndet för knappen
        check_noteOff(Cue_Rwd_State, Cue_Rwd_Note); // Kolla om någon midi skall skickas
        check_noteOff(Cue_Fwd_State, Cue_Fwd_Note); // Kolla om någon midi skall skickas
    }
    
    // BAR
    if (!gpio_get(BTN_BAR_PIN)) if (!Button_Bar_Pressed) {
        Button_Bar_Pressed = true;
        check_buttons();
    }
    else if (Button_Bar_Pressed) {   
        Button_Bar_Pressed = false;
        check_noteOff(Bar_Rwd_State, Bar_Rwd_Note);
        check_noteOff(Bar_Fwd_State, Bar_Fwd_Note);
    }

    // BIT
    if (!gpio_get(BTN_BIT_PIN)) if (!Button_Bit_Pressed) {
        Button_Bit_Pressed = true;
        check_buttons();
    }
    else if (Button_Bit_Pressed) {   
        Button_Bit_Pressed = false;
        check_noteOff(Bit_Rwd_State, Bit_Rwd_Note);
        check_noteOff(Bit_Fwd_State, Bit_Fwd_Note);
    }


    // RWD
    if (!gpio_get(BTN_FWD_PIN)) {
        if (!Button_Rwd_Pressed) {
            Button_Rwd_Pressed = true;
            check_buttons();
        }
    }
    else if (Button_Rwd_Pressed) {
        Button_Rwd_Pressed = false;        
        check_noteOff(Cue_Rwd_State, Cue_Rwd_Note);
        check_noteOff(Bar_Rwd_State, Bar_Rwd_Note);
        check_noteOff(Bit_Rwd_State, Bit_Rwd_Note);
    }

    // FWD
    if (!gpio_get(BTN_RWD_PIN)) {
        if (!Button_Fwd_Pressed) {
            Button_Fwd_Pressed = true;
            check_buttons();
        }
    }
    else if (Button_Fwd_Pressed) {
        Button_Fwd_Pressed = false;
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
