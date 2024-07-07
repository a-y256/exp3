#include <ResKeypad.h>
#include <mglcd.h>
#include <Arduino.h>
#include <math.h>

// A0につながっているキーパッドをkeypad1とする
ResKeypad keypad1(A4, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_A); // SIDE Aに部品を実装した場合はこの行を有効にする
//ResKeypad keypad1(A8, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_B); // SIDE Bに部品を実装した場合はこの行を有効にする

// A1につながっているキーパッドをkeypad2とする
ResKeypad keypad2(A5, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_A); // SIDE Aに部品を実装した場合はこの行を有効にする
//ResKeypad keypad2(A9, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_B); // SIDE Bに部品を実装した場合はこの行を有効にする

char keys[] = {
  '*', '-', '+', '0',
  '=', '3', '2', '1',
  'A', '6', '5', '4',
  'D', '9', '8', '7',
  ')', '(', 'r', 'b',
  't', 'c', 's', 'k',
  'e', 'l', '^', 'x',
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
  
};

int x;
int y;
int i = 1;
int n = 0;
int xa;
int ya;
int xb;
int yb;
String func = "";

#undef s
#undef M
#undef _

static mglcd_SG12864 MGLCD(PinAssignTable);

const int maxDigits = 16; // 最大表示桁数
char enteredDigits[maxDigits + 1] = ""; // 入力された数値を保存する配列
char pressedKey;

void setup() {
  Serial.begin(9600);

  while (MGLCD.Reset()); //LCDの初期化
  MGLCD.UserChars(UserChars, sizeof(UserChars) / 5);
  MGLCD.Locate(0,1);
}

void addDigitToDisplay(char digit) {
    int length = strlen(enteredDigits);
    if (length < maxDigits) {
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

// 計算結果を表示する関数
void displayResult(float result) {
    char resultStr[17];
    Serial.println(resultStr);
    dtostrf(result, 0, 2, resultStr); // 結果を文字列に変換
    MGLCD.Locate(100, 100); // LCDの2行目に移動
    MGLCD.print("=");
    MGLCD.print(resultStr);
}

void loop() {
  signed char key; // キーパッドから読み取ったキー番号(キーパッド1とキーパッド2で共用)

  key = keypad1.GetKey(); // keypad1からキー番号を読み取る。キーが押されていないならkey=-1
  if (key < 0) { // keypad1のキーが押されていなかった場合
    key = keypad2.GetKey(); // keypad2からキー番号を読み取る。キーが押されていないならkey=-1
    if (key >= 0) key += 16; // keypad2のキーが押されていたら、keyに16を足す
  } // if

  if (key >= 0) {
    pressedKey = keys[key];
    
    addDigitToDisplay(pressedKey);
    String Digits = String(enteredDigits);
    String input = Digits; // 文字列を読み込む
    input.trim(); // 先頭と末尾の空白を除去する
    
    String str = input.substring(i - 1, i);
    
    Serial.println(enteredDigits);
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
        if (str == "=") {
            MGLCD.print(" dx");
            float result = integrateExpression(Digits);
            displayResult(result);
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
    }else if (pressedKey == 'r') {
      MGLCD.print("\x83"); // Print the custom symbol corresponding to 'i'
    }else if (str == "P" || func == "H"){
    }else if (str == "G"){
      strncpy(&enteredDigits[i-1], " ", 1);
      n +=1;
    }else if (str == "s"){
      MGLCD.print("sin");
    }else if (str == "c"){
      MGLCD.print("cos");
    }else if (str == "t"){
      MGLCD.print("tan");
    }
    else if (str == "=") {
      
      float result = evaluateExpression(input);
      displayResult(result);
      //memset(enteredDigits, 0, sizeof(enteredDigits)); // 入力された数値をリセット
      //i = 0;
    }
    else{
      Digit(str);
    }
    i += 1;
  }
}

// 入力を簡略化された形式に対応する
String preprocessInput(String input) {
    input.replace("s", "sin");
    input.replace("c", "cos");
    input.replace("t", "tan");
    input.replace("asin", "asin");
    input.replace("acos", "acos");
    input.replace("atan", "atan");
    input.replace("r", "sqrt");
    return input;
}

// 数式の評価
float evaluateExpression(String expr) {
    int startIdx, endIdx;
    while ((startIdx = expr.lastIndexOf('(')) != -1) {
        endIdx = expr.indexOf(')', startIdx);
        if (endIdx == -1) {
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
    }
    return NAN;
}

// 数式を数値積分する関数（台形法）
float integrateExpression(String input) {
    int firstSpace = input.indexOf(' '); // 最初のスペースを見つける
    int secondSpace = input.indexOf(' ', firstSpace + 1);

    float lower = input.substring(1, firstSpace).toFloat(); // "k"の次から最初のスペースまでを下限として解析
    float upper = input.substring(firstSpace + 1, secondSpace).toFloat();
    String function = input.substring(secondSpace + 1); // 関数部分を取得

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

// 分数形式を評価する関数
float evaluateFraction(String input) {
    int firstSpace = input.indexOf(' ', 2); // "b "の次のスペースを見つける
    int secondSpace = input.indexOf(' ', firstSpace + 1);

    float numerator = input.substring(2, firstSpace).toFloat();
    float denominator = input.substring(firstSpace + 1, secondSpace).toFloat();

    return numerator / denominator;
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
