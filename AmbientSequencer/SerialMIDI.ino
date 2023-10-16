/* ---------------- シリアル通信でのMIDI信号操作用の関数群 ---------------- */

// PORT PIN ID DEFINITIONS start -----
#include <SoftwareSerial.h>
#define VELOCITY 127
SoftwareSerial MIDI(5, 6); // RX, TX
// PORT PIN ID DEFINITIONS end -------

void setupSerialMIDI() {
  randomSeed(analogRead(0));  // 未接続ピンのノイズを利用してランダムシード
  Serial.println("MIDI Setup");
  MIDI.begin(31250);
}

void SEND_MIDI(byte cate, byte note, boolean printNote) {
  if (note < 20) {
    Serial.print(" :lower Note ");
    Serial.println(note);
  } else {
    byte data[3] = {cate, note, VELOCITY};
    for (byte i = 0 ; i < 3 ; i++) {
      MIDI.write(data[i]);
    }
    if (printNote) {
      Serial.print("  ");
      Serial.print(note);
    }    
  }
}

void MIDIPanic() {
  MIDI.write(123);
  Serial.println("MIDI Panic 123 sent.");
  STOP_ALL_MIDI();
}

// Panicに対応していない機種のために
void STOP_ALL_MIDI() {
  for (int i=20; i<127; i++) {
    SEND_MIDI(MIDI_OFF, i, false);
  }  
}
