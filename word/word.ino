#include <LiquidCrystal.h>
#include <IRremote.h>
#include <HID-Project.h>
#include <math.h>

// LCDの初期化
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// IR受信機
int receiver = 3; // IR受信機の信号ピンをArduinoのデジタルピン3に接続
IRrecv irrecv(receiver); // 'irrecv'のインスタンスを作成
decode_results results; // 'decode_results'のインスタンスを作成

const int maxDigits = 16; // 最大表示桁数
char enteredDigits[maxDigits + 1] = ""; // 入力された数値を保存する配列
float lastResult = 0; // 最後の計算結果を保存

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
    case 0xFF629D: 
      Serial.println("VOL+"); 
      addSymbolToDisplay('+');
      break;
    case 0xFF22DD: 
      Serial.println("FAST BACK"); 
      calculateAndDisplayResult();
      break;
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
    case 0xFF9867: 
      Serial.println("EQ"); 
      addSymbolToDisplay('s');
      break;
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
}

void calculateAndDisplayResult() {
  float result = 0;
  if (enteredDigits[0] == 's' || enteredDigits[0] == 'c' || enteredDigits[0] == 't' || enteredDigits[0] == 'r') {
    char func = enteredDigits[0];
    float value = atof(enteredDigits + 1);
    switch (func) {
      case 's':
        result = sin(value * PI / 180.0);
        break;
      case 'c':
        result = cos(value * PI / 180.0);
        break;
      case 't':
        result = tan(value * PI / 180.0);
        break;
      case 'r':
        result = sqrt(value);
        break;
    }
  } else {
    // 数値と操作子を格納するリスト
    float operands[10]; // 最大10個の数値を処理
    char operators[10]; // 数値間の演算子を格納
    int operandCount = 0; // 数値の数
    int operatorIndex = -1; // 現在の演算子のインデックス
    int startIndex = 0;
    int endIndex = 0;

    while (endIndex <= strlen(enteredDigits)) {
      if (endIndex == strlen(enteredDigits) || enteredDigits[endIndex] == ' ' || isOperator(enteredDigits[endIndex])) {
        if (startIndex < endIndex) {
          operands[operandCount++] = atof(enteredDigits + startIndex);
          if (endIndex < strlen(enteredDigits) && isOperator(enteredDigits[endIndex])) {
            operators[++operatorIndex] = enteredDigits[endIndex];
          }
        }
        startIndex = endIndex + 1;
      }
      endIndex++;
    }

    result = operands[0];
    for (int i = 0; i <= operatorIndex; i++) {
      switch (operators[i]) {
        case '+':
          result += operands[i + 1];
          break;
        case '-':
          result -= operands[i + 1];
          break;
        case '*':
          result *= operands[i + 1];
          break;
        case '/':
          if (operands[i + 1] != 0) {
            result /= operands[i + 1];
          } else {
            lcd.setCursor(0, 1);
            lcd.print("Error: Div by 0");
            Serial.println("Error: Division by zero");
            return;
          }
          break;
        case '^':
          result = pow(operands[i], operands[i + 1]);
          break;
        default:
          lcd.setCursor(0, 1);
          lcd.print("Error: Invalid Op");
          Serial.println("Error: Invalid operator");
          return;
      }
    }
  }

  // 結果をLCDに表示
  lcd.setCursor(0, 1);
  lcd.print("                "); // クリア
  lcd.setCursor(0, 1);
  lcd.print(enteredDigits);
  lcd.print("=");
  lcd.print(result);
  
  // 結果をシリアルモニタに表示
  Serial.print("Result: ");
  Serial.print(enteredDigits);
  Serial.print("=");
  Serial.println(result);

  // 最後の計算結果を保存
  lastResult = result;
}

void sendEquationToPC() {
  // Shift + 2
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press('2');
  delay(400);
  Keyboard.releaseAll();
  
  // 数式をPCに送信
  for (int i = 0; i < strlen(enteredDigits); i++) {
    char c = enteredDigits[i];
    if (c == 'w') {
      Keyboard.print("/");
    } else if (c == '^') {
      // シフトキーと一緒に3を押して^を送信
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press('3');
      delay(400);
      Keyboard.releaseAll();
    } else if (c == 'r') {
      Keyboard.press(KEY_F8);
      delay(400);
      Keyboard.print("sqrt ");
      delay(400);
    }else if (c == 's') {
      Keyboard.press(KEY_F8);
      delay(400);
      Keyboard.print("sin ");
      delay(400);
    } else {
      Keyboard.print(c);
    }
  }

  // 結果をPCに送信
  Keyboard.press(KEY_HOME);
   delay(400); // 少しの間押し続ける
  Keyboard.print(lastResult);

  // エンターキーを送信
  Keyboard.write(KEY_RETURN);
  
  // 入力内容をリセット
  resetDisplay();
}

bool isOperator(char c) {
  return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn(); // 受信機を開始
  lcd.begin(16, 2);
  lcd.print("Number entered.");
  Keyboard.begin(); // HIDキーボードの初期化
}

void loop() {
  if (irrecv.decode(&results)) { // IR信号を受信したか？
    translateIR(); 
    irrecv.resume(); // 次の値を受信
  } else {
    delay(100);
  }
}
