const int   maxFileListNum = 24;  //[TODO]
String  bookTxtFileNames[24];
int     fileNum = 0;
int     point[2];


// "books"ディレクトリ内のファイル選択UI
void refreshBookList() {
  File root = SD.open("/books");
  for (int i=0; i<maxFileListNum; i++) {
    bookTxtFileNames[i] = "";
  }
  fileNum = 0;
  while (true) {
    File entry =  root.openNextFile();
    if (!entry)  break;
    String ename = String(entry.name());
    if (!ename.startsWith(".") && ename.endsWith(".txt")) {
      bookTxtFileNames[fileNum] = ename;
      fileNum++;
      if (fileNum >= maxFileListNum)  break;
    }
  }
}

void bookSelectionUI() {
  refreshBookList();  // ファイルリストを再取得
  M5.EPD.Clear(true);
  canvas.fillCanvas(0);
  canvas.setCursor(margin, margin);
  for (int i=0; i<maxFileListNum; i++) {
    if (bookTxtFileNames[i] == "")  break;
    String tmpstr = bookTxtFileNames[i];
    tmpstr.replace(".txt", "");
    canvas.println("  " + tmpstr);
    Serial.println(tmpstr);
  }
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
}
String getSelectedBook() {
  int lineH = fontSize + lineSpace;
  Serial.println(" >>> getSelectedBook starts Tap at Y:" + String(point[1]));
  if ((margin <= point[1]) && (point[1] <= (lineH*(fileNum+1) + margin))) {
    int fileIdx = (point[1]-margin - lineH/2) / lineH;
    if (0<=fileIdx && fileIdx<maxFileListNum) {
      canvas.fillRect(30, fileIdx*lineH+margin, 480, lineH, 15);
      canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
      Serial.println(" >>> Selected File Idx " + String(fileIdx) + " : " + bookTxtFileNames[fileIdx]);
      delay(1000);
      return bookTxtFileNames[fileIdx];
    }
  }
  return "Cancel";
}

// ページジャンププログレスバーの領域定義
int jumpBarAreaLeft = 40;
int jumpBarAreaTop = 380;
int jumpBarAreaW = 460;
int jumpBarAreaH = 180;

void pageJumpUI(int curPos, int fileLen) {
  float curPosF = (float)curPos / (float)fileLen;
  M5.EPD.Clear(true);
  canvas.fillCanvas(0);
  canvas.setCursor(20, 350);
  canvas.print("Tap Position to Jump");
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  canvas.drawRect(jumpBarAreaLeft, jumpBarAreaTop, jumpBarAreaW, jumpBarAreaH, 15);  
  canvas.fillRect(jumpBarAreaLeft, jumpBarAreaTop, (int)((float)jumpBarAreaW * curPosF), jumpBarAreaH, 15);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);  
}
float getTappedJumpPos() {
  Serial.println(" >>> getTappedJumpPos starts Tap at Y:" + String(point[1]));
  float val = -1.0;

  if ((point[1] < jumpBarAreaTop) || (jumpBarAreaTop + jumpBarAreaH) < point[1]) {
    return val;
  } else {
    Serial.println(" >>> Tap to jump at " + String(val));
    canvas.fillRect(jumpBarAreaLeft, jumpBarAreaTop, (point[0]-jumpBarAreaLeft), jumpBarAreaH, 15);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
    delay(1000);
    val = (float)(point[0] - jumpBarAreaLeft) / (float)jumpBarAreaW;
    Serial.println(" >>> Converted Tap Pos to " + String(val*100) + "% of Book");
    if (val < 0.0)  val = 0.0;
    if (val > 1.0)  val = 1.0;
  }
  return val;
}

// 電源オフ画面を表示
void displayPowerOffPage() {
  canvas.drawPngFile(SD, "/resources/offscreen1.png");
  canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
}

// 画面タッチの取得
bool getTouchPoints() {
  bool is_update = false;
  if (M5.TP.avaliable() && !M5.TP.isFingerUp()) {
    M5.TP.update();
    tp_finger_t FingerItem = M5.TP.readFinger(0);
    if ((point[0] != FingerItem.x) || (point[1] != FingerItem.y)) {
      is_update = true;
      point[0] = FingerItem.x;
      point[1] = FingerItem.y;
    }
  }
  return is_update;
}

boolean isFileSelectMenu() {
  boolean val = false;
  if ((100 <= point[0]) && (point[0] <= 440) && point[1] <= 100) {
    val = true;
  }
  return val;
}
boolean isPageJumpMenu() {
  boolean val = false;
  if ((100 <= point[0]) && (point[0] <= 440) && 800<point[1]) {
    val = true;
  }
  return val;
}

boolean isNextPageTap() {
  boolean val = false;
  if (240 <= point[0]) {
    val = true;
  }
  return val;
}
boolean isPrevPageTap() {
  boolean val = false;
  if (point[0] < 240) {
    val = true;
  }
  return val;
}
