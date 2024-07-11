int ledPin = A0;  // Arduinoの13番ピンに接続されたLED
int pirPin = 0; // HC-S501の入力ピン

int pirValue; // PIRセンサーの値を格納する場所
unsigned long lastMotionTime = 0; // 最後に動きを検出した時間
const unsigned long delayTime = 5000; // 遅延時間（ミリ秒単位、ここでは5秒）

void setup() {
  pinMode(ledPin, OUTPUT); // LEDピンを出力モードに設定
  pinMode(pirPin, INPUT); // PIRピンを入力モードに設定
  digitalWrite(ledPin, LOW); // 初期状態でLEDを消灯
}

void loop() {
  pirValue = digitalRead(pirPin); // PIRセンサーの値を読み取る
  
  if (pirValue == HIGH) {
    // 動きを検出した場合
    digitalWrite(ledPin, HIGH); // LEDを点灯
    lastMotionTime = millis(); // 最後に動きを検出した時間を更新
  } else {
    // 最後に動きを検出してから5秒経過したかどうかをチェック
    if (millis() - lastMotionTime > delayTime) {
      digitalWrite(ledPin, LOW); // LEDを消灯
    }
  }
}
