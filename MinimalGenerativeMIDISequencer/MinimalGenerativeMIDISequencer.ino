/* Arduinoで複数のロータリーエンコーダを使う
 * ごく単純な生成型シーケンサを創る
 */
void onInit() {
  initMidiUsb();
}

void setup() {
  Serial.begin(115200);
  
  // Setup Modules
  setupStopButton();
  delay(1000);

  setupMIDISequencer();
  setupMidiUsb();
}

void loop() {
  // コントロールボタンの処理
  if (readButtonControllers())  return;

  // シーケンス演奏
  if (!loopPlaySequenceNote()) {

    // シーケンス中でなければMIDIキーボードの読み込み処理
    loopMidiUsbInput();
  }
}
