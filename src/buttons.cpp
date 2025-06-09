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
  NOTE_ON,
  NOTE_OFF,
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

// const char *stringBtnStt(bool stt) {
//   stt ? printf("\033[32mPRESSED\033[0m") : printf("\033[31mRELEASED\033[0m");
// }

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

void durata_button_pressed(btn &btn){ // Hjälpfunktion för att hantera upprepad kod vid knapptryckningar
  // print_btn(btn); // Debugg print
  update_durata();   // Update durata
}

void durata_button_released(btn &btn){ // Hjälpfunktion för att hantera upprepad kod vid knappsläppningar
  // btn.stt = false;  // Stäng av
  // print_btn(btn); // Debugg print
  // btn.prv = 0;      // Nollställ tiden
  update_durata();   // Update durata
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



void update_buttons() {



  //   ░░          ░░░░░░     ░░░░░░    ░░░░░░░ 
  //   ▒▒         ▒▒    ▒▒   ▒▒    ▒▒   ▒▒    ▒▒
  //   ▓▓         ▓▓    ▓▓   ▓▓    ▓▓   ▓▓▓▓▓▓▓ 
  //   ██         ██    ██   ██    ██   ██      
  //   ████████    ██████     ██████    ██      

  if (pressed(btnLop)) {
    sendMidiCC(LOOPCC, 127); // Om AllInfinito är falskt, skicka CC med värde 127
    printf("\033[36m[MTX] \033[90m%s \033[38;5;229m%s \033[32m%s \033[97mAllInfinito: %s\n\033[0m", "B0", "LOOP", "F7", AllInfinito ? "\033[32mtrue" : "\033[31mfalse");


    // if (AllInfinito) {
    // }
    // else {
    // }
  }

  if (release(btnLop)) { // R E L E A S E
    sendMidiCC(LOOPCC, 0); // Om AllInfinito är sant, skicka CC med värde 0
    // printf("\033[36m[MTX] \033[90m%s \033[38;5;229m%s \033[31m%s \033[97mAllInfinito: %s\n\033[0m", "B0", "LOOP", "00", AllInfinito ? "\033[32mtrue" : "\033[31mfalse");

  }



  //    ░░░░░░░   ░░    ░░   ░░░░░░░░
  //   ▒▒         ▒▒    ▒▒   ▒▒      
  //   ▓▓         ▓▓    ▓▓   ▓▓▓▓▓▓  
  //   ██         ██    ██   ██      
  //    ███████    ██████    ████████  

  if (pressed(btnCue)) {
    update_durata();
    if (btnRwd.stt) midi_action(rwdCue, NOTE_ON); // <<- <<- <<- <<- <<- <<- <<- <<-
    if (btnFwd.stt) midi_action(fwdCue, NOTE_ON); // ->> ->> ->> ->> ->> ->> ->> ->>
  }
  if (release(btnCue)) { // R E L E A S E
    if (rwdCue.stt) midi_action(rwdCue, NOTE_OFF); // <<- <<- <<- <<- <<- <<- <<- <<-
    if (rwdCue.stt) midi_action(fwdCue, NOTE_OFF); // ->> ->> ->> ->> ->> ->> ->> ->>
    update_durata();
  }



  // ░░░░░░        ░░      ░░░░░░░ 
  // ▒▒   ▒▒     ▒▒  ▒▒    ▒▒    ▒▒
  // ▓▓▓▓▓▓     ▓▓    ▓▓   ▓▓▓▓▓▓▓ 
  // ██    ██   ████████   ██  ██  
  // ███████    ██    ██   ██    ██  

  if (pressed(btnBar)) {
    update_durata();
    if (btnRwd.stt) midi_action(rwdBar, NOTE_ON); // <<- <<- <<- <<- <<- <<- <<- <<-
    if (btnFwd.stt) midi_action(fwdBar, NOTE_ON); // ->> ->> ->> ->> ->> ->> ->> ->>
  }
  if (release(btnBar)) { // R E L E A S E
    if (rwdBar.stt) midi_action(rwdBar, NOTE_OFF); // <<- <<- <<- <<- <<- <<- <<- <<-
    if (fwdBar.stt) midi_action(fwdBar, NOTE_OFF); // ->> ->> ->> ->> ->> ->> ->> ->>
    update_durata();
  }   



// ░░░░░░     ░░░░░░░░      ░░      ░░░░░░░░
// ▒▒   ▒▒    ▒▒          ▒▒  ▒▒       ▒▒   
// ▓▓▓▓▓▓     ▓▓▓▓▓▓     ▓▓    ▓▓      ▓▓   
// ██    ██   ██         ████████      ██   
// ███████    ████████   ██    ██      ██   

  if (pressed(btnBit)) {
    update_durata();
    if (btnRwd.stt) midi_action(rwdBit, NOTE_ON); // <<- <<- <<- <<- <<- <<- <<- <<-
    if (btnFwd.stt) midi_action(fwdBit, NOTE_ON); // ->> ->> ->> ->> ->> ->> ->> ->>
  }
  if (release(btnBit)) { // R E L E A S E
    if (rwdBit.stt) midi_action(rwdBit, NOTE_OFF); // <<- <<- <<- <<- <<- <<- <<- <<-
    if (fwdBit.stt) midi_action(fwdBit, NOTE_OFF); // ->> ->> ->> ->> ->> ->> ->> ->>
    update_durata();
  }



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
