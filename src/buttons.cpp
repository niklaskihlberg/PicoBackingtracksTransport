#include "pico/stdlib.h"
#include "main.h"

uint32_t now = 0;
uint8_t durata = 0;

btn btnLop = { "LOOP", false, 0, BTN_LOP_PIN, { .pin = LED_LOP_PIN } }; // Loop-knapp
btn btnCue = { "CUE",  false, 0, BTN_CUE_PIN, { .pin = LED_CUE_PIN } }; // Cue-knapp
btn btnBar = { "BAR",  false, 0, BTN_BAR_PIN, { .pin = LED_BAR_PIN } }; // Bar-knapp
btn btnBit = { "BIT",  false, 0, BTN_BIT_PIN, { .pin = LED_BIT_PIN } }; // Bit-knapp
btn btnRwd = { "RWD",  false, 0, BTN_RWD_PIN, { .rgb = { LED_RWD_R_PIN, LED_RWD_G_PIN, LED_RWD_B_PIN } } }; // Rewind/Ancora-knapp
btn btnFwd = { "FWD",  false, 0, BTN_FWD_PIN, { .rgb = { LED_FWD_R_PIN, LED_FWD_G_PIN, LED_FWD_B_PIN } } }; // Forward/Avanti-knapp

enum ACTION {
  NOTE_ON, // Enum för att representera MIDI Note On-åtgärd
  NOTE_OFF, // Enum för att representera MIDI Note Off-åtgärd
};


bool AllInfinito = false; // Om LOOP LED är på eller av och Hur Loop-knappen skall bete sig!

typedef struct {
  bool           stt; // State för MIDI-not
  const uint8_t  mnn; // Midi-Note-Number
} note;

note rwdCue = { false, 0x01 };
note rwdBar = { false, 0x02 };
note rwdBit = { false, 0x03 };
note fwdCue = { false, 0x04 };
note fwdBar = { false, 0x05 };
note fwdBit = { false, 0x06 };

void print_btn(btn &btn) {
  printf("\033[33m[BTN] \033[90m%s %s\n\033[0m", btn.name, btn.stt ? "\033[32mPRESSED\033[0m" : "\033[31mRELEASED\033[0m");
}

bool pressed(btn &btn) {
  if (!gpio_get(btn.pin)) {
    if (!btn.stt && (now - btn.prv > DEBOUNCE)) {
      btn.stt = true;
      btn.prv = now;
      print_btn(btn);
      return true;
    }
  }
  return false;
}

bool release(btn &btn) { // Kollar om knappen släpps
  if (gpio_get(btn.pin)) {
    if (btn.stt && (now - btn.prv > DEBOUNCE)) {
      btn.stt = false;
      btn.prv = 0; // Nollställ tiden
      print_btn(btn);
      return true;
    }
  }
  return false;
}

void update_durata() {  // Uppdaterar variabeln "durata" baserat på senaste knapptryckningen (Cue, Bar, Beat Eller Ingen...)
  if (btnCue.prv + btnBar.prv + btnBit.prv == 0) { durata = DURATA::IDL; return; } // Om ingen knapp är nedtryckt, sätt till NON
  if      (btnCue.prv >= btnBar.prv && btnCue.prv >= btnBit.prv) durata = DURATA::CUE; // Hitta senaste knapptryckning
  else if (btnBar.prv >= btnCue.prv && btnBar.prv >= btnBit.prv) durata = DURATA::BAR;
  else if (btnBit.prv >= btnCue.prv && btnBit.prv >= btnBar.prv) durata = DURATA::BIT;
}


void midi_action(note &note, ACTION action) { // Hjälpfunktion för att hantera MIDI-åtgärder

  switch (action) {
    case ACTION::NOTE_ON:
      sendMidiNoteOn(note.mnn);
      note.stt = true;
    break;
  
    case ACTION::NOTE_OFF:
      sendMidiNoteOf(note.mnn); 
      note.stt = false;
    break;

  }
  
}

void durata_action(note &rwdNot, note &fwdNot, ACTION ACTION) { // Hjälpfunktion för att hantera MIDI-åtgärder vid knapptryckning
  update_durata();
  if (btnRwd.stt) midi_action(rwdNot, ACTION); // <<- <<- <<- <<- <<- <<- <<- <<-
  if (btnFwd.stt) midi_action(fwdNot, ACTION); // ->> ->> ->> ->> ->> ->> ->> ->>
}

void update_buttons() {

  if (pressed(btnLop)) {
    sendMidiCC(LOOPCC, 127); // Om AllInfinito är falskt, skicka CC med värde 127
    printf("\033[36m[MTX] \033[90m%s \033[38;5;229m%s \033[32m%s \033[97mAllInfinito: %s\n\033[0m", "B0", "LOOP", "F7", AllInfinito ? "\033[32mtrue" : "\033[31mfalse");
  }

  if (release(btnLop)) sendMidiCC(LOOPCC, 0); // Om AllInfinito är sant, skicka CC med värde 0

  if (pressed(btnCue)) durata_action(rwdCue, fwdCue, NOTE_ON); // Om CUE-knappen trycks ned, uppdatera durata och skicka Note On för rwdCue eller fwdCue
  if (release(btnCue)) durata_action(rwdCue, fwdCue, NOTE_OFF); // Om CUE-knappen trycks ned, uppdatera durata och skicka Note On för rwdCue eller fwdCue

  if (pressed(btnBar)) durata_action(rwdBar, fwdBar, NOTE_ON); // Om BAR-knappen trycks ned, uppdatera durata och skicka Note On för rwdBar eller fwdBar
  if (release(btnBar)) durata_action(rwdBar, fwdBar, NOTE_OFF); // Om BAR-knappen trycks ned, uppdatera durata och skicka Note On för rwdBar eller fwdBar

  if (pressed(btnBit)) durata_action(rwdBit, fwdBit, NOTE_ON); // Om BIT-knappen trycks ned, uppdatera durata och skicka Note On för rwdBit eller fwdBit
  if (release(btnBit)) durata_action(rwdBit, fwdBit, NOTE_OFF); // Om BIT-knappen trycks ned, uppdatera durata och skicka Note On för rwdBit eller fwdBit


  //   ░░░░░░░    ░░░░░░░░   ░░    ░░    ░░░░░░    ░░░   ░░   ░░░░░░░ 
  //   ▒▒    ▒▒   ▒▒         ▒▒ ▒▒ ▒▒      ▒▒      ▒▒▒▒  ▒▒   ▒▒    ▒▒
  //   ▓▓▓▓▓▓▓    ▓▓▓▓▓▓     ▓▓ ▓▓ ▓▓      ▓▓      ▓▓ ▓▓ ▓▓   ▓▓    ▓▓
  //   ██  ██     ██         ██ ██ ██      ██      ██  ████   ██    ██
  //   ██    ██   ████████   ███  ███    ██████    ██    ██   ███████ 

  if (pressed(btnRwd)) {
    switch (durata) {
      case DURATA::IDL: break; // Om ingen knapp är nedtryckt, gör ingenting
      case DURATA::CUE: midi_action(rwdCue, NOTE_ON); break; // Om CUE, skicka Note On för rwdCue
      case DURATA::BAR: midi_action(rwdBar, NOTE_ON); break; // Om BAR, skicka Note On för rwdBar
      case DURATA::BIT: midi_action(rwdBit, NOTE_ON); break; // Om BIT, skicka Note On för rwdBit
    }
  }

  if (release(btnRwd)) {
    switch (durata) {
      case DURATA::IDL: break; // Om ingen knapp är nedtryckt, gör ingenting
      case DURATA::CUE: midi_action(rwdCue, NOTE_OFF); break; // Om CUE, skicka Note Off för rwdCue
      case DURATA::BAR: midi_action(rwdBar, NOTE_OFF); break; // Om BAR, skicka Note Off för rwdBar
      case DURATA::BIT: midi_action(rwdBit, NOTE_OFF); break; // Om BIT, skicka Note Off för rwdBit
    }
  }



  //      ░░      ░░    ░░      ░░      ░░░   ░░   ░░░░░░░░    ░░░░░░ 
  //    ▒▒  ▒▒    ▒▒    ▒▒    ▒▒  ▒▒    ▒▒▒▒  ▒▒      ▒▒         ▒▒   
  //   ▓▓    ▓▓    ▓▓  ▓▓    ▓▓    ▓▓   ▓▓ ▓▓ ▓▓      ▓▓         ▓▓   
  //   ████████     ████     ████████   ██  ████      ██         ██   
  //   ██    ██      ██      ██    ██   ██    ██      ██       ██████ 

  if (pressed(btnFwd)) {
    switch (durata) {
      case DURATA::IDL: break; // Om ingen knapp är nedtryckt, gör ingenting
      case DURATA::CUE: midi_action(fwdCue, NOTE_ON); break; // Om CUE, skicka Note On för fwdCue
      case DURATA::BAR: midi_action(fwdBar, NOTE_ON); break; // Om BAR, skicka Note On för fwdBar
      case DURATA::BIT: midi_action(fwdBit, NOTE_ON); break; // Om BIT, skicka Note On för fwdBit
    }
  }

  if (release(btnFwd)) {
    switch (durata) {
      case DURATA::IDL: break; // Om ingen knapp är nedtryckt, gör ingenting
      case DURATA::CUE: midi_action(fwdCue, NOTE_OFF); break; // Om CUE, skicka Note Off för fwdCue
      case DURATA::BAR: midi_action(fwdBar, NOTE_OFF); break; // Om BAR, skicka Note Off för fwdBar
      case DURATA::BIT: midi_action(fwdBit, NOTE_OFF); break; // Om BIT, skicka Note Off för fwdBit
    }
  }
}
