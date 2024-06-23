#include <LiquidCrystal.h>
#include <IRremote.h>
#include <HID-Project.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int receiver = 3; // IR受信機の信号ピンをArduinoのデジタルピン3に接続

IRrecv irrecv(receiver);     // 'irrecv'のインスタンスを作成
decode_results results;      // 'decode_results'のインスタンスを作成

const int maxDigits = 16;  // 最大表示桁数
char enteredDigits[maxDigits + 1] = ""; // 入力された数値を保存する配列
bool waitingForExponent = false; // 次に入力されるのが指数かどうかを示すフラグ

void translateIR() {
  switch(results.value) {
    case 0xFFA25D: 
      Serial.println("POWER");
      sendEquationToPC();
      break;
    case 0xFFE21D: 
      Serial.println("FUNC/STOP"); 
      addSymbolToDisplay('w');
      break;
    case 0xFF629D: Serial.println("VOL+"); break;
    case 0xFF22DD: Serial.println("FAST BACK"); break;
    case 0xFF02FD: 
      Serial.println("PAUSE"); 
      addSymbolToDisplay('r');
      break;
    
    case 0xFFC23D: Serial.println("FAST FORWARD"); break;
    case 0xFFE01F: 
      Serial.println("DOWN");
      addSymbolToDisplay('^');
      break;
    case 0xFFA857: Serial.println("VOL-"); break;
    case 0xFF906F: Serial.println("UP"); break;
    case 0xFF9867: Serial.println("EQ"); break;
    case 0xFFB04F: 
      Serial.println("ST/REPT"); 
      resetDisplay();
      break;
    case 0xFF6897: addDigitToDisplay('0'); break;
    case 0xFF30CF: addDigitToDisplay('1'); break;
    case 0xFF18E7: addDigitToDisplay('2'); break;
    case 0xFF7A85: addDigitToDisplay('3'); break;
    case 0xFF10EF: addDigitToDisplay('4'); break;
    case 0xFF38C7: addDigitToDisplay('5'); break;
    case 0xFF5AA5: addDigitToDisplay('6'); break;
    case 0xFF42BD: addDigitToDisplay('7'); break;
    case 0xFF4AB5: addDigitToDisplay('8'); break;
    case 0xFF52AD: addDigitToDisplay('9'); break;
    case 0xFFFFFFFF: Serial.println("REPEAT"); break;
    default: Serial.println("other button");
  }

  delay(500); // 即座のリピートを防ぐ
} 

void addDigitToDisplay(char digit) {
  int length = strlen(enteredDigits);
  if (length < maxDigits) {
    enteredDigits[length] = digit;
    enteredDigits[length + 1] = '\0';
    lcd.setCursor(0, 1);
    lcd.print(enteredDigits);
  }
}

void addSymbolToDisplay(char symbol) {
  int length = strlen(enteredDigits);
  if (length < maxDigits) {
    enteredDigits[length] = symbol;
    enteredDigits[length + 1] = '\0';
    lcd.setCursor(0, 1);
    lcd.print(enteredDigits);
  }
}

void resetDisplay() {
  enteredDigits[0] = '\0'; // 配列をクリア
  lcd.setCursor(0, 1);
  lcd.print("                "); // LCDの2行目をクリア
  lcd.setCursor(0, 1);
  waitingForExponent = false; // フラグをリセット
}

void sendEquationToPC() {
  // Shift + 2
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press('2');
  delay(400);
  Keyboard.releaseAll();
  
  // 数式をPCに送信
  bool foundSlash = false;
  bool inFraction = false;
  for (int i = 0; i < strlen(enteredDigits); i++) {
    char c = enteredDigits[i];
    if (c == 'w') {
      Keyboard.print("/");
      foundSlash = true;
    } else if (c == '^') {
      // シフトキーと一緒に3を押して^を送信
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press('3');
      delay(400);
      Keyboard.releaseAll();
    } else if (c == 'r') {
      // 'r'を見つけたら、\sqrtを送信
      Keyboard.press(KEY_F8);
      delay(400);
      Keyboard.print("sqrt ");
    } else {
      Keyboard.print(c);
    }
  }
  
  if (inFraction) {
    Keyboard.print("}");
  }
  
  // エンターキーを送信
  Keyboard.write(KEY_RETURN);
  
  // 入力内容をリセット
  resetDisplay();
}

void setup() {
  // 無線
  Serial.begin(9600);
  Serial.println("IR Receiver Button Decode"); 
  irrecv.enableIRIn(); // 受信機を開始
  // ディスプレイ
  lcd.begin(16, 2);
  lcd.print("Number entered.");
  // HIDの初期化
  Keyboard.begin();
}

void loop() {
  if (irrecv.decode(&results)) { // IR信号を受信したか？
    translateIR(); 
    irrecv.resume(); // 次の値を受信
  } else {
    delay(100);
  }
}
