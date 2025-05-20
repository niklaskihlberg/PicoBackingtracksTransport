#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "tusb_config.h"

// Definiera pinnar för knapparna
#define BTN_LOP_PIN 2
#define BTN_CUE_PIN 3
#define BTN_BAR_PIN 4
#define BTN_BIT_PIN 5
#define BTN_RWD_PIN 7
#define BTN_FWD_PIN 6

// Definiera pinnar för LED:arna
#define LED_CUE_PIN 8
#define LED_BAR_PIN 9
#define LED_BIT_PIN 10
#define LED_LOP_PIN 15

// Definiera pinnar för RGB-LED 1
#define LED_RWD_R_PIN 11
#define LED_RWD_G_PIN 12
#define LED_RWD_B_PIN 13

// Definiera pinnar för RGB-LED 2
#define LED_FWD_R_PIN 0
#define LED_FWD_G_PIN 1
#define LED_FWD_B_PIN 14

// Färgkonstanter
#define OFF   0
#define RED   1
#define GREEN 2
#define BLUE  3
#define WHITE 4

// Definiera debouncetid i millisekunder
#define DEBOUNCE_MS 30

typedef struct {
    bool state;
    uint32_t last_bounce;
    uint pin;
} Button;

bool pressed(Button* btn, uint32_t now) {
    if (!gpio_get(btn->pin)) {
        if (!btn->state && (now - btn->last_bounce > DEBOUNCE_MS)) {
            btn->state = true;
            btn->last_bounce = now;
            return true;
        }
    }
    return false;
}

bool released(Button* btn, uint32_t now) {
    if (gpio_get(btn->pin)) {
        if (btn->state && (now - btn->last_bounce > DEBOUNCE_MS)) {
            btn->state = false;
            btn->last_bounce = now;
            return true;
        }
    }
    return false;
}

// Skapa knappobjekt
Button btnLop = {false, 0, BTN_LOP_PIN};
Button btnCue = {false, 0, BTN_CUE_PIN};
Button btnBar = {false, 0, BTN_BAR_PIN};
Button btnBit = {false, 0, BTN_BIT_PIN};
Button btnRwd = {false, 0, BTN_FWD_PIN};
Button btnFwd = {false, 0, BTN_RWD_PIN};


// Led-statusar
bool LedLop = false; // Bool för att hålla reda på om LOOP LED är på eller av
bool LedCue = false; // Bool för att hålla reda på om CUE LED är på eller av
bool LedBar = false; // Bool för att hålla reda på om BAR LED är på eller av
bool LedBit = false; // Bool för att hålla reda på om BIT LED är på eller av

// Knappstatus
bool BtnLopPrs = false; // Bool för att hålla reda på om LOOP-knappen är tryckt
bool BtnCuePrs = false; // Bool för att hålla reda på om CUE-knappen är tryckt
bool BtnBarPrs = false; // Bool för att hålla reda på om BAR-knappen är tryckt
bool BtnBitPrs = false; // Bool för att hålla reda på om BIT-knappen är tryckt
bool BitRwdPrs = false; // Bool för att hålla reda på om RWD-knappen är tryckt
bool BtnFwdPrs = false; // Bool för att hålla reda på om FWD-knappen är tryckt

// Spolningsstatus
bool CueRwdNow = false; // Bool för att hålla reda på om CUE RWD är aktiv
bool BarRwdNow = false; // Bool för att hålla reda på om BAR RWD är aktiv
bool BitRwdNow = false; // Bool för att hålla reda på om BIT RWD är aktiv
bool CueFwdNow = false; // Bool för att hålla reda på om CUE FWD är aktiv
bool BarFwdNow = false; // Bool för att hålla reda på om BAR FWD är aktiv
bool BitFwdNow = false; // Bool för att hålla reda på om BIT FWD är aktiv

// Midi-noter
uint8_t CueRwdNot = 1;   // Midi-not för CUE RWD
uint8_t BarRwdNot = 2;   // Midi-not för BAR RWD
uint8_t BitRwdNot = 3;   // Midi-not för BIT RWD
uint8_t CueFwdNot = 4;   // Midi-not för CUE FWD
uint8_t BarFwdNot = 5;   // Midi-not för BAR FWD
uint8_t BitFwdNot = 6;   // Midi-not för BIT FWD
uint8_t BtnLopNot = 100; // Midi-not för LOOP-knappen

// Debounce-timers
uint32_t BtnLopBnc = 0; // Debounce-timer för LOOP-knappen
uint32_t BtnCueBnc = 0; // Debounce-timer för CUE-knappen
uint32_t BtnBarBnc = 0; // Debounce-timer för BAR-knappen
uint32_t BtnBitBnc = 0; // Debounce-timer för BIT-knappen
uint32_t BtnRwdBnc = 0; // Debounce-timer för RWD-knappen
uint32_t BtnFwdBnc = 0; // Debounce-timer för FWD-knappen


void Led_Colors(uint8_t Led_Rwd_Color, uint8_t Led_Fwd_Color) {
    
    // LED RWD
    if (Led_Rwd_Color == RED) {
        gpio_put(LED_RWD_R_PIN, 1);
        gpio_put(LED_RWD_G_PIN, 0);
        gpio_put(LED_RWD_B_PIN, 0);
    } else if (Led_Rwd_Color == GREEN) {
        gpio_put(LED_RWD_R_PIN, 0);
        gpio_put(LED_RWD_G_PIN, 1);
        gpio_put(LED_RWD_B_PIN, 0);
    } else if (Led_Rwd_Color == BLUE) {
        gpio_put(LED_RWD_R_PIN, 0);
        gpio_put(LED_RWD_G_PIN, 0);
        gpio_put(LED_RWD_B_PIN, 1);
    } else if (Led_Rwd_Color == WHITE) {
        gpio_put(LED_RWD_R_PIN, 1);
        gpio_put(LED_RWD_G_PIN, 1);
        gpio_put(LED_RWD_B_PIN, 1);
    } else if (Led_Rwd_Color == OFF) {
        gpio_put(LED_RWD_R_PIN, 0);
        gpio_put(LED_RWD_G_PIN, 0);
        gpio_put(LED_RWD_B_PIN, 0);
    }

    // LED 2 FWD
    if (Led_Fwd_Color == RED) {
        gpio_put(LED_FWD_R_PIN, 1);
        gpio_put(LED_FWD_G_PIN, 0);
        gpio_put(LED_FWD_B_PIN, 0);
    } else if (Led_Fwd_Color == GREEN) {
        gpio_put(LED_FWD_R_PIN, 0);
        gpio_put(LED_FWD_G_PIN, 1);
        gpio_put(LED_FWD_B_PIN, 0);
    } else if (Led_Fwd_Color == BLUE) {
        gpio_put(LED_FWD_R_PIN, 0);
        gpio_put(LED_FWD_G_PIN, 0);
        gpio_put(LED_FWD_B_PIN, 1);
    } else if (Led_Fwd_Color == WHITE) {
        gpio_put(LED_FWD_R_PIN, 1);
        gpio_put(LED_FWD_G_PIN, 1);
        gpio_put(LED_FWD_B_PIN, 1);
    } else if (Led_Fwd_Color == OFF) {
        gpio_put(LED_FWD_R_PIN, 0);
        gpio_put(LED_FWD_G_PIN, 0);
        gpio_put(LED_FWD_B_PIN, 0);
    }
}



void set_loop_led() {
    gpio_put(LED_LOP_PIN, LedLop); // Sätt LED_LOP_PIN till det angivna tillståndet
}

void check_noteOff(bool &active_state, uint8_t note, uint8_t led, uint8_t base_color) {
    if (active_state) {
        uint8_t midi[3] = {0x80, note, 0}; // Skapa MIDI-meddelande // Note Off, Note, Velocity
        tud_midi_stream_write(0, midi, 3); // Skicka MIDI
        active_state = false; // Återställ tillståndet
    }
}

void check_buttons() {

    // CUE
    if (btnCue.state) {
        if (!CueRwdNow && btnRwd.state) {
            uint8_t note_on[3] = {0x90, CueRwdNot, 127};
            tud_midi_stream_write(0, note_on, 3);
            CueRwdNow = true;
        }
        if (!CueFwdNow && btnFwd.state) {
            uint8_t note_on[3] = {0x90, CueFwdNot, 127};
            tud_midi_stream_write(0, note_on, 3);
            CueFwdNow = true;
        }
    }

    // BAR
    else if (btnBar.state) {
        if (!BarRwdNow && btnRwd.state) {
            uint8_t note_on[3] = {0x90, BarRwdNot, 127};
            tud_midi_stream_write(0, note_on, 3);
            BarRwdNow = true;
        }
        if (!BarFwdNow && btnFwd.state) {
            uint8_t note_on[3] = {0x90, BarFwdNot, 127};
            tud_midi_stream_write(0, note_on, 3);
            BarFwdNow = true;
        }
    }

    // BIT
    else if (btnBit.state) {
        if (!BitRwdNow && btnRwd.state) {
            uint8_t note_on[3] = {0x90, BitRwdNot, 127};
            tud_midi_stream_write(0, note_on, 3);
            BitRwdNow = true;
        }
        if (!BitFwdNow && btnFwd.state) {
            uint8_t note_on[3] = {0x90, BitFwdNot, 127};
            tud_midi_stream_write(0, note_on, 3);
            BitFwdNow = true;
        }
    }
}

void update_led_colors() {

    // Om ingen av CUE, BAR eller BIT är nedtryckt: båda av
    if (!btnCue.state && !btnBar.state && !btnBit.state) {
        Led_Colors(OFF, OFF);
        return;
    }

    // Bestäm grundfärg
    uint8_t base_color = OFF;
    if (btnCue.state)      base_color = GREEN;
    else if (btnBar.state) base_color = BLUE;
    else if (btnBit.state) base_color = RED;

    // Om RWD eller FWD är nedtryckt samtidigt: respektive LED blir vit
    uint8_t rwd_color = (btnRwd.state) ? WHITE : base_color;
    uint8_t fwd_color = (btnFwd.state) ? WHITE : base_color;

    Led_Colors(rwd_color, fwd_color);
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
  gpio_init(LED_LOP_PIN);   gpio_set_dir(LED_LOP_PIN, GPIO_OUT);   gpio_put(LED_LOP_PIN, 0);
  gpio_init(LED_CUE_PIN);   gpio_set_dir(LED_CUE_PIN, GPIO_OUT);   gpio_put(LED_CUE_PIN, 0);
  gpio_init(LED_BAR_PIN);   gpio_set_dir(LED_BAR_PIN, GPIO_OUT);   gpio_put(LED_BAR_PIN, 0);
  gpio_init(LED_BIT_PIN);   gpio_set_dir(LED_BIT_PIN, GPIO_OUT);   gpio_put(LED_BIT_PIN, 0);
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

    // Kolla klockan
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // MIDI CC för loop-LED
    if (tud_midi_available()) {
        uint8_t midi[4];
        uint32_t bytes_read = tud_midi_stream_read(midi, sizeof(midi));
        if (bytes_read > 0) {
            if (((midi[0] & 0xF0) == 0xB0) && (midi[1] == 100)) {
                LedLop = (midi[2] > 63);
                set_loop_led();
            }
        }
    }

    // LOOP BUTTON
    if (pressed(&btnLop, now)) {
        uint8_t Lop_Val = LedLop ? 0 : 127;
        uint8_t midi[3] = {0xB0, BtnLopNot, Lop_Val};
        tud_midi_stream_write(0, midi, 3);
        LedLop = !LedLop;
        set_loop_led();
    }

    // CUE BUTTON
    if (pressed(&btnCue, now)) check_buttons();
    if (released(&btnCue, now)) {
        check_noteOff(CueRwdNow, CueRwdNot, 1, GREEN);
        check_noteOff(CueFwdNow, CueFwdNot, 2, GREEN);
    }

    // BAR BUTTON
    if (pressed(&btnBar, now)) check_buttons();
    if (released(&btnBar, now)) {
        check_noteOff(BarRwdNow, BarRwdNot, 1, OFF);
        check_noteOff(BarFwdNow, BarFwdNot, 2, OFF);
    }

    // BIT BUTTON
    if (pressed(&btnBit, now)) check_buttons();
    if (released(&btnBit, now)) {
        check_noteOff(BitRwdNow, BitRwdNot, 1, OFF);
        check_noteOff(BitFwdNow, BitFwdNot, 2, OFF);
    }

    // RWD BUTTON
    if (pressed(&btnRwd, now)) check_buttons();
    if (released(&btnRwd, now)) {
        check_noteOff(CueRwdNow, CueRwdNot);
        check_noteOff(BarRwdNow, BarRwdNot);
        check_noteOff(BitRwdNow, BitRwdNot);
    }

    // FWD BUTTON
    if (pressed(&btnFwd, now)) check_buttons();
    if (released(&btnFwd, now)) {
        check_noteOff(CueFwdNow, CueFwdNot);
        check_noteOff(BarFwdNow, BarFwdNot);
        check_noteOff(BitFwdNow, BitFwdNot);
    }

    // Uppdatera RGB-LED färger
    update_led_colors();
}


int main() {
  setup();
  while (1) {
    loop();
  }
  return 0;
}
