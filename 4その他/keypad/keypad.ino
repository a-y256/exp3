#include <ResKeypad.h>

// A0につながっているキーパッドをkeypad1とする
ResKeypad keypad1(A4,16,RESKEYPAD_4X4,RESKEYPAD_4X4_SIDE_A); // SIDE Aに部品を実装した場合はこの行を有効にする
//ResKeypad keypad1(A0,16,RESKEYPAD_4X4,RESKEYPAD_4X4_SIDE_B); // SIDE Bに部品を実装した場合はこの行を有効にする

// A1につながっているキーパッドをkeypad2とする
ResKeypad keypad2(A5,16,RESKEYPAD_4X4,RESKEYPAD_4X4_SIDE_A); // SIDE Aに部品を実装した場合はこの行を有効にする
//ResKeypad keypad2(A1,16,RESKEYPAD_4X4,RESKEYPAD_4X4_SIDE_B); // SIDE Bに部品を実装した場合はこの行を有効にする

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

void setup() {
  Serial.begin(9600);
} // setup

void loop() {
  signed char key; // キーパッドから読み取ったキー番号(キーパッド1とキーパッド2で共用)

  key = keypad1.GetKey(); // keypad1からキー番号を読み取る。キーが押されていないならkey=-1
  if (key < 0) { // keypad1のキーが押されていなかった場合
    key = keypad2.GetKey(); // keypad2からキー番号を読み取る。キーが押されていないならkey=-1
    if (key >= 0) key += 16; // keypad2のキーが押されていたら、keyに16を足す
  } // if

  // この時点で、keypad1のキーが押されていた場合は、keyの値は0～15になっており、
  // keypad1のキーが押されておらず、keypad2のキーが押されていた場合は、keyの値は16～31になっており、
  // キーが全く押されていない場合はkeyの値は-1になっている。

  if (key >= 0) {
    Serial.println(keys[key]); // もし何かキーが押されていたら、対応する文字をシリアル出力
  }
} // loop
