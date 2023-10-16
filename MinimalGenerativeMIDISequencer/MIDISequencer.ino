/* ノートの細かさ(全音符〜16分音符) 
 *  モードの音数とを与えて
 *  MIDI音列を生成する
 */
#include <MsTimer2.h>

#define MIDI_CH  1
#define MIDI_ON  0x90|(MIDI_CH - 1)
#define MIDI_OFF 0x80|(MIDI_CH - 1)


const int MINIMUM_TICKTIME = 2;
volatile int seqDelayTime = 1500;
volatile int timerTickCounter = 0;

volatile bool isPlayingSeq = false;
volatile bool MIDI_beeping = false;
volatile byte beepingMIDINote[2];
volatile int seqToneNum = 2;   // [TODO]基本2音の和音を鳴らす


volatile byte maxModeNoteNum = 1;
volatile int bpm = 20;
volatile byte currentStep = 0;


const byte MAX_NOTE_COUNT = 16;
const byte MAX_AVAILABLE_NOTE_NUM = 10;
const byte C4 = 60;   // MIDI C4 NOTE

volatile byte sequenceArray[] = {
  C4, C4, C4, C4,
  C4, C4, C4, C4,
  C4, C4, C4, C4,
  C4, C4, C4, C4
};



void setupMIDISequencer() {
  currentStep = 0;

  // MIDI信号処理を初期化
  setupMIDISignal();

  // BPM関連変数を再計算しておき
  resetTempo(bpm);

  // 固定の10msecを間隔としてTimerを開始
  // On/Off/テンポの変更は、この10msec間隔の倍数で制御
  MsTimer2::set(MINIMUM_TICKTIME, updateCurrentTick);
  MsTimer2::start();
}

void updateCurrentTick() {
  timerTickCounter = timerTickCounter + MINIMUM_TICKTIME;
}

int changeTempo(int change) {
  if (change != 0) {
    if (change > 0) {
      bpm = (bpm * 115) / 100;
    } else {
      bpm = (bpm * 75) / 100;
    }
    bpm = min(max(bpm, 10), 540);   //16分音符換算も考えてむっちゃ速いBPMもあり

    // Timer処理用の間隔値を変更
    resetTempo(bpm);
    Serial.print("BPM: ");
    Serial.println(bpm);
  }
  return bpm;
}

bool togglePlayingSeq() {
  isPlayingSeq = !isPlayingSeq;
  Serial.print("Sequence is now ");
  Serial.println(isPlayingSeq);
}

void resetTempo(int newbpm) {
  timerTickCounter = 0;
  //30secをBPMで割って、音のOn/Offのタイミングを決める
  //(=1拍の半分音を鳴らし、半分止める)
  seqDelayTime = int(30000 / newbpm);
}


/* 基本的なモードは「ドリアンモード」とし、
 * DORIANの中からシーケンス音列に使える音を
 * だんだん増やしていき、複雑なスケールからランダムに
 * シーケンス音列を生成するようなGenerativeな仕組みとする
 */
const byte DORIAN_MODE_SCALE[] = {
  0, 7, 3, 5, 2, 9, 10, 4, 11, 1
};
byte changeModeNoteNum(byte increasedNoteNum) {
  byte old_maxModeNoteNum = maxModeNoteNum;
  if (increasedNoteNum != 0) {
    maxModeNoteNum += increasedNoteNum;
    maxModeNoteNum = min(max(maxModeNoteNum, 1), 10);
    if (old_maxModeNoteNum != maxModeNoteNum) {
      reGenerateMIDISteps();  // シーケンス音列を更新
    }
  }
  return maxModeNoteNum;
}

void reGenerateMIDISteps() {
  Serial.print("New Sequence:");

  for (byte i=0; i<MAX_NOTE_COUNT; i++) {
    if (maxModeNoteNum > 1) {
      byte note_idx = byte(random(maxModeNoteNum));
      sequenceArray[i] = C4 + DORIAN_MODE_SCALE[note_idx];
    } else {
      sequenceArray[i] = C4;
    }
    Serial.print(" ");
    Serial.print(sequenceArray[i]);
  }
  Serial.println();
}

// シーケンスの音列からたまに音をランダムに抜かして再生する
bool loopPlaySequenceNote() {
  if (isPlayingSeq) {
    if (timerTickCounter > seqDelayTime) {
      if (MIDI_beeping) {
        // 今音が鳴っているので止める
        SEND_MIDI(MIDI_OFF, beepingMIDINote[0]);
        if (beepingMIDINote[0] != beepingMIDINote[1]) {
          SEND_MIDI(MIDI_OFF, beepingMIDINote[1]);
        }
        MIDI_beeping = false;
      }
      else {
        // 新しい音を鳴らす
        // まず鳴らす音を決めて
        beepingMIDINote[0] = sequenceArray[currentStep];
        beepingMIDINote[1] = sequenceArray[random(maxModeNoteNum)];
    
        for (int i=0; i<seqToneNum; i++) {
          SEND_MIDI(MIDI_ON, beepingMIDINote[0]);
          if (beepingMIDINote[0] != beepingMIDINote[1]) {
            SEND_MIDI(MIDI_ON, beepingMIDINote[1]);
          }
        }
        MIDI_beeping = true;
      }
      // 次のノートに進める
      currentStep++;
      if (currentStep >= MAX_NOTE_COUNT) {
          currentStep = 0;
      }
      // 次のタイマー処理に備えてカウンタをクリア
      timerTickCounter = 0;
    }
  }
  return isPlayingSeq;
}
