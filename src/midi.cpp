#include <stdio.h>
#include "class/midi/midi_device.h"
#include "main.h"


uint8_t midi_clock_count = 0;
uint32_t the_latest_beat = 0;
uint32_t mspb = 500; // Variabel för att hålla reda på tid i millisekunder per slag "milliseconds per beat"
uint16_t bpm = 120; // Variabel för att hålla reda på beats per minute "bpm"

bool is_valid_ms_per_beat(uint32_t ms) { // Returnerar true om ms_per_beat är inom giltigt intervall (250-1500 ms)
  return ms >= 250 && ms <= 1500;
}

void read_midi() {
  
  while (tud_midi_available()) {
    uint8_t midi[4]; // Buffer för att läsa in MIDI-data
    uint32_t bytes_read = tud_midi_stream_read(midi, sizeof(midi)); // Läs in MIDI-data från TinyUSB
    for (uint32_t i = 0; i < bytes_read; i++) { // Loopa igenom alla mottagna MIDI-byte
      

      if (midi[i] == 0xF8) { // Fånga MIDI Clock (0xF8)
        midi_clock_count++; // Öka räknaren för MIDI Clock
        if (midi_clock_count == 24) { // Var 24:e MIDI Clock är en beat
          if (the_latest_beat != 0) { // Om det inte är första slaget
            mspb = now - the_latest_beat; // Beräkna millisekunder per slag
            if (is_valid_ms_per_beat(mspb)) { // Kontrollera om mspb är inom rimligt intervall
              bpm = 60000.0f / mspb; // Beräkna BPM från mspb
              printf("\033[38;5;218m[MRX]\033[97m BPM: %.2f, \033[38;5;183mMSPB: %lu\n\033[0m", bpm, mspb);
            }
          }
          the_latest_beat = now; // Spara senaste slaget
          midi_clock_count = 0; // Nollställ räknaren för MIDI Clock
        }
      }

      if (i + 2 < bytes_read) { // Kontrollera att det finns tillräckligt med data för att läsa CC
        
        if ((midi[i] & 0xF0) == 0xB0 && midi[i+1] == 0x64) { // Fånga MIDI CC 100 (Loop)
          if (midi[i+2] >= 64 && !AllInfinito) { // Om CC 100 är 64 eller mer och AllInfinito är falskt
            printf("\033[38;5;218m[MRX]\033[90m %02X \033[38;5;229m%s\033[38;5;121m %02X\n\033[0m", midi[i], "LOOP", midi[i+2]);
            AllInfinito = true;
          } else if (midi[i+2] < 64 && AllInfinito) { // Om CC 100 är mindre än 64 och AllInfinito är sant
            printf("\033[38;5;218m[MRX]\033[90m %02X \033[38;5;229m%s\033[38;5;5m %02X\n\033[0m", midi[i], "LOOP", midi[i+2]);
            AllInfinito = false;
          }
        }
      }
    }
  }
}

void sendMidiCC(uint8_t CC, uint8_t value) {
  uint8_t midi_cc [3] = {0xB0, CC, value};
  tud_midi_stream_write(0, midi_cc, 3);
  // printf("[DEBUG] Skickar CC: CC=0x%02X, value=%d\n", CC, value);
}

void sendMidiNoteOn(uint8_t note, uint8_t velocity) {
  uint8_t note_on[3] = {0x90, note, velocity};
  tud_midi_stream_write(0, note_on, 3);
  // printf("[DEBUG] Skickar Note On: note=0x%02X, velocity=%d\n", note, velocity);
}

void sendMidiNoteOf(uint8_t note) {
  uint8_t midi[3] = {0x80, note, 0};
  tud_midi_stream_write(0, midi, 3);
  // printf("[DEBUG] Skickar Note Off: note=0x%02X\n", note);
}


