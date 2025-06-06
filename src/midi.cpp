#include "main.h"
#include <stdio.h>
#include "class/midi/midi_device.h"

uint8_t loopCC = 100;

note rwdCue = { false, 0x01 };
note rwdBar = { false, 0x02 };
note rwdBit = { false, 0x03 };
note fwdCue = { false, 0x04 };
note fwdBar = { false, 0x05 };
note fwdBit = { false, 0x06 };

const uint8_t rwd[3] = { rwdCue.mnn, rwdBar.mnn, rwdBit.mnn };
const uint8_t fwd[3] = { fwdCue.mnn, fwdBar.mnn, fwdBit.mnn };

bool isnArr(const uint8_t* arr, size_t len, uint8_t val) {
  for (size_t i = 0; i < len; ++i)
    if (arr[i] == val) return true;
  return false;
}

void read_midi() {
  while (tud_midi_available()) {
    uint8_t midi[4];
    uint32_t bytes_read = tud_midi_stream_read(midi, sizeof(midi));
    for (uint32_t i = 0; i + 2 < bytes_read; i++) {
      
      // printf("[DEBUG] MIDI RX: %02X %02X %02X\n", midi[i], midi[i+1], midi[i+2]);
      
      // if ((midi[i] & 0xF0) == 0xB0 && midi[i+1] == 100) {
      if ((midi[i] & 0xF0) == 0xB0 && midi[i+1] == 0x64) {
        if (midi[i+2] >= 64 && !AllInfinito) {
          AllInfinito = !AllInfinito;
        } else 
        if (midi[i+2] <  64 && AllInfinito) {
          AllInfinito = !AllInfinito;
        }
      }
    }
  }
}

void sendMidiCC(uint8_t CC, uint8_t value) {
  // printf("[DEBUG] Skickar CC: CC=0x%02X, value=%d\n", CC, value);
  uint8_t midi_cc [3] = {0xB0, CC, value};
  tud_midi_stream_write(0, midi_cc, 3);
}

void sendMidiNoteOn(uint8_t note, uint8_t velocity) {
  // printf("[DEBUG] Skickar Note On: note=0x%02X, velocity=%d\n", note, velocity);
  uint8_t note_on[3] = {0x90, note, velocity};
  tud_midi_stream_write(0, note_on, 3);
  if (isnArr(rwd, 3, note) && !Ancora) Ancora = true;
  else if (isnArr(fwd, 3, note) && !Avanti) Avanti = true;
}

void sendMidiNoteOf(uint8_t note) {
  // printf("[DEBUG] Skickar Note Off: note=0x%02X\n", note);
  uint8_t midi[3] = {0x80, note, 0};
  tud_midi_stream_write(0, midi, 3);
  if (isnArr(rwd, 3, note) && Ancora) Ancora = false;
  else if (isnArr(fwd, 3, note) && Avanti) Avanti = false;
}


