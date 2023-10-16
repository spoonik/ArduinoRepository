#include "SD.h"
#include "FS.h"

String  currentBook = "/lastbook.txt";

// フォントのロード処理
const char *font_mincho = "/resources/fonts/aozora_mincho_med.ttf";
const char *font_ja = "/resources/fonts/ipa_gothic.ttf";
const char *font_en = "/resources/fonts/mplus1_med.ttf";
const char *font_ui = "/resources/fonts/mplus1_ui.ttf";


// 1ページ分くらいのテキストの一部を取り出す
String textSubstring(uint8_t *textFileContent, int *startPtr, int len) {
  // 日本語UTF8対応のため、先頭2ビット目が1のバイト文字はスキップする
  while (!((textFileContent + *startPtr)[0] & 0x40) && ((textFileContent + *startPtr)[0] & 0x80)) {
    (*startPtr)++;
    len--;
  }
  int endPtr = min(fileLength, *startPtr+len);
  while (!((textFileContent + endPtr)[0] & 0x40) && ((textFileContent + endPtr)[0] & 0x80)) {
    endPtr--;
  }
  endPtr--; //もう一つ戻しておかないと、先頭バイトだけが引っかかる
  char buf[1000];
  memcpy(buf, (char*)(textFileContent + *startPtr), endPtr-*startPtr);
  return String(buf);
}

// ブックを開いてメモリに読み込む
void openBook(String &filename, uint8_t **textFileContent, int *fileLength, int *currentPos) {
  if (*textFileContent != 0) {
    free(*textFileContent);
  }
  if (!SD.exists("/books/" + filename)) {
    filename = "KafkaDiaries.txt";
  }
  *textFileContent = (uint8_t*)ps_malloc(oneFileLenMax);
  auto f = SD.open("/books/" + filename, "rb");
  *fileLength = min(f.available(), oneFileLenMax-1);
  f.read(*textFileContent, *fileLength);
  *currentPos = getCursorPosSD(filename);

  Serial.println("New Book len:" + String(*fileLength) + ", cursor:" + String(*currentPos));
}

//ブックマークの保存管理処理
void storeCursorPosSD(String filename, uint32_t c) {
  auto f = SD.open("/bookmarks/" + filename, "wb");
  uint8_t buf[4];
  buf[0] = c;
  buf[1] = c >> 8;
  buf[2] = c >> 16;
  auto bytes = f.write(&buf[0], 4);
  f.close();
}
void storeCurFileName(String filename) {
  auto f = SD.open(currentBook, FILE_WRITE);
  auto bytes = f.write((uint8_t*)filename.c_str(), filename.length()+1);
  f.close();
}
uint32_t getCursorPosSD(String filename) {
  uint32_t val = 0;
  if (SD.exists("/bookmarks/" + filename)) {
    auto f = SD.open("/bookmarks/" + filename, "rb");
    val = f.read();
    f.close();
  }
  return val;
}
String getCurrentBookFileName() {
  String val = "KafkaDiaries.txt";  //[TODO]

  if (SD.exists(currentBook)) {
    uint8_t *filenameBuf;
    filenameBuf = (uint8_t*)ps_malloc(256);
    auto f = SD.open(currentBook, "rb");
    int filelen = min(f.available(), 256-1);
    f.read(filenameBuf, filelen);
    val = String((char*)filenameBuf);
    Serial.println("Last Opened Book was " + String(val));
    f.close();
  }
  return val;
}

// 文字表示領域を設定し、フォントファイルを開く
void setCanvas() {
  canvas.createCanvas(dispW, dispH);
  canvas.setTextArea(margin, margin, dispW-margin*2, dispH-margin*4); //[TODO]
  canvas.setTextDatum(TC_DATUM);

  canvas.drawString("Loading...", dispW/2, 400);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
}
void loadFontFor(String filename) {
  //[TODO]
  if (true) {  //filename.indexOf("_ja.txt") != -1) {
    canvas.loadFont(font_mincho, SD);
  } else {
    canvas.loadFont(font_en, SD);
  }
}
