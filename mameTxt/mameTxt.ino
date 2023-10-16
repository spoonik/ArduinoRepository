#include <M5EPD.h>

M5EPD_Canvas canvas(&M5.EPD);
int dispW = 540;
int dispH = 960;
int margin = 15;
const int fontSize = 35;
const int lineSpace = 4;

// [TODO] 仮の決め打ち定数
int onePageLenMax = 1000;
int oneFileLenMax = 1000000;
uint8_t *textFileContent;

// グローバル変数
int currentPos = 0;
int curPageLen = 0;
int fileLength = 0;
boolean bookSelectingUIMode = false;
boolean pageJumpUIMode = false;
String curFileName = "KafkaDiaries.txt";


// 1ページを画面に入るだけ表示して、ページ長を計算する
void displayPage(uint8_t *textFileContent, int currentPos) {
  M5.EPD.Clear(true);
  canvas.fillCanvas(0);
  canvas.createRender(fontSize, 2048);
  canvas.setTextLineSpace(lineSpace);

  String text = textSubstring(textFileContent, &currentPos, onePageLenMax);
  curPageLen = onePageLenMax;

  canvas.setCursor(margin, margin);

  //デバイス上ではみだす文字量をページ文字数とする描画処理
  int wordStart = 0;
  int wordEnd = 0;
  while (wordStart <= text.length()) {
    if (canvas.getCursorX() + 1 >= dispW-margin*2) {
      canvas.println();
    }
    if (canvas.getCursorY() >= dispH-margin*2) {
      break;
    }
    canvas.print(text[wordStart]);
    curPageLen = wordStart;
    wordStart++;
  }
  Serial.println("  cursor: " + String(currentPos) + ", PageLen: " + String(curPageLen));
  canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
  canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);

  storeCursorPosSD(curFileName, currentPos);
}

void setup() {
  M5.begin();
  M5.TP.SetRotation(90);
  M5.EPD.SetRotation(90);
  M5.EPD.Clear(true);

  // ブックを開いて表示
  curFileName = getCurrentBookFileName();
  openBook(curFileName, &textFileContent, &fileLength, &currentPos);

  // 表示領域を設定し、SDからフォントをロード
  setCanvas();
  loadFontFor(curFileName);

  displayPage(textFileContent, currentPos);
}

void loop() {
  delay(500);
  M5.update();  //ボタン状態取得に必要

  if (bookSelectingUIMode) {
    // ファイル選択UI中のタッチ処理
    if (getTouchPoints()) {
      bookSelectingUIMode = false;
      String newBook = getSelectedBook();
      if (newBook != "Cancel") {
        curFileName = newBook;
        openBook(curFileName, &textFileContent, &fileLength, &currentPos);
        delay(500);
        storeCurFileName(curFileName);
      }
      displayPage(textFileContent, currentPos);
    }
  } else if (pageJumpUIMode) {
    // ページジャンプUI中のタッチ処理
    if (getTouchPoints()) {
      float percentage = getTappedJumpPos();
      if (percentage >= 0.0) {
        pageJumpUIMode = false;
        currentPos = (int)((float)fileLength * percentage);
        currentPos = min(currentPos, fileLength - curPageLen);
        Serial.println("Jumping to " + String(percentage) + "(%) of Book");
        delay(500);
        displayPage(textFileContent, currentPos);
      }
    }
  } else {
    // 通常のテキストページ表示
    if (M5.BtnL.wasPressed()) {
      currentPos = max(0, currentPos - curPageLen);
      displayPage(textFileContent, currentPos);
    }
    else if (M5.BtnR.wasPressed()) {
      currentPos = min(fileLength - curPageLen, currentPos + curPageLen);
      displayPage(textFileContent, currentPos);
    }
    else if (M5.BtnP.wasPressed()) {
      displayPowerOffPage();
      delay(1000);
      M5.shutdown();
    }
    else if (getTouchPoints()) {
      if (isFileSelectMenu()) {    // ファイル選択UIを表示
        bookSelectingUIMode = true;
        bookSelectionUI();
      } else if (isPageJumpMenu()) { // ページジャンプUIを表示
        pageJumpUIMode = true;
        pageJumpUI(currentPos, fileLength);
      } else if (isPrevPageTap()) {
        currentPos = max(0, currentPos - curPageLen);
        displayPage(textFileContent, currentPos);
      } else if (isNextPageTap()) {
        currentPos = min(fileLength - curPageLen, currentPos + curPageLen);
        displayPage(textFileContent, currentPos);
      }
    }
  }
}
