/* Arduinoで複数のロータリーエンコーダを使う
 * ごく単純な生成型シーケンサを創る
 */

/* 
 * コンパイルのために「UHS2-MIDI」ライブラリをインストールすること（引き連れるものも全部）
 */
#include <usbh_midi.h>
#include <usbhub.h>

USB Usb;
//USBHub Hub(&Usb);
USBH_MIDI  Midi(&Usb);

void initMidiUsb()
{
  char buf[20];
  uint16_t vid = Midi.idVendor();
  uint16_t pid = Midi.idProduct();
  sprintf(buf, "VID:%04X, PID:%04X", vid, pid);
  Serial.println(buf); 
}

void setupMidiUsb() {
  if (Usb.Init() == -1) {
    while (1); //halt
  }
  Midi.attachOnInit(onInit);
}

bool loopMidiUsbInput() {
  // MIDIキーボードの読み込み処理
  Usb.Task();
  if (Midi) {
    return MIDI_poll();
  }
  return false;
}

// Poll USB MIDI Controler and send to serial MIDI
bool MIDI_poll() {
  uint8_t bufMidi[MIDI_EVENT_PACKET_SIZE];
  uint16_t  rcvd;

  if (Midi.RecvData(&rcvd, bufMidi) == 0) {
    Serial.print(byte(bufMidi[1]));
    Serial.print(" :");
    Serial.println(byte(bufMidi[2]));
    SEND_MIDI(byte(bufMidi[1]), byte(bufMidi[2]));
    return true;
  }
  return false;
}
