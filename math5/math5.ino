#include <Arduino.h>

void setup() {
    Serial.begin(9600);
}

void loop() {
    if (Serial.available() > 0) {
        delay(20); // 待機してすべてのデータを受信する

        String input = Serial.readStringUntil('\n'); // 改行までの文字列を読み込む
        input.trim(); // 先頭と末尾の空白を除去する
        input = preprocessInput(input); // 入力を簡略化された形式に対応する

        if (input.startsWith("i ")) {
            float result = integrate(input);
            Serial.print(input);
            Serial.println(result, 2); // 結果を2桁の精度で表示
        } else if (input.startsWith("l ")) {
            float result = calculateLog(input);
            Serial.print("log_");
            Serial.print(input.substring(2, input.indexOf(' ', 2)));
            Serial.print(" ");
            Serial.print(input.substring(input.indexOf(' ', 2) + 1));
            Serial.println(result, 2); // 結果を2桁の精度で表示
        } else {
            float result = evaluateExpression(input);
            Serial.print(input);
            Serial.println(result, 2); // 結果を2桁の精度で表示
        }
    }
}

// 入力を簡略化された形式に対応する
String preprocessInput(String input) {
    input.replace("s", "sin");
    input.replace("c", "cos");
    input.replace("t", "tan");
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
float integrate(String input) {
    int firstSpace = input.indexOf(' ', 2); // "i "の次のスペースを見つける
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
