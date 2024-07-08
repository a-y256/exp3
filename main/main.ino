#include <ResKeypad.h>
#include <mglcd.h>
#include <Arduino.h>
#include <HID-Project.h>
#include <math.h>




// A0につながっているキーパッドをkeypad1とする
ResKeypad keypad1(A4, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_A); // SIDE Aに部品を実装した場合はこの行を有効にする

// A1につながっているキーパッドをkeypad2とする
ResKeypad keypad2(A5, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_A); // SIDE Aに部品を実装した場合はこの行を有効にする

char keys[] = {
  '*', '-', '+', '0',
  '=', '3', '2', '1',
  'A', '6', '5', '4',
  'D', '9', '8', '7',
  ')', '(', 'r', 'b',
  't', 'c', 's', 'k',
  '.', 'l', '^', 'x',
  'F', 'G', 'H', 'P'
};

static const TLcdPinAssignTable PinAssignTable = {
  A0_DI  : A3, // A0 for SG12232, D/I for SG12864
  CS1_E1 : 3, // CS1
  CS2_E2 : 2, // CS2
  E      : 13,
  RW     : 12,
  DB0    : 8,
  DB1    : 9,
  DB2    : 10,
  DB3    : 11,
  DB4    : 4,
  DB5    : 5,
  DB6    : 6,
  DB7    : 7
}; // PinAssignTable;
  
#define s ((((((((((((((((0
#define M <<1)+1)
#define _ <<1))
  
PROGMEM const uint8_t UserChars[][5] = {
  { // 80
    s  _ _ _ _ _ _ _ _,
    s  _ _ _ _ _ _ _ _,
    s  M M M M M M M _,
    s  _ _ _ _ _ _ _ M,
    s  _ _ _ _ _ _ M _
  },
  { // 81
    s  _ _ _ _ _ _ _ _,
    s  _ _ _ _ _ _ _ _,
    s  M M M M M M M M,
    s  _ _ _ _ _ _ _ _,
    s  _ _ _ _ _ _ _ _
  },
  { // 82
    s  _ M _ _ _ _ _ _,
    s  M _ _ _ _ _ _ _,
    s  _ M M M M M M M,
    s  _ _ _ _ _ _ _ _,
    s  _ _ _ _ _ _ _ _
  },
  { // 83
    s  _ _ M _ _ _ _ _,
    s  _ _ M _ _ _ _ _,
    s  _ M _ _ _ _ _ _,
    s  M M M M M M M M,
    s  _ _ _ _ _ _ _ _
  },
  { // 84
    s  M _ _ _ _ _ _ _,
    s  M _ _ _ _ _ _ _,
    s  M _ _ _ _ _ _ _,
    s  M _ _ _ _ _ _ _,
    s  M _ _ _ _ _ _ _
  },
  { // 85
    s  _ _ _ M _ _ _ _,
    s  _ _ _ M _ _ _ _,
    s  _ _ _ M _ _ _ _,
    s  _ _ _ M _ _ _ _,
    s  _ _ _ M _ _ _ _
  }
};


int x;
int y;
int i = 1;
int n = 0;
int m = 0;
int xa;
int ya;
int xb;
int yb;
String func = "";
float lastResult = 0;
int sensorPin = 0;
bool sensorTriggered = false;
int ledPin = A0;
int pirValue; // PIRセンサーの値を格納する場所
unsigned long lastMotionTime = 0; // 最後に動きを検出した時間
const unsigned long delayTime = 5000; // 遅延時間（ミリ秒単位、ここでは5秒）



#undef s
#undef M
#undef _

static mglcd_SG12864 MGLCD(PinAssignTable);

char enteredDigits[17] = ""; // 入力された数値を保存する配列
char pressedKey;

void setup() {
  Serial.begin(9600);

  while (MGLCD.Reset()); //LCDの初期化
  MGLCD.UserChars(UserChars, sizeof(UserChars) / 5);
  MGLCD.Locate(0,1);
  pinMode(ledPin, OUTPUT); // LEDピンを出力モードに設定
  pinMode(sensorPin, INPUT); // PIRピンを入力モードに設定
  digitalWrite(ledPin, LOW);
}

void addDigitToDisplay(char digit) {
    int length = strlen(enteredDigits);
    if (length < 17) {
      enteredDigits[length] = digit;
      enteredDigits[length + 1] = '\0';// リストに追加
    }
}

void Digit(String digit){
  MGLCD.print(digit);
}

// 積分
void displayIntegral() {
   MGLCD.print("\x81");
  MGLCD.Locate(x,y-1);
  MGLCD.print("\x80");
  xb = MGLCD.GetX() - 1;
  yb = MGLCD.GetY();
  MGLCD.Locate(x,y+1);
  MGLCD.print("\x82");
  xa = MGLCD.GetX() - 1;
  ya = MGLCD.GetY();
  MGLCD.Locate(xb, yb);
  x = MGLCD.GetX();
  y = MGLCD.GetY();
  MGLCD.Locate(x+1,y-1);
}

// 数式の評価
float evaluateExpression(String expr);
float evaluateAddSub(String expr);
float evaluateMulDivPow(String expr, int &idx);
float evaluateFactor(String expr, int &idx);
float integrateExpression(String input);
float evaluateFunctionAtX(String function, float x);
float calculateLog(String input);
float evaluateFraction(String input);
float evaluateInverseTrig(String input);
int lite=0 ;

// 計算結果を表示する関数
void displayResult(float result) {
    char resultStr[17];
    dtostrf(result, 0, 2, resultStr); 
    MGLCD.Locate(strlen(enteredDigits)+2 , 1); 
    MGLCD.print("=");
    MGLCD.print(resultStr);
}

void loop() {
  
  signed char key; // キー番号
  pirValue = digitalRead(sensorPin); 
  Serial.println(pirValue);
  if (pirValue == 1) {
      lite = 1;
      digitalWrite(ledPin, HIGH); // LEDを点灯
    lastMotionTime = millis();
    }else {
    // 最後に動きを検出してから5秒経過したかどうかをチェック
    if (millis() - lastMotionTime > delayTime) {
      digitalWrite(ledPin, LOW); // LEDを消灯
      lite=0;
    }
  }
  
  key = keypad1.GetKey(); 
  if (key < 0) { // keypad1のキーが押されていなかった場合
    key = keypad2.GetKey(); // 
    if (key >= 0) key += 16; // 
  } 
  if (key >= 0) {
    pressedKey = keys[key];
    // センサーがトリガーされている場合、s, c, tをi, o, aに変換
    if (lite == 1) {
      if (pressedKey == 's') pressedKey = 'i';
      digitalWrite(ledPin, LOW); // LEDを消灯
      lite=0;
      
    }

    // "="が押されたときの処理
    if (pressedKey == '=') {
        if (enteredDigits[0] == 'k') {
            MGLCD.Locate(strlen(enteredDigits)-3, 1);
            MGLCD.print(" dx");
            float result = integrateExpression(String(enteredDigits));
            displayResult(result);
        } else {
            float result = evaluateExpression(String(enteredDigits));
            displayResult(result);
        }
        return;
    }

    if (pressedKey == 'F') {
      sendEquationToPC();
      return;
    }
    
    addDigitToDisplay(pressedKey);
    String Digits = String(enteredDigits);
    String input = Digits; 
    input.trim(); // 

    String str = input.substring(i - 1, i);
   
    x = MGLCD.GetX();
    y = MGLCD.GetY();

    if (func == "k") {
        if (n == 0) {
            MGLCD.Locate(xa + 1, ya);
            xa = MGLCD.GetX();
            ya = MGLCD.GetY();
        } else if (n == 1) {
            MGLCD.Locate(xb + 1, yb);
            xb = MGLCD.GetX();
            yb = MGLCD.GetY();
        } else if (n == 2) {
            if (xa <= xb) {
                MGLCD.Locate(xb, yb + 1);
            } else {
                MGLCD.Locate(xa, ya + 1);
            }
            n = 3;
        }
        
    }else if (func == "l"){ // 対数のカーソル移動
      if(n == 0){
        MGLCD.Locate(x, y+1);
      }
      else if(n == 1){
        MGLCD.Locate(x, y-2);
        func = "";
        n = 0;
      }
    }
    else if (func == "^" && n == 1){ // 累乗のカーソル移動
      MGLCD.Locate(x, y+1);
      int length = strlen(enteredDigits);
      enteredDigits[length - 1] = '\0';
      i -= 1;
      n = 0;
      func = "";
    }
    else if (func == "b"){ // 分数のカーソル移動
      if (n == 0){
        MGLCD.Locate(x-1, y-1);
      }else if(n == 1){
        MGLCD.Locate(x-1, y+2);
        m += 1;
      }
      else if(n == 2){
        MGLCD.Locate(x-m, y+2);
      
        n = 0;
        m = 0;
        func = "";
      }
    }else if (func == "r"){
      if(n == 0){
        MGLCD.Locate(x, y-1);
        MGLCD.print("\x84");
        MGLCD.Locate(x, y);
      }
      else if(n == 1){
        n = 0;
        func = "";
      }
    }

    // 特殊機能の処理
    if (str == "A") {
      MGLCD.Reset();
      memset(enteredDigits, 0, sizeof(enteredDigits)); // 入力された数値をリセット
      i = 0;
      MGLCD.Locate(0,1);
    }else if (str == "D"){
      int length = strlen(enteredDigits);
      if (length > 0) {
        enteredDigits[length - 1] = '\0';
        i -= 1;
        MGLCD.Locate(x-1,y-1);
        MGLCD.print(" ");
        MGLCD.Locate(x-1,y+1);
        MGLCD.print(" ");
        MGLCD.Locate(x-1,y);
        MGLCD.print(" ");
        x = MGLCD.GetX();
        y = MGLCD.GetY();
        MGLCD.Locate(x-1,y);
      }
    }else if (str == "k") {
      displayIntegral();
      func = str;
    }else if (str == "r") { // 平方根
      MGLCD.print("\x83");
      func = str;
    }else if (str == "l"){ // 対数
      MGLCD.print("log");
      func = str;
    }else if (str == "^"){ // 累乗
      MGLCD.Locate(x, y-1);
    }else if (str == "b"){ // 分数
      MGLCD.print("\x85");
      func = str;
    }else if (str == "P" || func == "H"){ // 割り振りなし
    }else if (str == "G"){ // 関数わけ
      strncpy(&enteredDigits[i-1], " ", 1);
      n +=1;
    }else if (str == "s"){ // sin
      MGLCD.print("sin");
    }else if (str == "c"){ // cos
      MGLCD.print("cos");
    }else if (str == "t"){ // tan
      MGLCD.print("tan");
    }else if (str == "i"){ // sin
      MGLCD.print("asin");
    }else if (str == "*"){ // 乗算
      MGLCD.print("*");
    }else{ // 数字
      Digit(str);
    }
    i += 1;
  }

   //Serial.println(enteredDigits);
}

// 数式の評価
float evaluateExpression(String expr) {

    int startIdx, endIdx;
    while ((startIdx = expr.lastIndexOf('(')) != -1) {
        endIdx = expr.indexOf(')', startIdx);
        if (endIdx == -1) {
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
    if (idx < expr.length() && (isdigit(expr.charAt(idx)) || expr.charAt(idx) == '.')) {
      int start = idx;
      while (idx < expr.length() && (isdigit(expr.charAt(idx)) || expr.charAt(idx) == '.')) idx++;
      return expr.substring(start, idx).toFloat();
    }
    if (idx < expr.length() && expr.charAt(idx) == 's') {
        idx += 1;
        return sin(evaluateFactor(expr, idx) * PI / 180.0);
    } else if (idx < expr.length() && expr.charAt(idx) == 'c') {
        idx += 1;
        return cos(evaluateFactor(expr, idx) * PI / 180.0);
    } else if (idx < expr.length() && expr.charAt(idx) == 't') {
        idx += 1;
        return tan(evaluateFactor(expr, idx) * PI / 180.0);
    } else if (idx < expr.length() && expr.charAt(idx) == 'r') {
        idx += 1;
        return sqrt(evaluateFactor(expr, idx));
    } else if (idx < expr.length() && expr.charAt(idx) == 'i') {
        idx += 1;
        float value = evaluateFactor(expr, idx);
        return asin(value) * 180.0 / PI; // ラジアンを度に変換
    } else if (expr.charAt(idx) == 'b') {
        return evaluateFraction(expr);
     } else if (expr.charAt(idx) == 'l') {
        return calculateLog(expr);
 
     } else if (expr.charAt(idx) == '^') {
        return pow(evaluateFactor(expr, idx), evaluateFactor(expr, ++idx));
     }   
    return NAN;
}

// 数式を数値積分する関数（台形法）
float integrateExpression(String input) {
    int firstSpace = input.indexOf(' '); // 最初のスペースを見つける
    int secondSpace = input.indexOf(' ', firstSpace + 1);

    float lower = input.substring(1, firstSpace).toFloat(); // "k"の次から最初のスペースまでを下限として解析
    float upper = input.substring(firstSpace + 1 , secondSpace).toFloat();
    String function = input.substring(secondSpace + 1); // 関数部分を取得

    const int n = 1000; // 分割数
    float h = (upper - lower) / n; // 区間の幅
    Serial.println(h);
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
    int splitIndex = input.indexOf(' ');

    float base = input.substring(1, splitIndex).toFloat();
    float value = input.substring(splitIndex + 1).toFloat();

    return log(value) / log(base); //対数の変換公式を使用
}

float evaluateFraction(String input) {
    int firstSpace = input.indexOf(' ', 1); // "b"の次のスペースを見つける
    float numerator = input.substring(1, firstSpace).toFloat(); // "b"の次の文字から最初のスペースまでの部分文字列を数値に変換
    float denominator = input.substring(firstSpace + 1).toFloat(); // 最初のスペースの後から最後までの部分文字列を数値に変換
    return numerator / denominator;
}



void sendEquationToPC() {;

    // Shift + 2
    pressCtrlAndKey('1');

    // 数式をPCに送信
 for (int i = 0; i < strlen(enteredDigits); i++) {
        char c = enteredDigits[i];
        if (c == '^') {
            pressCtrlAndKey('5');
            i++;
            if (i < strlen(enteredDigits)) {
                Keyboard.print(enteredDigits[i]);
            }
        } else if (c == '+') {
            pressCtrlAndKey('8');
        } else if (c == '-') {
            pressCtrlAndKey('9');
        } else if (c == '*') {
            pressCtrlAndKey('0');
        } else if (c == '(') {
            pressCtrlAndKey(KEY_F1);
        } else if (c == ')') {
            pressCtrlAndKey(KEY_F3);
        } else if (c == 'r') {
            pressCtrlAndKey('2'); // \;
            Keyboard.print("sqrt ");
            delay(400);
        } else if (c == 's') {
            pressCtrlAndKey('2'); // \;
            Keyboard.print("sin ");
            delay(400);
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
        } else if (c == 'i') {
            pressCtrlAndKey('2');
            Keyboard.print("sin ");
            delay(400);
            performKeySequence(enteredDigits, i);
        } else if (c == 'c') {
            pressCtrlAndKey('2');
            Keyboard.print("cos ");
            delay(400);
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
        } else if (c == 't') {
            pressCtrlAndKey('2');
            Keyboard.print("tan ");
            delay(400);
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
        } else if (c == 'b') {
            pressCtrlAndKey('2'); // \
            delay(500);
            Keyboard.print("frac");
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
            while (enteredDigits[i] == ' ') { // 分母の前のスペースを飛ばす
                i++;
            }
           handleSubFunction2(i, enteredDigitsStr);
            continue; // ループの残りをスキップして次に進む
        } else if (c == 'l') { // log対応
            pressCtrlAndKey('2');
            delay(500);
            Keyboard.print("log");
            pressCtrlAndKey('7');
            i++; // 'l'の次の文字に進む
            while (enteredDigits[i] == ' ') { // スペースを飛ばす
                i++;
            }
            pressCtrlAndKey('3'); 
            // 底を取得
            while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
                Keyboard.print(enteredDigits[i]);
                i++;
            }
            while (enteredDigits[i] == ' ') { // 真数の前のスペースを飛ばす
                i++;
            }
            pressCtrlAndKey('4');
            String enteredDigitsStr = enteredDigits; handleSubFunction2(i, enteredDigitsStr);
            continue; // ループの残りをスキップして次に進む

        } else if (c == 'k') { //積分処理
            pressCtrlAndKey('2');
            delay(700);
            Keyboard.print("int");
            pressCtrlAndKey('7');
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
            
            // 上限を取得
            while (enteredDigits[i] == ' ') { // スペースを飛ばす
                i++;
            }
            pressCtrlAndKey('5');
            handleSubFunction2(i, enteredDigitsStr);
            while (enteredDigits[i] == ' ') { // スペースを飛ばす
                i++;
            }
            handleSubFunction2(i, enteredDigitsStr);
           
            continue; // ループの残りをスキップして次に進む
        } else if (c == ' ') {
            Keyboard.print(' ');
        } else {
          Keyboard.print(c);
        
        }
    }

    // 結果をPCに送信
    pressCtrlAndKey('6');
    delay(100);
    float result = evaluateExpression(String(enteredDigits));
    Keyboard.releaseAll(); // 少しの間押し続ける
    Keyboard.print(result);

    // エンターキーを送信
    Keyboard.write(KEY_RETURN);
}


void performKeySequence(String enteredDigits, int i) {
    pressCtrlAndKey('5');
    pressCtrlAndKey('3');
    Keyboard.print("-1");
    pressCtrlAndKey('4');
    String enteredDigitsStr = enteredDigits;handleSubFunction1(i, enteredDigitsStr);
} 






void pressCtrlAndKey(char key) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(key);
    delay(400);
    Keyboard.releaseAll();
}

void handleSubFunction1(int &index, String &function) {
    pressCtrlAndKey('3');
    index += 1;
    if (isFunction(function[index])) {
        String subFunction = extractFunction(index);
        sendFunctionToPC(subFunction);
    } else {
        while (index < function.length() && (isDigit(function[index]) || function[index] == '.' || function[index] == 'x')) {
            Keyboard.print(function[index]);
            index++;
        }
    }
    if (isFunction(function[index])) {
        String subFunction = extractFunction(index);
        sendFunctionToPC(subFunction);
        index++;
    }
    pressCtrlAndKey('4');
}

void handleSubFunction2(int &index, String &function) {
    pressCtrlAndKey('3'); 
    Serial.println(function[index]);
    if (isFunction(function[index])) {
        String subFunction = extractFunction(index);
        sendFunctionToPC(subFunction);
        index++;
      
    } else {
        while (index < function.length() && (isDigit(function[index]) || function[index] == '.'|| function[index] == 'x')) {
            Keyboard.print(function[index]);
            index++;
        }
    }
    if (isFunction(function[index])) {
        String subFunction = extractFunction(index);
        sendFunctionToPC(subFunction);
        index++;
    }
    pressCtrlAndKey('4');
}

int countTotalCharacters(const char* enteredDigits) {
    int totalCount = 0;
    for (int i = 0; enteredDigits[i] != '\0'; i++) {
        if (isdigit(enteredDigits[i]) || isalpha(enteredDigits[i]) || isspace(enteredDigits[i])) {
            totalCount++;
        }
    }
    return totalCount;
}

// 新しい関数: サブ関数の抽出
bool isFunction(char c) {
    return (c == 's' || c == 'c' || c == 't' || c == 'r' || c == 'b' ||  c == 'l' || c == '^');
}

// 新しい関数: サブ関数の抽出
String extractFunction(int &index) {
    String func;
    // 関数名の抽出 (^ と r を含む)
    func += enteredDigits[index];
    index++;

    Serial.print("func1");
    Serial.println(func);
    // スペースをスキップ
    while (index < strlen(enteredDigits) && enteredDigits[index] == ' ') {
        index++;
    }

    // 引数の抽出
    while (index < strlen(enteredDigits) && (isDigit(enteredDigits[index]) || enteredDigits[index] == '.' || enteredDigits[index] == 'x')) {
        func += enteredDigits[index];
        index++;
    }
    while (index < strlen(enteredDigits) && enteredDigits[index] == ' ') {
        func += enteredDigits[index];
        index++;
    }

    // 引数の抽出
    while (index < strlen(enteredDigits) && (isDigit(enteredDigits[index]) || enteredDigits[index] == '.' || enteredDigits[index] == 'x')) {
        func += enteredDigits[index];
        index++;
    }
    
    return func;
    
}

void pressCtrlAndKeyWithNextChar(int &index, String &function) {
    index++;
    pressCtrlAndKey('3');
    while (index < function.length() && function[index] != ' ') {
        Keyboard.print(function[index]);
        index++;
    }
    pressCtrlAndKey('4');
}

void sendFunctionToPC(String function) {

    // サブ関数の評価と送信
    for (int j = 0; j < function.length(); j++) {
        char c = function[j];
        if (c == 's') {
            pressCtrlAndKey('2');
            Keyboard.print("sin ");
            delay(400);
            pressCtrlAndKeyWithNextChar(j, function);
        } else if (c == 'c') {
            pressCtrlAndKey('2');
            Keyboard.print("cos ");
            delay(400);
            pressCtrlAndKeyWithNextChar(j, function);
        } else if (c == 't') {
            pressCtrlAndKey('2');
            Keyboard.print("tan ");
            delay(400);
            pressCtrlAndKeyWithNextChar(j, function);
        } else if (c == 'r') {
            pressCtrlAndKey('2');
            Keyboard.print("sqrt ");
            delay(400);
            pressCtrlAndKeyWithNextChar(j, function);
        } else if (c == 'b') { // \frac対応
            pressCtrlAndKey('2'); // \
            delay(500);
            Keyboard.print("frac");
             Serial.println("ここまで来とる");
             Serial.println(function);
            pressCtrlAndKey('3'); // {
            j++; // 'b'の次の文字に進む
            while (j < function.length() && (isDigit(function[j]) || function[j] == '.'|| function[j] == 'x')) {
                Keyboard.print(function[j]);
                j++;
            }
            pressCtrlAndKey('4'); // }
            while (function[j] == ' ') { // 分母の前のスペースを飛ばす
                j++;
            }
            pressCtrlAndKey('3'); // {
            // 分母を取得
            while (j < function.length() && (isDigit(function[j]) || function[j] == '.'|| function[j] == 'x')) {
                Keyboard.print(function[j]);
                j++;
            }
            pressCtrlAndKey('4'); // }
            continue; // ループの残りをスキップして次に進む

        } else if (c == '^') {
            pressCtrlAndKey('5');
            pressCtrlAndKeyWithNextChar(j, function);
        } else if (c == '+') {
            pressCtrlAndKey('8');
            delay(400);
            Keyboard.releaseAll();
        } else if (c == '-') {
            pressCtrlAndKey('9');
            delay(400);
            Keyboard.releaseAll();
        } else if (c == '*') {
            pressCtrlAndKey('0');
            delay(400);
            Keyboard.releaseAll();
        }
    }
}
