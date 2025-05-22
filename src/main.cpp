#include "bsp/board.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "tusb.h"
#include "tusb_config.h"
#include <math.h> // För sinf()


// ██████████████████████████████████████████████████████████████████████████████████████████████████

#define BTN_LOP_PIN   2     // Definierad pinne för knapp, Loop
#define BTN_CUE_PIN   3     // Definierad pinne för knapp, Cue
#define BTN_BAR_PIN   4     // Definierad pinne för knapp, Bar
#define BTN_BIT_PIN   5     // Definierad pinne för knapp, Beat
#define BTN_RWD_PIN   7     // Definierad pinne för knapp, Rewind
#define BTN_FWD_PIN   6     // Definierad pinne för knapp, Fast-Forward
#define LED_LOP_PIN   15    // Definierad pinne för LED, Loop
#define LED_RWD_R_PIN 11    // Definierad pinne för LED, Rewind Röd
#define LED_RWD_G_PIN 13    // Definierad pinne för LED, Rewind Grön
#define LED_RWD_B_PIN 12    // Definierad pinne för LED, Rewind Blå
#define LED_FWD_R_PIN 8     // Definierad pinne för LED, Fast-Forward Röd
#define LED_FWD_G_PIN 9     // Definierad pinne för LED, Fast-Forward Grön
#define LED_FWD_B_PIN 10    // Definierad pinne för LED, Fast-Forward Blå
#define LED_KLK_PIN   14    // Definierad pinne för LED, Midi-In-Blink-Klockan
// #define LED_CUE_PIN   8     // Definierad pinne för LED, Cue
// #define LED_BAR_PIN   9     // Definierad pinne för LED, Bar
// #define LED_BIT_PIN   10    // Definierad pinne för LED, Beat

#define DEBOUNCE 30  // Definiera debouncetid i millisekunder


// ██████████████████████████████████████████████████████████████████████████████████████████████████

// Global tid:
uint32_t now = 0;  // Variabel för att hålla reda på tiden

// Loopstatus
bool AllInfinito = false;  // Bool för att hålla reda på om LOOP LED är på eller av

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

const uint8_t rwd[3] = { rwdCue, rwdBar, rwdBit }; // Array för att hålla reda på RWD-noter
const uint8_t fwd[3] = { fwdCue, fwdBar, fwdBit }; // Array för att hålla reda på FWD-noter

// MIDI-CCs
uint8_t loopCC = 100;  // MIDI-CC för Loop

// MIDI Klockan
volatile uint32_t prvMidiKl = 0; // Tidpunkt för senaste/"previous" MIDI-klocka
volatile bool ledMidiKl = false; // Bool för att hålla reda på om MIDI-klockan är aktiv
volatile uint32_t midi_tick_count = 0;
volatile uint32_t last_beat_time = 0;
volatile float bpm = 120.0f; // BPM Startvärde

volatile bool midi_clock_active = false;
volatile uint32_t last_midi_clock = 0;


// ██████████████████████████████████████████████████████████████████████████████████████████████████

typedef struct {
  bool     stt; // Knappens tillstånd (tryckt eller inte)
  uint32_t prv; // Tidpunkt för senaste knapptryckning, de-bounce timer. "Previous value" eller "last_pressed time"...
  uint     pin; // Pinne som knappen är kopplad till
} btn;

btn btnLop = { false, 0, BTN_LOP_PIN }; // Knappobjekt. State, last/previous value och pin definieras.
btn btnCue = { false, 0, BTN_CUE_PIN }; // Knappobjekt. State, last/previous value och pin definieras.
btn btnBar = { false, 0, BTN_BAR_PIN }; // Knappobjekt. State, last/previous value och pin definieras.
btn btnBit = { false, 0, BTN_BIT_PIN }; // Knappobjekt. State, last/previous value och pin definieras.
btn btnRwd = { false, 0, BTN_RWD_PIN }; // Knappobjekt. State, last/previous value och pin definieras.
btn btnFwd = { false, 0, BTN_FWD_PIN }; // Knappobjekt. State, last/previous value och pin definieras.

btn* btnDur[3] = { &btnCue, &btnBar, &btnBit }; // Radio-knappar: CUE, BAR, BIT

const uint8_t CUE = 0, BAR = 1, BIT = 2, NON = 127; // Definiera knapparna som CUE, BAR, BIT och ingen aktiv knapp
uint8_t durata = NON; // Definiera durata och initialisera den till ingen aktiv knapp

uint32_t timCue = 0, timBar = 0, timBit = 0; // Timers för knapparna: CUE, BAR, BIT
uint32_t* timDur[3] = { &timCue, &timBar, &timBit }; // Array för att hålla reda på timers för knapparna: CUE, BAR, BIT



// ██████████████████████████████████████████████████████████████████████████████████████████████████

bool pressed(btn *btn) {
  if (!gpio_get(btn->pin)) { // Kolla om pinnen är låg
    if (!btn->stt && (now - btn->prv > DEBOUNCE)) { // Kolla state och de-bounce
      btn->stt = true; // Sätt state till true
      btn->prv = now; // Ställ om klockan
      return true;
    }
  }
  return false;
}

bool release(btn *btn) {
  if (gpio_get(btn->pin)) { // Kolla ifall pinnen är hög
    if (btn->stt && (now - btn->prv > DEBOUNCE)) { // Kolla state false och de-bounce-tid
      btn->stt = false; // Sätt state till false
      btn->prv = now; // Ställ om klockan
      return true;
    }
  }
  return false;
}

bool isnArr(const uint8_t* arr, size_t len, uint8_t val) {
    for (size_t i = 0; i < len; ++i)
        if (arr[i] == val) return true;
    return false;
}


// ██████████████████████████████████████████████████████████████████████████████████████████████████

void read_midi() { // Läs av inkommande MIDI-meddelande
  while (tud_midi_available()) { // Kolla om det finns något att läsa med hjälp av "Tiny USB"
    uint8_t midi[4]; // Skapa en array för att lagra MIDI-meddelande
    uint32_t bytes_read = tud_midi_stream_read(midi, sizeof(midi)); // Läs av inkommande MIDI med hjäälp av "Tiny USB"
    for (uint32_t i = 0; i + 2 < bytes_read; i++) { // Loop för att läsa MIDI-meddelande
      
      // MIDIKLOCKA ██████████████
      if (midi[i] == 0xF8) { // Om inkommande MIDI är klocka
        midi_tick_count++;
        midi_clock_active = true;
        last_midi_clock = now;
        // ledMidiKl = true; // Sätt prvMidiKl till nuvarande tid
      }

      // LOOP ████████████████████
      if ((midi[i] & 0xF0) == 0xB0 && midi[i+1] == 100) { // Om inkommande MIDI är CC och nummret är "100"
        if (midi[i+2] > 63 && !AllInfinito) { // Om värdet är över "63" och AllInfinito är false
          uint8_t midi_cc[3] = {0xB0, loopCC, 127}; // Skapa MIDI-meddelande // CC + CH, CC-nummer, värde
          tud_midi_stream_write(0, midi_cc, 3); // Skicka MIDI
          AllInfinito = !AllInfinito; // Sätt AllInfinito till true
        } else
        if (midi[i+2] <= 63 && AllInfinito) { // Om värdet är över "63" och AllInfinito är true
          uint8_t midi_cc[3] = {0xB0, loopCC, 0}; // Skapa MIDI-meddelande // CC + Kanal, CC-nummer, värde
          tud_midi_stream_write(0, midi_cc, 3); // Skicka MIDI
          AllInfinito = !AllInfinito; // Sätt AllInfinito till false
        }
      }
    }
  }
}

// ██████████████████████████████████████████████████████████████████████████████████████████████████

void sendMidiNoteOn(uint8_t note, uint8_t velocity = 127) {
  uint8_t note_on[3] = {0x90, note, velocity}; // Skapa MIDI-meddelande // Note On, Note, Velocity
  tud_midi_stream_write(0, note_on, 3); // Skicka MIDI
  if (isnArr(rwd, 3, note) && !Ancora) Ancora = true; // Sätt Ancora till true om MIDI-noten är i RWD-arrayen
  else if (isnArr(fwd, 3, note) && !Avanti) Avanti = true; // Sätt Avanti till true om MIDI-noten är i FWD-arrayen
}

void sendMidiNoteOf(uint8_t note) {
  uint8_t midi[3] = {0x80, note, 0};  // Skapa MIDI-meddelande // Note Off, Note, Velocity
  tud_midi_stream_write(0, midi, 3);  // Skicka MIDI
  if (isnArr(rwd, 3, note) && Ancora) Ancora = false; // Sätt Ancora till false om MIDI-noten vi skickar är i RWD-arrayen
  else if (isnArr(fwd, 3, note) && Avanti) Avanti = false; // Sätt Avanti till false om MIDI-noten vi skickar är i FWD-arrayen
}


// ██████████████████████████████████████████████████████████████████████████████████████████████████



void update_loop() {// Update funktion
  gpio_put(LED_LOP_PIN, AllInfinito);  // Sätt LED_LOP_PIN till AllInfinito

}

void update_ancora_led() { // Update funktion
  if (Ancora)            { gpio_put(LED_RWD_R_PIN, 1); gpio_put(LED_RWD_G_PIN, 1); gpio_put(LED_RWD_B_PIN, 1); // LED ON
  } else if (btnCue.stt) { gpio_put(LED_RWD_R_PIN, 0); gpio_put(LED_RWD_G_PIN, 1); gpio_put(LED_RWD_B_PIN, 0); // CUE R,G,B
  } else if (btnBar.stt) { gpio_put(LED_RWD_R_PIN, 0); gpio_put(LED_RWD_G_PIN, 0); gpio_put(LED_RWD_B_PIN, 1); // BAR R,G,B
  } else if (btnBit.stt) { gpio_put(LED_RWD_R_PIN, 1); gpio_put(LED_RWD_G_PIN, 0); gpio_put(LED_RWD_B_PIN, 0); // BIT R,G,B
  } else                 { gpio_put(LED_RWD_R_PIN, 0); gpio_put(LED_RWD_G_PIN, 0); gpio_put(LED_RWD_B_PIN, 0); // LED OFF
  }
}

void update_avanti_led() { // Update funktion
  if (Avanti)            { gpio_put(LED_FWD_R_PIN, 1); gpio_put(LED_FWD_G_PIN, 1); gpio_put(LED_FWD_B_PIN, 1); // LED ON
  } else if (btnCue.stt) { gpio_put(LED_FWD_R_PIN, 0); gpio_put(LED_FWD_G_PIN, 1); gpio_put(LED_FWD_B_PIN, 0); // CUE R,G,B
  } else if (btnBar.stt) { gpio_put(LED_FWD_R_PIN, 0); gpio_put(LED_FWD_G_PIN, 0); gpio_put(LED_FWD_B_PIN, 1); // BAR R,G,B
  } else if (btnBit.stt) { gpio_put(LED_FWD_R_PIN, 1); gpio_put(LED_FWD_G_PIN, 0); gpio_put(LED_FWD_B_PIN, 0); // BIT R,G,B
  } else                 { gpio_put(LED_FWD_R_PIN, 0); gpio_put(LED_FWD_G_PIN, 0); gpio_put(LED_FWD_B_PIN, 0); // LED OFF
  }
}

void update_bpm_led() {

  static float ledBpmFas = 0.0f; // Skapa variabel för fas
  static uint32_t ledBpmPrv = 0; // Skapa variabel för senaste MIDI-klocka
  static float fadeVal = 0.0f; // Håller nuvarande ljusstyrka

  if (now - ledBpmPrv < 5) return; // Om tiden sedan senaste MIDI-klocka är mindre än 5ms, gör inget
  ledBpmPrv = now; // Sätt senaste MIDI-klocka till nuvarande tid

  float ledBpmTim = 60000.0f / bpm; // Beräkna tiden för en cykel i ms
  ledBpmFas += 5.0f / ledBpmTim; // Öka fasen med 5ms / tiden för en cykel
  if (ledBpmFas >= 1.0f) ledBpmFas -= 1.0f; // Om fasen är större än 1, sätt den till 0

  float target = 0.0f; // Mål för fade

  if (ledBpmFas < 0.5f) target = cosf(ledBpmFas * 3.1415926f); // Första halvan: fade enligt cosinuskurva (100% till 0%)
  else { // Andra halvan:
    if (midi_clock_active && (now - last_midi_clock <= 200)) target = 0.25f * sinf((ledBpmFas - 0.5f) * 3.1415926f); // Om vi får MIDI-klocka: fade till 25%
    else target = 0.0f; // Om ingen MIDI-klocka: fade mjukt ner mot 0%
  }

  static float prev_phase = 0.0f; // Skapa variabel för föregående fas
  if (ledBpmFas < prev_phase) target = 1.0f; // Om vi är i en ny cykel, sätt target till 100%
  prev_phase = ledBpmFas; // Spara föregående fas

  fadeVal += (target - fadeVal) * 0.05f; // Mjuk fade mot target // Justera fade-hastigheten med värdet 0.05f om du vill ha snabbare eller långsammare avtoning!

  if (fadeVal < 0.0f) fadeVal = 0.0f; // Se till att fadeVal aldrig går under 0
  if (fadeVal > 1.0f) fadeVal = 1.0f; // Se till att fadeVal aldrig går över 1

  uint16_t ledBpmPwm = (uint16_t)(fadeVal * 255); // Beräkna PWM-värde
  pwm_set_gpio_level(LED_KLK_PIN, ledBpmPwm); // Sätt PWM-värde på LED_KLK_PIN
}

void update_leds() { // Update funktion
  update_ancora_led();
  update_avanti_led();
  update_bpm_led();
  update_loop();
}

void update_buttons() {
  
  if (pressed(&btnLop)) { // LOOP ████████████████████
    uint8_t value = AllInfinito ? 0 : 127;
    uint8_t midi_cc[3] = {0xB0, loopCC, value};
    tud_midi_stream_write(0, midi_cc, 3);
    AllInfinito = !AllInfinito;
  }

  

  for (int i = 0; i < 3; ++i) { // DURATION ████████████████
    if (pressed(btnDur[i])) { // om någon knapp blivit nedtryckt
      if (!Ancora && btnRwd.stt) sendMidiNoteOn(rwdCue + i);
      if (!Avanti && btnFwd.stt) sendMidiNoteOn(fwdCue + i);
      *(timDur[i]) = now; // ställ in tiden på den tryckta knappen!
    } else if (release(btnDur[i])) { // kolla om någon knapp släppts
      if (Ancora && btnRwd.stt) sendMidiNoteOf(rwdCue + i); // skicka note-off
      if (Avanti && btnFwd.stt) sendMidiNoteOf(fwdCue + i); // skicka note-off
      *(timDur[i]) = 0;
    }
  }

  uint32_t timMax = 0; // Variabel för att hålla reda på den maximala tiden
  for (int i = 0; i < 3; ++i) { // Loop för att kolla vilken knapp som är nedtryckt
    if (*(timDur[i]) > timMax) { // kolla om tiden är större än den maximala tiden
      timMax = *(timDur[i]); // Ställ in den större tiden som ny max-tid!
      durata = i; // 0 = CUE, 1 = BAR, 2 = BIT
    }
  }
  if (timMax == 0) durata = NON; // om ingen knapp är nedtryckt, sätt durata till NON
  
  // RWD  ████████████████████
  if (pressed(&btnRwd) && durata != NON && !Ancora && btnDur[durata]->stt) sendMidiNoteOn(rwd[durata]); // Kolla RWD-knappen och Skicka MIDI
  else if (release(&btnRwd) && durata != NON && Ancora && btnDur[durata]->stt) sendMidiNoteOf(rwd[durata]); // Kolla RWD-knappen och Skicka MIDI

  // FWD  ████████████████████
  if (pressed(&btnFwd) && durata != NON && !Avanti && btnDur[durata]->stt) sendMidiNoteOn(fwd[durata]); // Kolla FWD-knappen och Skicka MIDI
  else if (release(&btnFwd) && durata != NON && Avanti && btnDur[durata]->stt) sendMidiNoteOf(fwd[durata]); // Kolla FWD-knappen och Skicka MIDI
}


// ██████████████████████████████████████████████████████████████████████████████████████████████████

void setup() {
  board_init(); // Initiera GPIO
  tusb_init(); // Initiera Tiny USB
  gpio_init(BTN_LOP_PIN); gpio_set_dir(BTN_LOP_PIN, GPIO_IN); gpio_pull_up(BTN_LOP_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_CUE_PIN); gpio_set_dir(BTN_CUE_PIN, GPIO_IN); gpio_pull_up(BTN_CUE_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_BAR_PIN); gpio_set_dir(BTN_BAR_PIN, GPIO_IN); gpio_pull_up(BTN_BAR_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_BIT_PIN); gpio_set_dir(BTN_BIT_PIN, GPIO_IN); gpio_pull_up(BTN_BIT_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_FWD_PIN); gpio_set_dir(BTN_FWD_PIN, GPIO_IN); gpio_pull_up(BTN_FWD_PIN); // Konfigurera knappar som ingångar
  gpio_init(BTN_RWD_PIN); gpio_set_dir(BTN_RWD_PIN, GPIO_IN); gpio_pull_up(BTN_RWD_PIN); // Konfigurera knappar som ingångar
  gpio_init(LED_LOP_PIN); gpio_set_dir(LED_LOP_PIN, GPIO_OUT); gpio_put(LED_LOP_PIN, 0); // Konfigurera LED som utgång
  gpio_init(LED_RWD_R_PIN); gpio_set_dir(LED_RWD_R_PIN, GPIO_OUT); gpio_put(LED_RWD_R_PIN, 0); // Konfigurera LED som utgång
  gpio_init(LED_RWD_G_PIN); gpio_set_dir(LED_RWD_G_PIN, GPIO_OUT); gpio_put(LED_RWD_G_PIN, 0); // Konfigurera LED som utgång
  gpio_init(LED_RWD_B_PIN); gpio_set_dir(LED_RWD_B_PIN, GPIO_OUT); gpio_put(LED_RWD_B_PIN, 0); // Konfigurera LED som utgång
  gpio_init(LED_FWD_R_PIN); gpio_set_dir(LED_FWD_R_PIN, GPIO_OUT); gpio_put(LED_FWD_R_PIN, 0); // Konfigurera LED som utgång
  gpio_init(LED_FWD_G_PIN); gpio_set_dir(LED_FWD_G_PIN, GPIO_OUT); gpio_put(LED_FWD_G_PIN, 0); // Konfigurera LED som utgång
  gpio_init(LED_FWD_B_PIN); gpio_set_dir(LED_FWD_B_PIN, GPIO_OUT); gpio_put(LED_FWD_B_PIN, 0); // Konfigurera LED som utgång
  gpio_init(LED_KLK_PIN); gpio_set_dir(LED_KLK_PIN, GPIO_OUT);  gpio_put(LED_KLK_PIN, 0); // Konfigurera LED som utgång
  gpio_set_function(LED_KLK_PIN, GPIO_FUNC_PWM); // Sätt LED_KLK_PIN till PWM-funktion
  uint slice = pwm_gpio_to_slice_num(LED_KLK_PIN); 
  pwm_set_enabled(slice, true);
}


// ██████████████████████████████████████████████████████████████████████████████████████████████████

int main() {
  setup();
  while (1) {
    tud_task();   // TinyUSB, gör din grej!
    now = to_ms_since_boot(get_absolute_time()); // Kolla klockan
    update_buttons(); // Uppdateringar
    update_leds(); // Uppdateringar
    read_midi(); // Uppdateringar
  }
  return 0;
}
