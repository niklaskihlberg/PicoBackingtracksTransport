#include "main.h"
#include "pico/stdlib.h"

btn btnLop = { "LOP", false, 0, BTN_LOP_PIN };
btn btnCue = { "CUE", false, 0, BTN_CUE_PIN };
btn btnBar = { "BAR", false, 0, BTN_BAR_PIN };
btn btnBit = { "BIT", false, 0, BTN_BIT_PIN };
btn btnRwd = { "RWD", false, 0, BTN_RWD_PIN };
btn btnFwd = { "FWD", false, 0, BTN_FWD_PIN };

uint8_t durata = 0;

bool Ancora = false;
bool Avanti = false;

bool AllInfinito = false;

bool pressed(btn *btn) {
  if (!gpio_get(btn->pin)) {
    if (!btn->stt && (now - btn->prv > DEBOUNCE)) {
      btn->stt = true;
      btn->prv = now;
      return true;
    }
  }
  return false;
}

bool release(btn *btn) {
  if (gpio_get(btn->pin)) {
    if (btn->stt && (now - btn->prv > DEBOUNCE)) {
      btn->stt = false;
      btn->prv = now;
      return true;
    }
  }
  return false;
}

void update_durata() {
  if (btnCue.prv + btnBar.prv + btnBit.prv == 0) { durata = NON; return; } // Om ingen knapp är nedtryckt, sätt till NON
  if      (btnCue.prv >= btnBar.prv && btnCue.prv >= btnBit.prv) durata = CUE; // Hitta senaste knapptryckning
  else if (btnBar.prv >= btnCue.prv && btnBar.prv >= btnBit.prv) durata = BAR;
  else if (btnBit.prv >= btnCue.prv && btnBit.prv >= btnBar.prv) durata = BIT;
}



const char *stringBtnStt(bool stt) {
  if (stt) return "\033[32mPRESSED\033[0m";
  else return "\033[31mRELEASED\033[0m";  
}

void print_button(btn *btn) {
  printf("\033[33m[BUTTON]\033[0m ");
  printf("%s, %s, ", btn->nam, stringBtnStt(btn->stt));
}


  



void update_buttons() {

  if (pressed(&btnLop)) { // LOOP ████████████████████
    btnLop.stt = true; //                                                             ██
    print_button(&btnLop);
    uint8_t value = AllInfinito ? 0 : 127;
    sendMidiCC(loopCC, value);
    AllInfinito = !AllInfinito;
  }

  if (release(&btnLop)) { // R E L E A S E
    btnLop.stt = false; //                                                            ██
    print_button(&btnLop);
  }

  if (pressed(&btnCue)) { // ███ CUE ███████████████████████████████████████████████████
    
    btnCue.stt = true; //                                                             ██
    print_button(&btnCue);
    btnCue.prv = now; //                                                              ██
    update_durata(); //                                                               ██
    if (btnRwd.stt) { // <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <--  ██
      sendMidiNoteOn(rwdCue.mnn); //                                                  ██
      rwdCue.stt = true; //                                                           ██
    } //                                                                              ██
    if (btnFwd.stt) { // --> --> --> --> --> --> --> --> --> --> --> --> --> --> -->  ██
      sendMidiNoteOn(fwdCue.mnn); //                                                  ██
      fwdCue.stt = true; //                                                           ██
    } //                                                                              ██
  } //                                                                                ██
  if (release(&btnCue)) { // R E L E A S E                                            ██
    
    if (rwdCue.stt) { // <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <--  ██
      sendMidiNoteOf(rwdCue.mnn); //                                                  ██
      rwdCue.stt = false; //                                                          ██
    } //                                                                              ██
    if (fwdCue.stt) { // --> --> --> --> --> --> --> --> --> --> --> --> --> --> -->  ██
      sendMidiNoteOf(fwdCue.mnn); //                                                  ██
      fwdCue.stt = false; //                                                          ██
    } //                                                                              ██
    btnCue.stt = false; //                                                            ██
    print_button(&btnCue);
    btnCue.prv = 0; //                                                                ██
    update_durata(); //                                                               ██
  } // █████████████████████████████████████████████████████████████████████████████████
  if (pressed(&btnBar)) { // BAR ███████████████████████████████████████████████████████
    
    btnBar.stt = true; //                                                             ██
    print_button(&btnBar);
    btnBar.prv = now; //                                                              ██
    update_durata(); //                                                               ██
    if (btnRwd.stt) { // <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <--  ██
      sendMidiNoteOn(rwdBar.mnn); //                                                  ██
      rwdBar.stt = true; //                                                           ██
    } //                                                                              ██
    if (btnFwd.stt) { // --> --> --> --> --> --> --> --> --> --> --> --> --> --> -->  ██
      sendMidiNoteOn(fwdBar.mnn); //                                                  ██
      rwdBar.stt = true; //                                                           ██
    } //                                                                              ██
  } //                                                                                ██
  if (release(&btnBar)) { // R E L E A S E                                            ██
    
    if (rwdBar.stt) { // <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <--  ██
      sendMidiNoteOf(rwdBar.mnn); //                                                  ██
      rwdBar.stt = false; //                                                          ██
    } //                                                                              ██
    if (fwdBar.stt) { // --> --> --> --> --> --> --> --> --> --> --> --> --> --> -->  ██
      sendMidiNoteOf(fwdBar.mnn); //                                                  ██
      fwdBar.stt = false; //                                                          ██
    } //                                                                              ██
    btnBar.stt = false; //                                                            ██
    print_button(&btnBar);
    btnBar.prv = 0; //                                                                ██
    update_durata(); //                                                               ██
  } // █████████████████████████████████████████████████████████████████████████████████
  if (pressed(&btnBit)) { // BIT ███████████████████████████████████████████████████████
    btnBit.stt = true; //                                                             ██
    print_button(&btnBit);
    btnBit.prv = now; //                                                              ██
    update_durata(); //                                                               ██
    if (btnRwd.stt) { // <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <--  ██
      sendMidiNoteOn(rwdBit.mnn); //                                                  ██
      rwdBit.stt = true; //                                                           ██
    } //                                                                              ██
    if (btnFwd.stt) { // --> --> --> --> --> --> --> --> --> --> --> --> --> --> -->  ██
      sendMidiNoteOn(fwdBit.mnn); //                                                  ██
      rwdBit.stt = true; //                                                           ██
    } //                                                                              ██
  } //                                                                                ██
  if (release(&btnBit)) { // R E L E A S E                                            ██
    
    if (rwdBit.stt) { // <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <-- <--  ██
      sendMidiNoteOf(rwdBit.mnn); //                                                  ██
      rwdBit.stt = false; //                                                          ██
    } //                                                                              ██
    if (fwdBit.stt) { // --> --> --> --> --> --> --> --> --> --> --> --> --> --> -->  ██
      sendMidiNoteOf(fwdBit.mnn); //                                                  ██
      fwdBit.stt = false; //                                                          ██
    } //                                                                              ██
    btnBit.stt = false; //                                                            ██
    print_button(&btnBit);
    btnBit.prv = 0; //                                                                ██
    update_durata(); //                                                               ██
  } // █████████████████████████████████████████████████████████████████████████████████

  if (pressed(&btnRwd)) {

    btnRwd.stt = true;
    print_button(&btnRwd);

    if (durata == NON) {
      return;
    }

    if (durata == CUE) {
      sendMidiNoteOn(rwdCue.mnn);
      rwdCue.stt = true;
      return;
    }

    if (durata == BAR) {
      sendMidiNoteOn(rwdBar.mnn);
      rwdBar.stt = true;
      return;
    }

    if (durata == BIT) {
      sendMidiNoteOn(rwdBit.mnn);
      rwdBit.stt = true;
      return;
    }

  }

  if (release(&btnRwd)) {

    if (rwdCue.stt) {
      sendMidiNoteOf(rwdCue.mnn);
      rwdCue.stt = false;
    }

    if (rwdBar.stt) {
      sendMidiNoteOf(rwdBar.mnn);
      rwdBar.stt = false;
    }

    if (rwdBit.stt) {
      sendMidiNoteOf(rwdBit.mnn);
      rwdBit.stt = false;
    }

    btnRwd.stt = false;
    print_button(&btnRwd);

  }

  if (pressed(&btnFwd)) {

    btnFwd.stt = true;
    print_button(&btnFwd);

    if (durata == NON) {
      return;
    }

    if (durata == CUE) {
      sendMidiNoteOn(fwdCue.mnn);
      fwdCue.stt = true;
      return;
    }

    if (durata == BAR) {
      sendMidiNoteOn(fwdBar.mnn);
      fwdBar.stt = true;
      return;
    }

    if (durata == BIT) {
      sendMidiNoteOn(fwdBit.mnn);
      fwdBit.stt = true;
      return;
    }

  }

  if (release(&btnFwd)) {


    if (fwdCue.stt) {
      sendMidiNoteOf(fwdCue.mnn);
      fwdCue.stt = false;
    }

    if (fwdBar.stt) {
      sendMidiNoteOf(fwdBar.mnn);
      fwdBar.stt = false;
    }

    if (fwdBit.stt) {
      sendMidiNoteOf(fwdBit.mnn);
      fwdBit.stt = false;
    }

    btnFwd.stt = false;
    print_button(&btnFwd);

  }
}
