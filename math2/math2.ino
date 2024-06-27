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

        if (input.startsWith("integrate")) {
            float result = integrate(input);
            Serial.print(input);
            Serial.print(" = ");
            Serial.println(result, 1); // 結果を6桁の精度で表示
        } else {
            // 通常の計算処理
            float result = evaluateExpression(input);
            Serial.print(input);
            Serial.print(" = ");
            Serial.println(result, 1); // 結果を6桁の精度で表示
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
    // 括弧を処理する
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

    // 乗算、除算、累乗を先に処理する
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
            if (op == '+') {
                result += nextTerm;
            } else {
                result -= nextTerm;
            }
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
            if (op == '*') {
                result *= nextFactor;
            } else if (op == '/') {
                if (nextFactor == 0) {
                    Serial.println(": error (divide by zero)");
                    return NAN;
                }
                result /= nextFactor;
            } else if (op == '^') {
                result = pow(result, nextFactor);
            }
        } else {
            break;
        }
    }
    return result;
}

// 数値と関数の評価
float evaluateFactor(String expr, int &idx) {
    while (idx < expr.length() && expr.charAt(idx) == ' ') {
        idx++; // 空白をスキップする
    }

    if (idx < expr.length() && (expr.charAt(idx) == '+' || expr.charAt(idx) == '-')) {
        char sign = expr.charAt(idx++);
        float result = evaluateFactor(expr, idx);
        return (sign == '-') ? -result : result;
    }

    if (idx < expr.length() && isDigit(expr.charAt(idx))) {
        int start = idx;
        while (idx < expr.length() && (isDigit(expr.charAt(idx)) || expr.charAt(idx) == '.')) {
            idx++;
        }
        return expr.substring(start, idx).toFloat();
    }

    // 関数の評価
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

// 文字が演算子かどうかをチェックする関数
bool isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == 's' || c == 'c' || c == 't' || c == 'r' || c == '^');
}

// 数式を数値積分する関数（台形法）
float integrate(String input) {
    // "integrate <function> <lower> <upper>" 形式を想定
    int firstSpace = input.indexOf(' ', 10); // "integrate "の次のスペースを見つける
    int secondSpace = input.indexOf(' ', firstSpace + 1);

    String function = input.substring(10, firstSpace);
    float lower = input.substring(firstSpace + 1, secondSpace).toFloat();
    float upper = input.substring(secondSpace + 1).toFloat();

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
