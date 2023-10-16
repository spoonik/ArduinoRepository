/* ---------------- 偏りありの乱数計算用プリミティブ関数群 ---------------- */
// 正規分布での偏りを持った乱数生成ユーティリティ
int randomWeightedIndex(int availableCount) {
  // 選択確率の分布を計算
  float distribution[100];  //TODO: サイズ決め打ちでやってる
  float sumDistribution = 0.0;
  for (int i = 0; i < availableCount; i++) {
    float distance = float(i);
    distribution[i] = exp(-0.5 * (distance / 2) * (distance / 2));
    sumDistribution += distribution[i];
  }

  // 確率の合計が1になるように正規化
  for (int i = 0; i < availableCount; i++) {
    distribution[i] /= sumDistribution;
  }

  float randomValue = random(0, 1000) / 1000.0; // 0から1の乱数を生成
  for (int i = 0; i < availableCount; i++) {
    randomValue -= distribution[i];
    if (randomValue <= 0) {
      return i;
    }
  }
  return availableCount - 1;
}

// 正規分布の重み付けで、5度圏の転調の主音を選択する
// TODO: グローバル変数を削除したい
void changeTonic() {
  int tonic = currentTonic;
  int fifthRange[] = {tonic - 7, tonic - 6, tonic - 5, tonic - 4,
                      tonic - 3, tonic - 2, tonic - 1, tonic + 1,
                      tonic + 2, tonic + 3, tonic + 4, tonic + 5,
                      tonic + 6, tonic + 7};
  int availableNotes[14];
  int availableNotesCount = 0;

  // MIDIノートの範囲内で5度圏の音を抽出
  for (int i = 0; i < 14; i++) {
    if (fifthRange[i] >= LOWEST_NOTE && fifthRange[i] <= HIGHEST_NOTE) {
      availableNotes[availableNotesCount] = fifthRange[i];
      availableNotesCount++;
    }
  }

  // 正規分布でランダムに次の中心音を選択
  int nextTonic = availableNotes[randomWeightedIndex(availableNotesCount)];
  if (nextTonic == currentTonic)  return;

  currentTonic = nextTonic;
  Serial.print("\nChange Tonic: ");
  Serial.println(nextTonic);
  flashLED(2);  // 変更時に2回Lチカ
}

// 指定のパーセントの確率で、choice2を選択して返す
int choiceByPercent(int choice1, int choice2, float percent) {
  float r = random(0, 100);
  if (r < percent) {
    return choice2;
  }
  return choice1;
}


// MIDI Noteと、その音名をprintする
String NOTE_NAME[] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};
void printNoteName(int midiNote) {
  // ノート名を出す
  int gap = midiNote - 60;
  while (gap < 0) {
    gap += 12;
  }
  while (gap > 71) {
    gap -= 12;
  }
  Serial.print(NOTE_NAME[gap]);
  Serial.print(" ");
  Serial.print(midiNote);
  Serial.print("   ");
}


/* ---------------- 音楽的なランダム変更のユーティリティ関数群 ---------------- */
// 正規分布の重み付けで、モードを選択する
// TODO: グローバル変数を削除したい
void changeMode() {
  int nextModeIndex = randomWeightedIndex(numModes);
  if (nextModeIndex == currentModeIndex)  return;

  // モード構成音は同じまま、トニック主音を差し替える形にする
  int nextTonic = currentTonic + TONIC_TONE_DISTANCE[nextModeIndex] - TONIC_TONE_DISTANCE[currentModeIndex];
  Serial.print("\nChange Mode: from ");
  Serial.print(currentModeIndex);
  Serial.print(" on ");
  Serial.print(currentTonic);
  Serial.print(" -to-> ");
  Serial.print(nextModeIndex);
  Serial.print(" on ");
  Serial.println(nextTonic);

  currentModeIndex = nextModeIndex;
  currentTonic = nextTonic;
  flashLED(2);  // 変更時に2回Lチカ
}

// 3音のMIDIノートを入力にして、配列にランダムな音高にシャッフルしたコードを入れる
void makeShuffleChordNotes(int *sequence, int tonic, int note2, int note3) {
  // MIDIシーケンスを生成し、順序をシャッフル
  sequence[0] = tonic;
  sequence[1] = min(note2, note3) + (random(-1, 1) * 12);  //TODO: ランダムにオクターブ下
  sequence[2] = max(note2, note3) + (random(-1, 1) * 12);  //TODO: ランダムにオクターブ下
  for (int i = MAX_NOTE_COUNT - 1; i > 0; i--) {
    int j = random(0, i + 1);
    int temp = sequence[i];
    sequence[i] = sequence[j];
    sequence[j] = temp;
  }
  // TODO: currentStepのカウントがうまくいってないバグを回避、安直・・・
  sequence[3] = sequence[2];
  Serial.print("\nNew Chord: ");
  for (int i=0; i < MAX_NOTE_COUNT; i++) {
    printNoteName(sequence[i]);
  }
  Serial.println("");  
}

// 現在のモードでのトニックの3和音を作る
void makeTonicThreeChord(int tonic, int mode, int *sequence) {
  // 現在のモードの第3音と第5音または第7音を選択
  int characteristicNoteIndex = 2;
  int otherNoteIndex = choiceByPercent(4, 7, 50.0);

  // モード中の各音Indexの度数とトニックを合わせる
  int* selectedMode = CHURCH_MODES[mode];
  int characteristicNote = tonic + selectedMode[characteristicNoteIndex];
  int otherNote = tonic + selectedMode[otherNoteIndex];

  // MIDIノートの音高をシャッフルしたコードにする
  makeShuffleChordNotes(sequence, tonic, characteristicNote, otherNote);  
}

// 現在のモードでのサブドミナントの3和音を作る
// サブドミナントは、トニック＋第4音＋第6音のこととする
void makeSubDominantThreeChord(int tonic, int mode, int *sequence) {
  // 現在のモードの第3音と第5音または第7音を選択
  int characteristicNoteIndex = 3;
  int otherNoteIndex = 5;

  // モード中の各音Indexの度数とトニックを合わせる
  int* selectedMode = CHURCH_MODES[mode];
  int characteristicNote = tonic + selectedMode[characteristicNoteIndex];
  int otherNote = tonic + selectedMode[otherNoteIndex];

  // MIDIノートの音高をシャッフルしたコードにする
  makeShuffleChordNotes(sequence, tonic, characteristicNote, otherNote);  
}

// 現在のモードでのドミナントの3和音を作る
// *ドミナントとは、そのモードの「特性音」を含めることを意味する。3和音とは限らない
void makeDominantChord(int tonic, int mode, int *sequence) {
  int characteristicNoteIndex = CHARACTER_TONE[mode];
  int otherNoteIndex = int(random(1, 7));
  while (otherNoteIndex == characteristicNoteIndex) {
    otherNoteIndex = int(random(1, 7));
  }

  // モード中の各音Indexの度数とトニックを合わせる
  int* selectedMode = CHURCH_MODES[mode];
  int characteristicNote = tonic + selectedMode[characteristicNoteIndex];
  int otherNote = tonic + selectedMode[otherNoteIndex];

  // MIDIノートの音高をシャッフルしたコードにする
  makeShuffleChordNotes(sequence, tonic, characteristicNote, otherNote);  
}

//Arduino Nano 本体Lチカ用ユーティリティ
void setupLEDFlash() {
  pinMode(13,OUTPUT);
}

void flashLED(int repeat) {
  for (int i=0; i<repeat; i++) {
    digitalWrite(13,HIGH);
    delay(100);
    digitalWrite(13,LOW);
    delay(100);
  }
}
