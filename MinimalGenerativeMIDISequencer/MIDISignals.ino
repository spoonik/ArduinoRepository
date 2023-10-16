#include <SoftwareSerial.h>

// PORT PIN ID DEFINITIONS start -----
SoftwareSerial MIDI(5, 6); // RX, TX
// PORT PIN ID DEFINITIONS end -------

const byte VELOCITY = 127;

void setupMIDISignal() {
  randomSeed(analogRead(0)); // 未接続ピンのノイズを利用してランダムシード
  Serial.println("MIDI Setup");
  MIDI.begin(31250);
}

void SEND_MIDI(byte cate, byte note) {
  if (note < 20) {
    Serial.print(cate);
    Serial.print(" :lower Note ");
    Serial.println(note);
    return;
  }
  byte data[3] = {cate, note, VELOCITY};
  for (byte i = 0 ; i < 3 ; i++) {
    MIDI.write(data[i]);
  }
  Serial.print("Playing Note: ");
  Serial.println(note);
}

void MIDIPanic() {
  MIDI.write(123);
  Serial.println("MIDI Panic 123 sent.");
}
