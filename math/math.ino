void setup() {
    Serial.begin(9600);
}

void loop() {
    if (Serial.available() > 0) {
        delay(20); // 待機してすべてのデータを受信する
        
        String input = Serial.readStringUntil('\n'); // 改行までの文字列を読み込む
        input.trim(); // 先頭と末尾の空白を除去する
        
        // 数値と操作子を格納するリスト
        float operands[10]; // 最大10個の数値を処理することができます
        char operators[10]; // 数値間の演算子を格納する
        
        int operandCount = 0; // 数値の数
        int operatorIndex = -1; // 現在の演算子のインデックス
        
        // 入力文字列をスキャンして数値と演算子を抽出
        int startIndex = 0;
        int endIndex = 0;
        
        while (endIndex <= input.length()) {
            if (endIndex == input.length() || input.charAt(endIndex) == ' ' || isOperator(input.charAt(endIndex))) {
                if (startIndex < endIndex) {
                    operands[operandCount++] = input.substring(startIndex, endIndex).toFloat();
                    if (endIndex < input.length() && isOperator(input.charAt(endIndex))) {
                        operators[++operatorIndex] = input.charAt(endIndex);
                    }
                }
                startIndex = endIndex + 1;
            }
            endIndex++;
        }
        
        // 最初の数値を結果に設定する
        float result = operands[0];
        
        // 演算を実行する
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
                        Serial.println(": error (divide by zero)");
                        return;
                    }
                    break;
                case 's': // サインを計算
                    result = sin(operands[i]*PI/180.0);
                    break;
                case 'c': // コサインを計算
                    result = cos(operands[i]*PI/180.0);
                    break;
                case 't': // タンジェントを計算
                    result = tan(operands[i]*PI/180.0);
                    break;
                case 'r': // 平方根を計算
                    result = sqrt(operands[i]);
                    break;
                case '^': // 累乗を計算
                    result = pow(operands[i], operands[i + 1]);
                    break;
                default:
                    Serial.println(": error (unsupported operation)");
                    return;
            }
         
        }
        
        // 結果を出力する
        Serial.print("Result: ");
        Serial.println(result);
    }
}

// 文字が演算子かどうかをチェックする関数
bool isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == 's' || c == 'c' || c == 't' || c == 'r' || c == '^');
}
