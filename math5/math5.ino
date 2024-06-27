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
      addSymbolToDisplay('b');
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
    case 0xFFC23D: 
      Serial.println("FAST FORWARD");
      addSymbolToDisplay(' ');
      break;
    case 0xFFE01F: 
      Serial.println("DOWN");
      addSymbolToDisplay('^');
      break;
    case 0xFFA857: 
      Serial.println("VOL-"); 
      addSymbolToDisplay('k');
      break;
    case 0xFF906F: 
      Serial.println("UP"); 
      addSymbolToDisplay('x');
      break;
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
  if (enteredDigits[0] == 's' || enteredDigits[0] == 'c' || enteredDigits[0] == 't' || enteredDigits[0] == 'r' || enteredDigits[0] == 'b' || enteredDigits[0] == 'k') {
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
      case 'b':
        result = evaluateFraction(enteredDigits);
        break;
      case 'k':
        result = integrate(enteredDigits);
        break;
    }
  } else {
    // 数式の評価
    result = evaluateExpression(enteredDigits);
  }

  // 結果をLCDに表示
  lcd.setCursor(0, 1);
  lcd.print("                "); // クリア
  lcd.setCursor(0, 1);
  lcd.print(enteredDigits);
  lcd.print("=");
  lcd.print(result, 2);
  
  // 結果をシリアルモニタに表示
  Serial.print("Result: ");
  Serial.print(enteredDigits);
  Serial.print("=");
  Serial.println(result, 2);

  // 最後の計算結果を保存
  lastResult = result;
}

void sendEquationToPC() {
  // Shift + 2
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press('1');
  delay(400);
  Keyboard.releaseAll();
  
  // 数式をPCに送信
  for (int i = 0; i < strlen(enteredDigits); i++) {
    char c = enteredDigits[i];
    if (c == 'w') {
      Keyboard.print("/");
    } else if (c == '^') {
      // シフトキーと一緒に5を押して^を送信
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('5');
      delay(400);
      Keyboard.releaseAll();
    } else if (c == 'r') {
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('2');
      delay(400);
      Keyboard.releaseAll();
      Keyboard.print("sqrt ");
      delay(400);
    } else if (c == 's') {
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('2');
      delay(400);
      Keyboard.releaseAll();
      Keyboard.print("sin ");
      delay(400);
    } else if (c == 'b') {
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('2');
      delay(400);
      Keyboard.releaseAll();
      delay(500);
      Keyboard.print("frac");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('3');
      delay(400);
      Keyboard.releaseAll();
      i++; // 'b'の次の文字に進む
      while (enteredDigits[i] == ' ') { // スペースを飛ばす
        i++;
      }
      // 分子を取得
      while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
        Keyboard.print(enteredDigits[i]);
        i++;
      }
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('4');
      delay(400);
      Keyboard.releaseAll();
      while (enteredDigits[i] == ' ') { // 分母の前のスペースを飛ばす
        i++;
      }
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('3');
      delay(400);
      Keyboard.releaseAll();
      // 分母を取得
      while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
        Keyboard.print(enteredDigits[i]);
        i++;
      }
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('4');
      delay(400);
      Keyboard.releaseAll();
      continue; // ループの残りをスキップして次に進む
       } else if (c == 'k') { //積分処理
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('2');
            delay(400);
            Keyboard.releaseAll();
            delay(700);
            Keyboard.print("int");
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('7');
            delay(400);
            Keyboard.releaseAll();
            delay(400);
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('3');
            delay(400);
            Keyboard.releaseAll();
            i++; // 'k'の次の文字に進む
            while (enteredDigits[i] == ' ') { // スペースを飛ばす
              i++;
              i++;
              i++;
            }
            while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
              Keyboard.print(enteredDigits[i]);
              i++;
            }
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('4');
            delay(400);
            Keyboard.releaseAll();
            // シフトキーと一緒に5を押して^を送信
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('5');
            delay(400);
            Keyboard.releaseAll();
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('3');
            delay(400);
            Keyboard.releaseAll();

            // 上限を取得
            while (enteredDigits[i] == ' ') { // スペースを飛ばす
              i++;
            }
            while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
              Keyboard.print(enteredDigits[i]);
              i++;
            }
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('4');
            delay(400);
            Keyboard.releaseAll();
    
            for (int j = 2; enteredDigits[j] != '\0' && enteredDigits[j] != ' '; j++) {
              
              Keyboard.print(enteredDigits[j]);
            }
          
            Keyboard.print("dx");
            continue; // ループの残りをスキップして次に進む
          } else if (c == ' ') {
            Keyboard.print(' ');
          } else {
            Keyboard.print(c);
          }
        }

  // 結果をPCに送信
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press('6');
  delay(100);
  Keyboard.releaseAll(); // 少しの間押し続ける
  Keyboard.print(lastResult);
  Serial.println("FUNC/STOP"); 

  // エンターキーを送信
  Keyboard.write(KEY_RETURN);
  
  // 入力内容をリセット
  //resetDisplay();
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

// 数式の評価
float evaluateExpression(String expr) {
  int startIdx, endIdx;
  while ((startIdx = expr.lastIndexOf('(')) != -1) {
    endIdx = expr.indexOf(')', startIdx);
    if (endIdx == -1) {
      lcd.setCursor(0, 1);
      lcd.print("Err: mismatch ()");
      Serial.println(": error (mismatched parentheses)");
      return NAN;
    }
    String subExpr = expr.substring(startIdx + 1, endIdx);
    float subResult = evaluateExpression(subExpr);
    expr = expr.substring(0, startIdx) + String(subResult, 6) + expr.substring(endIdx + 1);
  }
  return evaluateAddSub(expr);
}

// 足し算と引き算の評価
float evaluateAddSub(String expr) {
  int idx = 0;
  float result = evaluateMulDivPow(expr, idx);
  while (idx < expr.length()) {
    char op = expr.charAt(idx);
    if (op == '+' || op == '-') {
      idx++;
      float nextTerm = evaluateMulDivPow(expr, idx);
      result = (op == '+') ? result + nextTerm : result - nextTerm;
    } else {
      idx++;
    }
  }
  return result;
}

// 乗算、除算、累乗の評価
float evaluateMulDivPow(String expr, int &idx) {
  float result = evaluateFactor(expr, idx);
  while (idx < expr.length()) {
    char op = expr.charAt(idx);
    if (op == '*' || op == '/' || op == '^') {
      idx++;
      float nextFactor = evaluateFactor(expr, idx);
      if (op == '*') result *= nextFactor;
      else if (op == '/') result /= nextFactor;
      else if (op == '^') result = pow(result, nextFactor);
    } else {
      break;
    }
  }
  return result;
}

// 数値と関数の評価
float evaluateFactor(String expr, int &idx) {
  while (idx < expr.length() && expr.charAt(idx) == ' ') idx++;
  if (idx < expr.length() && (expr.charAt(idx) == '+' || expr.charAt(idx) == '-')) {
    char sign = expr.charAt(idx++);
    float result = evaluateFactor(expr, idx);
    return (sign == '-') ? -result : result;
  }
  if (idx < expr.length() && isDigit(expr.charAt(idx))) {
    int start = idx;
    while (idx < expr.length() && (isDigit(expr.charAt(idx)) || expr.charAt(idx) == '.')) idx++;
    return expr.substring(start, idx).toFloat();
  }
  if (idx < expr.length() - 3 && expr.substring(idx, idx + 3) == "sin") {
    idx += 3;
    return sin(evaluateFactor(expr, idx) * PI / 180.0);
  } else if (idx < expr.length() - 3 && expr.substring(idx, idx + 3) == "cos") {
    idx += 3;
    return cos(evaluateFactor(expr, idx) * PI / 180.0);
  } else if (idx < expr.length() - 3 && expr.substring(idx, idx + 3) == "tan") {
    idx += 3;
    return tan(evaluateFactor(expr, idx) * PI / 180.0);
  } else if (idx < expr.length() - 4 && expr.substring(idx, idx + 4) == "sqrt") {
    idx += 4;
    return sqrt(evaluateFactor(expr, idx));
  } else if (expr.charAt(idx) == 'b') {
    return evaluateFraction(expr);
  }
  return NAN;
}

// 分数形式を評価する関数
float evaluateFraction(String input) {
  int firstSpace = input.indexOf(' ', 2); // "b "の次のスペースを見つける
  int secondSpace = input.indexOf(' ', firstSpace + 1);

  float numerator = input.substring(2, firstSpace).toFloat();
  float denominator = input.substring(firstSpace + 1).toFloat();

  return numerator / denominator;
}

// 数式を数値積分する関数（台形法）
float integrate(String input) {
  int firstSpace = input.indexOf(' ', 2); // "k "の次のスペースを見つける
  int secondSpace = input.indexOf(' ', firstSpace + 1);

  String function = input.substring(2, firstSpace);
  float lower = input.substring(firstSpace + 1, secondSpace).toFloat();
  float upper = input.substring(secondSpace + 1).toFloat(); // "=" を無視

  const int n = 1000; // 分割数
  float h = (upper - lower) / n; // 区間の幅
  float sum = 0.0;

  for (int i = 0; i <= n; i++) {
    float x = lower + i * h;
    float y = evaluateFunctionAtX(function, x);
    if (i == 0 || i == n) {
      sum += y / 2.0;
    } else {
      sum += y;
    }
  }
  return sum * h;
}

// 関数の式と x の値を受け取って評価する関数
float evaluateFunctionAtX(String function, float x) {
  String expr = function;
  expr.replace("x", String(x, 6)); // 変数を置換
  return evaluateExpression(expr);
}

// 対数の計算
float calculateLog(String input) {
  int firstSpace = input.indexOf(' ', 2); // "l "の次のスペースを見つける
  int secondSpace = input.indexOf(' ', firstSpace + 1);

  float base = input.substring(2, firstSpace).toFloat();
  float value = input.substring(firstSpace + 1).toFloat(); // "=" を含むすべてを値として取得

  return log(value) / log(base); // 対数の変換公式を使用
}

// 逆三角関数を評価する関数
float evaluateInverseTrig(String input) {
  String type = input.substring(0, 4); // "asin", "acos", "atan"
  float value = input.substring(4).toFloat();

  if (type == "asin") {
    return asin(value) * 180.0 / PI; // ラジアンを度に変換
  } else if (type == "acos") {
    return acos(value) * 180.0 / PI; // ラジアンを度に変換
  } else if (type == "atan") {
    return atan(value) * 180.0 / PI; // ラジアンを度に変換
  }
  return NAN;
}
