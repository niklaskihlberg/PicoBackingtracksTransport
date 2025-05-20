#include "bsp/board.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "tusb_config.h"

#define BTN_LOP_PIN   2     // Definierad pinne för knapp, Loop
#define BTN_CUE_PIN   3     // Definierad pinne för knapp, Cue
#define BTN_BAR_PIN   4     // Definierad pinne för knapp, Bar
#define BTN_BIT_PIN   5     // Definierad pinne för knapp, Beat
#define BTN_RWD_PIN   7     // Definierad pinne för knapp, Rewind
#define BTN_FWD_PIN   6     // Definierad pinne för knapp, Fast-Forward
#define LED_LOP_PIN   15    // Definierad pinne för LED, Loop
#define LED_CUE_PIN   8     // Definierad pinne för LED, Cue
#define LED_BAR_PIN   9     // Definierad pinne för LED, Bar
#define LED_BIT_PIN   10    // Definierad pinne för LED, Beat
#define LED_RWD_R_PIN 11    // Definierad pinne för LED, Rewind Röd
#define LED_RWD_G_PIN 12    // Definierad pinne för LED, Rewind Grön
#define LED_RWD_B_PIN 13    // Definierad pinne för LED, Rewind Blå
#define LED_FWD_R_PIN 0     // Definierad pinne för LED, Fast-Forward Röd
#define LED_FWD_G_PIN 1     // Definierad pinne för LED, Fast-Forward Grön
#define LED_FWD_B_PIN 14    // Definierad pinne för LED, Fast-Forward Blå

#define DEBOUNCE_MS 30  // Definiera debouncetid i millisekunder

typedef struct {
  bool     state;            // Knappens tillstånd (tryckt eller inte)
  uint32_t last_bounce;      // Tidpunkt för senaste knapptryckning, de-bounce timer.
  uint     pin;              // Pinne som knappen är kopplad till
} Button;

Button btnLop = { false, 0, BTN_LOP_PIN, 100 };  // Knappobjekt. State, last_bounce och pin definieras.
Button btnCue = { false, 0, BTN_CUE_PIN, 1 };    // Knappobjekt. State, last_bounce och pin definieras.
Button btnBar = { false, 0, BTN_BAR_PIN, 2 };    // Knappobjekt. State, last_bounce och pin definieras.
Button btnBit = { false, 0, BTN_BIT_PIN, 3 };    // Knappobjekt. State, last_bounce och pin definieras.
Button btnRwd = { false, 0, BTN_FWD_PIN, 4 };    // Knappobjekt. State, last_bounce och pin definieras.
Button btnFwd = { false, 0, BTN_RWD_PIN, 5 };    // Knappobjekt. State, last_bounce och pin definieras.

// ██████████████████████████████████████████████████████████████████████████████████████████████████

bool pressed(Button *btn) {

  uint32_t now = to_ms_since_boot(get_absolute_time()); // Kolla klockan

  if (!gpio_get(btn->pin)) { // Kolla om pinnen är låg
    if (!btn->state && (now - btn->last_bounce > DEBOUNCE_MS)) { // Kolla state och de-bounce
      btn->state = true; // Sätt state till true
      btn->last_bounce = now; // Ställ om klockan
      return true;
    }
  }
  return false;
}

bool released(Button *btn) {

  uint32_t now = to_ms_since_boot(get_absolute_time()); // Kolla klockan

  if (gpio_get(btn->pin)) { // Kolla ifall pinnen är hög
    if (btn->state && (now - btn->last_bounce > DEBOUNCE_MS)) { // Kolla state false och de-bounce-tid
      btn->state = false; // Sätt state till false
      btn->last_bounce = now; // Ställ om klockan
      return true;
    }
  }
  return false;
}

// ██████████████████████████████████████████████████████████████████████████████████████████████████

// Led-statusar
bool AllInfinito = false;  // Bool för att hålla reda på om LOOP LED är på eller av
bool LedCue = false;  // Bool för att hålla reda på om CUE LED är på eller av
bool LedBar = false;  // Bool för att hålla reda på om BAR LED är på eller av
bool LedBit = false;  // Bool för att hålla reda på om BIT LED är på eller av

// Spolningsstatus
bool Ancora = false;  // Bool för att hålla reda på om RWD är aktiv
bool Avanti = false;  // Bool för att hålla reda på om FWD är aktiv

// MIDI-noter
uint8_t rwdCue = 0x01;  // MIDI-not för CUE/RWD
uint8_t rwdBar = 0x02;  // MIDI-not för BAR/RWD
uint8_t rwdBit = 0x03;  // MIDI-not för BIT/RWD
uint8_t fwdCue = 0x04;  // MIDI-not för CUE/FWD
uint8_t fwdBar = 0x05;  // MIDI-not för BAR/FWD
uint8_t fwdBit = 0x06;  // MIDI-not för BIT/FWD


// ██████████████████████████████████████████████████████████████████████████████████████████████████


void update_loop(bool state) {
  gpio_put(LED_LOP_PIN, AllInfinito);  // Sätt LED_LOP_PIN till det angivna tillståndet
}


void sendMidiNoteOn(uint8_t note, uint8_t velocity = 127) {
  uint8_t note_on[3] = {0x90, note, velocity}; // Skapa MIDI-meddelande // Note On, Note, Velocity
  tud_midi_stream_write(0, note_on, 3); // Skicka MIDI
  if ((note == rwdCue || note == rwdBar || note == rwdBit) && !Ancora) Ancora = true; // Sätt Ancora till true vid uppfyllt villkor...
  else if ((note == fwdCue || note == fwdBar || note == fwdBit) && !Ancora) Avanti = true; // Sätt Avanti till true vid uppfyllt villkor...
}

void sendMidiNoteOf(uint8_t note) {
  uint8_t midi[3] = {0x80, note, 0};  // Skapa MIDI-meddelande // Note Off, Note, Velocity
  tud_midi_stream_write(0, midi, 3);  // Skicka MIDI
  if ((note == rwdCue || note == rwdBar || note == rwdBit) && Ancora) Ancora = false; // Sätt Ancora till false vid uppfyllt villkor...
  else if ((note == fwdCue || note == fwdBar || note == fwdBit) && Avanti) Avanti = false; // Sätt Avanti till false vid uppfyllt villkor...
}

void update_buttons() {

  // LOOP BUTTON
  if (pressed(&btnLop)) {
    uint8_t value = AllInfinito ? 0 : 127;
    uint8_t midi_cc[3] = {0xB0, BtnLopNot, value};
    tud_midi_stream_write(0, midi_cc, 3);
    AllInfinito = !AllInfinito;
  }
  
  // CUE
  if (pressed(&btnCue)) {
    if (!Ancora && btnRwd.state) sendMidiNoteOn(rwdCue);
    if (!Avanti && btnFwd.state) sendMidiNoteOn(fwdCue);
  } else
  if (released(&btnCue)) {
    if (Ancora && btnRwd.state) sendMidiNoteOf(rwdCue);
    if (Avanti && btnFwd.state) sendMidiNoteOf(fwdCue);
  }

  // BAR
  if (pressed(&btnBar)) {
    if (!Ancora && btnRwd.state) sendMidiNoteOn(rwdBar);
    if (!Avanti && btnFwd.state) sendMidiNoteOn(fwdBar);
  } else
  if (released(&btnBar)) {
    if (Ancora && btnRwd.state) sendMidiNoteOf(rwdBar);
    if (Avanti && btnFwd.state) sendMidiNoteOf(fwdBar);
  }

  // BIT
  if (pressed(&btnBit)) {
    if (!Ancora && btnRwd.state) sendMidiNoteOn(rwdBit);
    if (!Avanti && btnFwd.state) sendMidiNoteOn(fwdBit);
  } else
  if (released(&btnBit)) {
    if (Ancora && btnRwd.state) sendMidiNoteOf(rwdBit);
    if (Avanti && btnFwd.state) sendMidiNoteOf(fwdBit);
  }

  // RWD
    if (pressed(&btnRwd)) {
        if (!Ancora && btnCue.state) sendMidiNoteOn(rwdCue);
        if (!Ancora && btnBar.state) sendMidiNoteOn(rwdBar);
        if (!Ancora && btnBit.state) sendMidiNoteOn(rwdBit);
    } else
    if (released(&btnRwd)) {
        if (Ancora && btnCue.state) sendMidiNoteOf(rwdCue);
        if (Ancora && btnBar.state) sendMidiNoteOf(rwdBar);
        if (Ancora && btnBit.state) sendMidiNoteOf(rwdBit);
    }
  
    // FWD
    if (pressed(&btnFwd)) {
        if (!Avanti && btnCue.state) sendMidiNoteOn(fwdCue);
        if (!Avanti && btnBar.state) sendMidiNoteOn(fwdBar);
        if (!Avanti && btnBit.state) sendMidiNoteOn(fwdBit);
    } else
    if (released(&btnFwd)) {
        if (Avanti && btnCue.state) sendMidiNoteOf(fwdCue);
        if (Avanti && btnBar.state) sendMidiNoteOf(fwdBar);
        if (Avanti && btnBit.state) sendMidiNoteOf(fwdBit);
    }
}

void update_ancora_led() {
    if (Ancora) {
        gpio_put(LED_RWD_R_PIN, 1);
        gpio_put(LED_RWD_G_PIN, 1);
        gpio_put(LED_RWD_B_PIN, 1);
    } else
    if (btnCue.state) {
        gpio_put(LED_RWD_R_PIN, 0);
        gpio_put(LED_RWD_G_PIN, 1);
        gpio_put(LED_RWD_B_PIN, 0);
    } else
    if (btnBar.state) {
        gpio_put(LED_RWD_R_PIN, 0);
        gpio_put(LED_RWD_G_PIN, 0);
        gpio_put(LED_RWD_B_PIN, 1);
    } else 
    if (btnBit.state) {
        gpio_put(LED_RWD_R_PIN, 1);
        gpio_put(LED_RWD_G_PIN, 0);
        gpio_put(LED_RWD_B_PIN, 0);
    } else
    if (AllInfinito) {
        gpio_put(LED_RWD_R_PIN, 1);
        gpio_put(LED_RWD_G_PIN, 1);
        gpio_put(LED_RWD_B_PIN, 0);
    } else {
        gpio_put(LED_RWD_R_PIN, 0);
        gpio_put(LED_RWD_G_PIN, 0);
        gpio_put(LED_RWD_B_PIN, 0);
    }
}

void update_avanti_led() {
    if (Avanti) {
        gpio_put(LED_FWD_R_PIN, 1);
        gpio_put(LED_FWD_G_PIN, 1);
        gpio_put(LED_FWD_B_PIN, 1);
    } else
    if (btnCue.state) {
        gpio_put(LED_FWD_R_PIN, 0);
        gpio_put(LED_FWD_G_PIN, 1);
        gpio_put(LED_FWD_B_PIN, 0);
    } else
    if (btnBar.state) {
        gpio_put(LED_FWD_R_PIN, 0);
        gpio_put(LED_FWD_G_PIN, 0);
        gpio_put(LED_FWD_B_PIN, 1);
    } else
    if (btnBit.state) {
        gpio_put(LED_FWD_R_PIN, 1);
        gpio_put(LED_FWD_G_PIN, 0);
        gpio_put(LED_FWD_B_PIN, 0);
    } else
    if (AllInfinito) {
        gpio_put(LED_FWD_R_PIN, 1);
        gpio_put(LED_FWD_G_PIN, 1);
        gpio_put(LED_FWD_B_PIN, 0);
    } else {
        gpio_put(LED_FWD_R_PIN, 0);
        gpio_put(LED_FWD_G_PIN, 0);
        gpio_put(LED_FWD_B_PIN, 0);
    }
}

void update_leds() {
    update_ancora_led();
    update_avanti_led();
    update_loop();
}

void read_midi() {
  if (tud_midi_available()) {  // Läs inkommande midi-meddelanden för loop-knappen
    uint8_t midi[4];
    uint32_t bytes_read = tud_midi_stream_read(midi, sizeof(midi));
    if (bytes_read > 0 && (midi[0] & 0xF0) == 0xB0 && midi[1] == 100 && midi[2] > 63) AllInfinito = true;
    else AllInfinito = false;
  }
}


void setup() {

  // Initiera GPIO
  board_init();
  tusb_init();

  // Konfigurera knappar som ingångar
  gpio_init(BTN_LOP_PIN); gpio_set_dir(BTN_LOP_PIN, GPIO_IN); gpio_pull_up(BTN_LOP_PIN);
  gpio_init(BTN_CUE_PIN); gpio_set_dir(BTN_CUE_PIN, GPIO_IN); gpio_pull_up(BTN_CUE_PIN);
  gpio_init(BTN_BAR_PIN); gpio_set_dir(BTN_BAR_PIN, GPIO_IN); gpio_pull_up(BTN_BAR_PIN);
  gpio_init(BTN_BIT_PIN); gpio_set_dir(BTN_BIT_PIN, GPIO_IN); gpio_pull_up(BTN_BIT_PIN);
  gpio_init(BTN_FWD_PIN); gpio_set_dir(BTN_FWD_PIN, GPIO_IN); gpio_pull_up(BTN_FWD_PIN);
  gpio_init(BTN_RWD_PIN); gpio_set_dir(BTN_RWD_PIN, GPIO_IN); gpio_pull_up(BTN_RWD_PIN);

  // Konfigurera LED:er som utgång
  gpio_init(LED_LOP_PIN); gpio_set_dir(LED_LOP_PIN, GPIO_OUT); gpio_put(LED_LOP_PIN, 0);
  gpio_init(LED_CUE_PIN); gpio_set_dir(LED_CUE_PIN, GPIO_OUT); gpio_put(LED_CUE_PIN, 0);
  gpio_init(LED_BAR_PIN); gpio_set_dir(LED_BAR_PIN, GPIO_OUT); gpio_put(LED_BAR_PIN, 0);
  gpio_init(LED_BIT_PIN); gpio_set_dir(LED_BIT_PIN, GPIO_OUT); gpio_put(LED_BIT_PIN, 0);
  gpio_init(LED_RWD_R_PIN); gpio_set_dir(LED_RWD_R_PIN, GPIO_OUT); gpio_put(LED_RWD_R_PIN, 0);
  gpio_init(LED_RWD_G_PIN); gpio_set_dir(LED_RWD_G_PIN, GPIO_OUT); gpio_put(LED_RWD_G_PIN, 0);
  gpio_init(LED_RWD_B_PIN); gpio_set_dir(LED_RWD_B_PIN, GPIO_OUT); gpio_put(LED_RWD_B_PIN, 0);
  gpio_init(LED_FWD_R_PIN); gpio_set_dir(LED_FWD_R_PIN, GPIO_OUT); gpio_put(LED_FWD_R_PIN, 0);
  gpio_init(LED_FWD_G_PIN); gpio_set_dir(LED_FWD_G_PIN, GPIO_OUT); gpio_put(LED_FWD_G_PIN, 0);
  gpio_init(LED_FWD_B_PIN); gpio_set_dir(LED_FWD_B_PIN, GPIO_OUT); gpio_put(LED_FWD_B_PIN, 0);

}

void loop() {
  
  // Initiera TinyUSB
  tud_task();

  // Uppdateringar
  update_buttons();
  update_leds();
  read_midi();

}

int main() {
  setup();
  while (1) {
    loop();
  }
  return 0;
}
