#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "bsp/board.h"

// Pinnar
#define BTN_LOP_PIN   2
#define BTN_CUE_PIN   3
#define BTN_BAR_PIN   4
#define BTN_BIT_PIN   5
#define BTN_RWD_PIN   7
#define BTN_FWD_PIN   6
#define LED_LOP_PIN   15
#define LED_RWD_R_PIN 11
#define LED_RWD_G_PIN 13
#define LED_RWD_B_PIN 12
#define LED_FWD_R_PIN 8
#define LED_FWD_G_PIN 9
#define LED_FWD_B_PIN 10
#define LED_KLK_PIN   14

#define DEBOUNCE 30 // 30 ms debounce-tid för knappar

// Färgkonstanter för RWD/FWD LED och PWM-nivåer
#define OFF   0 // 0% PWM
#define WHITE 1 // 100% PWM
#define GRAY  2 // 80% PWM (PWM_GRAY)
#define GREEN 3 // 100% PWM för grön (Cue)
#define BLUE  4 // 100% PWM för blå (Bar)
#define RED   5 // 100% PWM för röd (Bit)

#define PWM_OFF   0   // 0% PWM
#define PWM_FULL  255 // 100% PWM
#define PWM_DIM   80  // Dimmad (80% PWM)

#define RWD 0 // RWD LED
#define FWD 1 // FWD LED

#define NON 0 // Durata enum
#define CUE 1 // Durata enum
#define BAR 2 // Durata enum
#define BIT 3 // Durata enum



extern uint32_t now;  // Variabel för att hålla reda på tiden
extern uint8_t durata; // Variabel för att bestämma värde åt RWD/FWD (CUE, BAR, BIT)
extern bool AllInfinito;  // Bool för att hålla reda på om LOOP LED är på eller av

#include <string.h>

typedef struct {
  char           nam[4]; // Namn på knappen (ändra storlek vid behov)
  bool           stt;
  uint32_t       prv;
  const uint8_t  pin;
  bool           act;
} btn;

extern btn btnLop;
extern btn btnCue;
extern btn btnBar;
extern btn btnBit;
extern btn btnRwd;
extern btn btnFwd;

extern uint8_t loopCC;  // MIDI-CC för Loop

typedef struct {
  bool           stt; // State för MIDI-not
  const uint8_t  mnn; // Midi-Note-Number
} note;


extern note rwdCue; // MIDI-note .stt State och .mnn Midi-Note-Number
extern note rwdBar; // MIDI-note .stt State och .mnn Midi-Note-Number
extern note rwdBit; // MIDI-note .stt State och .mnn Midi-Note-Number
extern note fwdCue; // MIDI-note .stt State och .mnn Midi-Note-Number
extern note fwdBar; // MIDI-note .stt State och .mnn Midi-Note-Number
extern note fwdBit; // MIDI-note .stt State och .mnn Midi-Note-Number

// MIDI-knapphjälpare
extern const uint8_t rwd[3]; // Array för RWD-MIDI-not-nummer
extern const uint8_t fwd[3]; // Array för FWD-MIDI-not-nummer

// Spolningsstatus
extern bool Ancora;  // Bool för att hålla reda på om RWD är aktiv (Rwd-Led:en använder denna!)
extern bool Avanti;  // Bool för att hålla reda på om FWD är aktiv (Fwd-Led:en använder denna!)

bool isnArr(const uint8_t* arr, size_t len, uint8_t val); // Help-funktion... "Finder"... "Is in Array"

// BUTTONS:
bool pressed(btn *btn); // Kollar om knappen är trycks ned
bool release(btn *btn); // Kollar om knappen släpps
void update_durata(); // Uppdaterar variabeln "durata" baserat på senaste knapptryckningen (Cue, Bar, Beat Eller Ingen...)
void update_buttons(); // Knapp logik, kollar om knappar är tryckta eller släppta och skickar MIDI-noter

// MIDI:
void read_midi(); // Läser MIDI-meddelanden med hjälp av TinyUSB
void sendMidiCC(uint8_t CC, uint8_t value); // Skickar MIDI CC-meddelande
void sendMidiNoteOn(uint8_t note, uint8_t velocity = 127); // Skickar MIDI Note On-meddelande
void sendMidiNoteOf(uint8_t note); // Skickar MIDI Note Off-meddelande

// LED:
void getLedPin(uint8_t ledType, int &r, int &g, int &b); // Hjälpfunktion för att hämta RGB-pinnar
void setLedRgb(uint8_t ledType, uint8_t color); // Sätter RGB-färg på LED baserat på typ och färg
void update_ancora_led(); // Uppdaterar RWD LED baserat på knappstatus
void update_avanti_led(); // Uppdaterar FWD LED baserat på knappstatus
void update_leds(); // Uppdaterar alla LED:ar baserat på knappstatus
void blink_loop(); // Test: Blinkar/Fade:ar Loop-pinnen med PWM

