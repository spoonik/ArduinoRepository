/* ---------------- 乱数を使ったMIDIシーケンス生成用の中心コード ---------------- */

#define LOWEST_NOTE 54  // これ以上
#define HIGHEST_NOTE 66 // これ以下の範囲のMIDIノートをトニックに使う
int tick_cnt = 0;


// 教会旋法(モード)のスケール定義(音の間の間隔)
// Indexの順番を、使い易い順に並べてある
const int CHURCH_MODES[][7] = {
  {0, 2, 4, 5, 7, 9, 11},  // Ionian (メジャースケール)
  {0, 2, 3, 5, 7, 8, 10},  // Aeolian (ナチュラルマイナースケール)
  {0, 2, 3, 5, 7, 9, 10},  // Dorian
  {0, 2, 4, 6, 7, 9, 11},  // Lydian
  {0, 2, 4, 5, 7, 9, 10},  // Mixolydian
  {0, 1, 3, 5, 7, 8, 10}   // Phrygian
  //{0, 1, 3, 5, 6, 8, 10}   // Locrianは省略
};
const int numModes = 6;

// 各モードの主音を、Ionian(Index=0)からのMIDI Note距離で定義
const int TONIC_TONE_DISTANCE[] = {
  0,  // Ionian (メジャースケール)
  9,  // Aeolian (ナチュラルマイナースケール)
  2,  // Dorian
  5,  // Lydian
  7,  // Mixolydian
  4   // Phrygian
  //11   // Locrianは省略
};

// 各モードの特性音 (そのモードの特徴を示す音。配列中の音、0-Origin)
const int CHARACTER_TONE[] = {
  3,  // Ionian (メジャースケール)
  5,  // Aeolian (ナチュラルマイナースケール)
  5,  // Dorian
  3,  // Lydian
  6,  // Mixolydian
  1   // Phrygian
  //1   // Locrianは省略
};


// 現在の状態を保持するグローバル変数 (TODO)
volatile int currentTonic = 60; // 現在の中心音MIDIノート番号（例: ドの場合は60）
volatile int currentModeIndex = 0;


// 偏りを持たせたランダムネスで新しい3音からなるモードを生成
// ・教会旋法モード切り替えか
// ・転旋を切り替える
void generateChord() {
  float randomValue = random(0, 100);
  if (randomValue < 10) {
    // 10%の確率で転旋＝モードを変更する(主音は同じ)
    changeMode();
    tick_cnt = 0;
  } else if (randomValue < 15) {
    // さらに5%の確率で転調＝現トニックから、正規分布に沿って5度圏で転調
    changeTonic();
    tick_cnt = 0;
  }

  // デフォルトでトニックの3和音を生成
  makeTonicThreeChord(currentTonic, currentModeIndex, sequence);
  if (tick_cnt%2 != 0) {
    // トニックと交互に和音の変化をつける
    if (randomValue < 50) {  // 50%はサブドミナントコードを生成
      makeSubDominantThreeChord(currentTonic, currentModeIndex, sequence);
    } else {    // それ以外はドミナントコードを生成
      makeDominantChord(currentTonic, currentModeIndex, sequence);
    }
  }  
  tick_cnt++;
}
