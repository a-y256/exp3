#include <LiquidCrystal.h>
#include <IRremote.h>
#include <HID-Project.h>
#include <math.h>
//

void pressCtrlAndKey(char key) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(key);
    delay(400);
    Keyboard.releaseAll();
}

void pressKey(char key) {
    Keyboard.press(key);
    delay(400);
    Keyboard.releaseAll();
}

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
    switch (results.value) {
        case 0xFFA25D:
            sendEquationToPC();
            break;
        case 0xFFE21D:
            addSymbolToDisplay('b');
            break;
        case 0xFF629D:
            addSymbolToDisplay('s');
            break;
        case 0xFF22DD:
            calculateAndDisplayResult();
            break;
        case 0xFF02FD:
            addSymbolToDisplay('r');
            break;
        case 0xFFC23D:
            addSymbolToDisplay(' ');
            break;
        case 0xFFE01F:
            addSymbolToDisplay('^');
            break;
        case 0xFFA857:
            addSymbolToDisplay('k');
            break;
        case 0xFF906F:
            addSymbolToDisplay('x');
            break;
        case 0xFF9867:
            addSymbolToDisplay('l');
            break;
        case 0xFFB04F:
            resetDisplay();
            break;
        case 0xFF6897:
            addDigitToDisplay('0');
            break;
        case 0xFF30CF:
            addDigitToDisplay('1');
            break;
        case 0xFF18E7:
            addDigitToDisplay('2');
            break;
        case 0xFF7A85:
            addDigitToDisplay('3');
            break;
        case 0xFF10EF:
            addDigitToDisplay('4');
            break;
        case 0xFF38C7:
            addDigitToDisplay('5');
            break;
        case 0xFF5AA5:
            addDigitToDisplay('6');
            break;
        case 0xFF42BD:
            addDigitToDisplay('7');
            break;
        case 0xFF4AB5:
            addDigitToDisplay('8');
            break;
        case 0xFF52AD:
            addDigitToDisplay('9');
            break;
        case 0xFFFFFFFF:
            break;
        default:
            Serial.println("other button");
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
    String enteredString = String(enteredDigits);
    String func = enteredString.substring(0, enteredString.indexOf(' '));

    Serial.print("Function: ");
    Serial.println(func);
    Serial.print("Entered String: ");
    Serial.println(enteredString);

    if (func == "s") result = sin(parseValue(enteredString) * PI / 180.0);
    else if (func == "c") result = cos(parseValue(enteredString) * PI / 180.0);
    else if (func == "t") result = tan(parseValue(enteredString) * PI / 180.0);
    else if (func == "r") result = sqrt(parseValue(enteredString));
    else if (func == "b") result = evaluateFraction(enteredString);
    else if (func == "k") result = integrate(enteredString);
    else if (func == "l") {
        Serial.println("Calling calculateLog function");
        result = calculateLog(enteredString);
    } else if (func == "asin") result = evaluateInverseTrig(enteredString);
    else if (func == "acos") result = evaluateInverseTrig(enteredString);
    else if (func == "atan") result = evaluateInverseTrig(enteredString);
    else {
        // 数式の評価
        result = evaluateExpression(enteredString);
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

float parseValue(String input) {
    int firstSpace = input.indexOf(' ');
    return input.substring(firstSpace + 1).toFloat();
}

void sendEquationToPC() {
    // Shift + 2
    pressCtrlAndKey('1');

    // 数式をPCに送信
    for (int i = 0; i < strlen(enteredDigits); i++) {
        char c = enteredDigits[i];
        if (c == 'w') {
            Keyboard.print("/");
        } else if (c == '^') {
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
        } else if (c == 'q') {
            pressCtrlAndKey('2');
            Keyboard.print("sin ");
            delay(400);
            pressCtrlAndKey('5');
            pressCtrlAndKey('3');
            Keyboard.print("-1");
            pressCtrlAndKey('4');
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
        } else if (c == 'c') {
            pressCtrlAndKey('2');
            Keyboard.print("cos ");
            delay(400);
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
        } else if (c == 'q') {
            pressCtrlAndKey('2');
            Keyboard.print("cos ");
            delay(400);
            pressCtrlAndKey('5');
            pressCtrlAndKey('3');
            Keyboard.print("-1");
            pressCtrlAndKey('4');
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
        } else if (c == 't') {
            pressCtrlAndKey('2');
            Keyboard.print("tan ");
            delay(400);
            String enteredDigitsStr = enteredDigits; handleSubFunction1(i, enteredDigitsStr);
        } else if (c == 'q') {
            pressCtrlAndKey('2');
            Keyboard.print("tan ");
            delay(400);
            pressCtrlAndKey('5');
            pressCtrlAndKey('3');
            Keyboard.print("-1");
            pressCtrlAndKey('4');
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

            Serial.println(enteredDigits);
            int totalCount = countTotalCharacters(enteredDigits);
            i=totalCount-1;
            Serial.print("Total count of digits, letters, and spaces: ");
            Serial.println(totalCount);
            while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
                i--;
                
            }

            i-=1;
            while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) { 
                i--;
            }
         
            int upperCount =i;
            pressCtrlAndKey('3');
            i++; // 'k'の下限の位置へ
                        while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
                Keyboard.print(enteredDigits[i]);
                i++;
            }
            pressCtrlAndKey('4');
            pressCtrlAndKey('5');
            pressCtrlAndKey('3');
            // 上限を取得
            while (enteredDigits[i] == ' ') { // スペースを飛ばす
                i++;
            }
            while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
                Keyboard.print(enteredDigits[i]);
                i++;
            }
            pressCtrlAndKey('4');
            i = 0;
            char destination[maxDigits + 1] = "";
            for (int k = 0; k <= upperCount; k++) {
            destination[k] = enteredDigits[k];
            }
            String destinationsStr = destination; handleSubFunction1(i,destinationsStr);
            Keyboard.print("dx");
            i = totalCount;
            continue; // ループの残りをスキップして次に進む
        } else if (c == ' ') {
            Keyboard.print(' ');
        } else if (strncmp(enteredDigits + i, "asin", 4) == 0 || strncmp(enteredDigits + i, "acos", 4) == 0 || strncmp(enteredDigits + i, "atan", 4) == 0) {
            char trigFunc[5];
            strncpy(trigFunc, enteredDigits + i, 4);
            trigFunc[4] = '\0';
            i += 4;
            pressCtrlAndKey('2');
            Keyboard.print(trigFunc);
            Keyboard.print(" ");
            delay(400);
            pressCtrlAndKey('3');
            while (i < strlen(enteredDigits) && (isDigit(enteredDigits[i]) || enteredDigits[i] == '.')) {
                Keyboard.print(enteredDigits[i]);
                i++;
            }
            pressCtrlAndKey('4');
            continue; // ループの残りをスキップして次に進む
        } else {
            Keyboard.print(c);
        }
    }

    // 結果をPCに送信
    pressCtrlAndKey('6');
    delay(100);
    Keyboard.releaseAll(); // 少しの間押し続ける
    Keyboard.print(lastResult);

    // エンターキーを送信
    Keyboard.write(KEY_RETURN);

    // 入力内容をリセット
    //resetDisplay();
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

void handleSubFunction1(int &index, String &function) {
    pressCtrlAndKey('3');
    index += 2;
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




// 組み合わせ用
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
    if (idx < expr.length() && expr.charAt(idx) == 's') {
        idx += 3;
        return sin(evaluateFactor(expr, idx) * PI / 180.0);
    } else if (idx < expr.length() && expr.charAt(idx) == 'c') {
        idx += 3;
        return cos(evaluateFactor(expr, idx) * PI / 180.0);
    } else if (idx < expr.length() && expr.charAt(idx) == 't') {
        idx += 3;
        return tan(evaluateFactor(expr, idx) * PI / 180.0);
    } else if (idx < expr.length() && expr.charAt(idx) == 'r') {
        idx += 4;
        return sqrt(evaluateFactor(expr, idx));
    } else if (expr.charAt(idx) == 'b') {
        return evaluateFraction(expr);
    } else if (expr.charAt(idx) == '^') {
        return pow(evaluateFactor(expr, idx), evaluateFactor(expr, ++idx));
    } else if (expr.substring(idx, idx + 4) == "asin" || expr.substring(idx, idx + 4) == "acos" || expr.substring(idx, idx + 4) == "atan") {
        return evaluateInverseTrig(expr.substring(idx));
    }
    return NAN;
}

// 分数形式を評価する関数
float evaluateFraction(String input) {
    int firstSpace = input.indexOf(' ', 2); // "b "の次のスペースを見つける
    int secondSpace = input.indexOf(' ', firstSpace + 1);

    float numerator = input.substring(2, firstSpace).toFloat();
    float denominator = input.substring(firstSpace + 1, secondSpace).toFloat();

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
    int firstSpace = input.indexOf(' ', 2); // "b "の次のスペースを見つける
    int secondSpace = input.indexOf(' ', firstSpace + 1);

    float base = input.substring(2, firstSpace).toFloat();
    float value = input.substring(firstSpace + 1).toFloat();

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

// 新しい関数: サブ関数の抽出
bool isFunction(char c) {
    return (c == 's' || c == 'c' || c == 't' || c == 'r' || c == 'b' || c == 'k' || c == 'l' || c == '^');
}

// 新しい関数: サブ関数の抽出
String extractFunction(int &index) {
    String func;
    // 関数名の抽出 (^ と r を含む)
    while (index < strlen(enteredDigits) && enteredDigits[index] != ' ' && enteredDigits[index] != '^' && enteredDigits[index] != 'r') {
        func += enteredDigits[index];
        index++;
    }
    // "^" と "r" も関数名に含める
    if (index < strlen(enteredDigits) && (enteredDigits[index] == '^' || enteredDigits[index] == 'r')) {
        func += enteredDigits[index];
        index++;
    }
    // スペースをスキップ
    while (index < strlen(enteredDigits) && enteredDigits[index] == ' ') {
        index++;
    }


    // 引数の抽出
    while (index < strlen(enteredDigits) && (isDigit(enteredDigits[index]) || enteredDigits[index] == '.' || enteredDigits[index] == 'x')) {
        func += enteredDigits[index];
        index++;
    }
    Serial.print("デバック2:");
    Serial.println(func);
    
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
    Serial.print("Function list:");
    Serial.println(function);

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
        } else if (c == 'k') {
            pressCtrlAndKey('2');
            delay(700);
            Keyboard.print("int");
            pressCtrlAndKey('7');
            delay(400);
            pressCtrlAndKey('3');
        } else if (c == 'b') { // \frac対応
            pressCtrlAndKey('2'); // \
            delay(500);
            Keyboard.print("frac");
            pressCtrlAndKey('3'); // {
            j++; // 'b'の次の文字に進む
            while (function[j] == ' ') { // スペースを飛ばす
                j++;
            }
            // 分子を取得
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
    Serial.print("Function list2:");
    Serial.println(function);
}
