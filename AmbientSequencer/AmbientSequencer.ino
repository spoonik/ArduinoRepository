/* Minimum Generative Ambient Sequencer for Original Arduino Nano */
#include <MsTimer2.h>
#define MIDI_CH  1
#define MIDI_ON  0x90|(MIDI_CH - 1)
#define MIDI_OFF 0x80|(MIDI_CH - 1)

#define MINIMUM_TIMER_TICKTIME 5

// 各音符の発音のタイミング＝旋律のリズム・テンポ(msec)
#define TEMPO_NOTE_DELAY 2000

// 最大3音までのモードを生成
const int MAX_NOTE_COUNT = 3;


// 状態管理用変数
int currentStep = 0;
int sequence[4];  //TODO: currentStepのカウントがうまくいってないバグを回避、安直・・・
int noteOnDelay = TEMPO_NOTE_DELAY;
volatile int timerTickCounter = 0;
void updateCurrentTick() {
  timerTickCounter = timerTickCounter + MINIMUM_TIMER_TICKTIME;
}
void setupSequencer() {
  // 固定の10msecを間隔としてTimerを開始
  // On/Off/テンポの変更は、この10msec間隔の倍数で制御
  MsTimer2::set(MINIMUM_TIMER_TICKTIME, updateCurrentTick);
  MsTimer2::start();

  // 初回のコードを生成しておく
  generateChord();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setupLEDFlash();

  // MIDI(シリアル通信)を初期化し、以前の鳴りっぱなしを回避用パニック
  setupSerialMIDI();
  MIDIPanic();

  // シーケンサを初期化
  setupSequencer();
}

void loop() {
  if (timerTickCounter > (noteOnDelay * ((currentStep+1)*2))) {
    if (currentStep == 0) {
      STOP_ALL_MIDI();
      delay(1000);
    }
    SEND_MIDI(MIDI_ON, sequence[currentStep], true);
    flashLED(1);  // 発音時にLチカ

    // 少しずつテンポを遅くする
    if (noteOnDelay < 20000) {
      noteOnDelay += 5;
    }

    // 次のノートに進める
    if (++currentStep > MAX_NOTE_COUNT) {
      // カウンタクリアし、新しいコードを生成
      timerTickCounter = 0;
      currentStep = 0;
      generateChord();
    }
  }
}
