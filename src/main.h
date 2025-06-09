#pragma once
#include "bsp/board.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum Pinnar {
  BTN_LOP_PIN   = 2, // Loop-knapp
  BTN_CUE_PIN   = 3, // Cue-knapp
  BTN_BAR_PIN   = 4, // Bar-knapp
  BTN_BIT_PIN   = 5, // Bit-knapp
  BTN_RWD_PIN   = 7, // Rewind/Ancora-knapp
  BTN_FWD_PIN   = 6, // Forward/Avanti-knapp
  LED_LOP_PIN   = 15, // Loop LED
  LED_RWD_R_PIN = 11, // RWD Red LED
  LED_RWD_G_PIN = 13, // RWD Green LED
  LED_RWD_B_PIN = 12, // RWD Blue LED
  LED_FWD_R_PIN = 8, // FWD Red LED
  LED_FWD_G_PIN = 9, // FWD Green LED
  LED_FWD_B_PIN = 10, // FWD Blue LED
  LED_CUE_PIN   = 14, // Cue LED
  LED_BAR_PIN   = 16, // Bar LED
  LED_BIT_PIN   = 17, // Bit LED
};


#define DEBOUNCE 30 // 30 ms debounce-tid för knappar

extern uint32_t now;  // Variabel för att hålla reda på tiden
extern uint8_t durata; // Variabel för att bestämma värde åt RWD/FWD (CUE, BAR, BIT)
extern uint32_t the_latest_beat; // Variabel för att hålla reda på senaste slaget (beat)
extern uint32_t mspb; // Variabel för att hålla reda på tid i millisekunder per slag "milliseconds per beat"
extern uint16_t bpm; // Variabel för att hålla reda på beats per minute "bpm"
extern bool AllInfinito;  // Bool för att hålla reda på om LOOP LED är på eller av



// BUTTONS:
//   ░░░░░░     ░░    ░░   ░░░░░░░░   ░░░░░░░░    ░░░░░░    ░░░   ░░    ░░░░░░░
//   ▒▒   ▒▒    ▒▒    ▒▒      ▒▒         ▒▒      ▒▒    ▒▒   ▒▒▒▒  ▒▒   ▒▒      
//   ▓▓▓▓▓▓     ▓▓    ▓▓      ▓▓         ▓▓      ▓▓    ▓▓   ▓▓ ▓▓ ▓▓    ▓▓▓▓▓▓ 
//   ██    ██   ██    ██      ██         ██      ██    ██   ██  ████         ██
//   ███████     ██████       ██         ██       ██████    ██    ██   ███████ 

typedef struct {
  const char     name[6]; // Namn på knappen (ändra storlek vid behov)
  bool           stt;
  uint32_t       prv;
  const uint8_t  pin;

  // bool           rgb; // Om knappen har RGB LED

  union {
    struct { // RGB LED-knapp
      const uint8_t  r; // Röd LED-pin
      const uint8_t  g; // Grön LED-pin
      const uint8_t  b; // Blå LED-pin/* data */
    } rgb;
  const uint8_t  pin; // LED-pin för enkel LED-knapp
  } led; // LED-knapp

} btn;

extern btn btnLop;
extern btn btnCue;
extern btn btnBar;
extern btn btnBit;
extern btn btnRwd;
extern btn btnFwd;

enum DURATA { // Durata bestämmer längd på RWD/FWD!
  IDL = 3,  // Off-längd
  CUE = 2,  // Cue-längd
  BAR = 1,  // Bar-längd
  BIT = 0,  // Beat-längd
};

void update_buttons(); // Knapp logik, kollar om knappar är tryckta eller släppta och skickar MIDI-noter



// MIDI:
//   ░░░  ░░░    ░░░░░░    ░░░░░░░     ░░░░░░ 
//   ▒▒ ▒▒ ▒▒      ▒▒      ▒▒    ▒▒      ▒▒   
//   ▓▓ ▓▓ ▓▓      ▓▓      ▓▓    ▓▓      ▓▓   
//   ██ ██ ██      ██      ██    ██      ██   
//   ██    ██    ██████    ███████     ██████ 

#define LOOPCC 100 // MIDI CC för Loop

void read_midi(); // Läser MIDI-meddelanden med hjälp av TinyUSB
void sendMidiCC(uint8_t CC, uint8_t value); // Skickar MIDI CC-meddelande
void sendMidiNoteOn(uint8_t note, uint8_t velocity = 127); // Skickar MIDI Note On-meddelande
void sendMidiNoteOf(uint8_t note); // Skickar MIDI Note Off-meddelande

// LED:
//   ░░         ░░░░░░░░   ░░░░░░░ 
//   ▒▒         ▒▒         ▒▒    ▒▒
//   ▓▓         ▓▓▓▓▓▓     ▓▓    ▓▓
//   ██         ██         ██    ██
//   ████████   ████████   ███████ 

void update_leds(); // Uppdaterar alla LED:ar baserat på knappstatus

