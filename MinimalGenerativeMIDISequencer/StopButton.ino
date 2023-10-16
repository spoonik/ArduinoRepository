/* シーケンサのStart/Stopボタン
 * Stop時には、MIDI Panicも送信する
 */

#define SEQ_START_BUTTON_PIN 3

#define TEMPO_CHANGE_BUTTON_PIN 2
#define NOTENUM_CHANGE_BUTTON_PIN 1


void setupStopButton() {
  pinMode(SEQ_START_BUTTON_PIN, OUTPUT);
  delay(200);
}

// 各種ボタンコントロールの状態読み取り
//[TODO] ほんとは割り込み処理した方がいい
bool readButtonControllers() {
  // シーケンサーのPlay/Stop
  if (digitalRead(SEQ_START_BUTTON_PIN) == HIGH) {
    MIDIPanic();
    Serial.println("<===SquenceStop/Run");
    togglePlayingSeq();
    return true;
  }
  // テンポ変更のボタン。押すとテンポアップする
  else if (digitalRead(TEMPO_CHANGE_BUTTON_PIN) == LOW) {
    Serial.println("<===TempoChange Btn Pushed");
    return true;
  }
  // モード音数変更変更のボタン。押すと音数アップする
  else if (digitalRead(NOTENUM_CHANGE_BUTTON_PIN) == LOW) {
    Serial.println("<===NoteNum Change Pushed");
    return true;
  }

  return false;
}
