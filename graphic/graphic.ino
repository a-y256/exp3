#include <ResKeypad.h>
#include <mglcd.h>

// A0につながっているキーパッドをkeypad1とする
ResKeypad keypad1(A5, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_A); // SIDE Aに部品を実装した場合はこの行を有効にする
//ResKeypad keypad1(A8, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_B); // SIDE Bに部品を実装した場合はこの行を有効にする

// A1につながっているキーパッドをkeypad2とする
ResKeypad keypad2(A6, 16, RESKEYPAD_4X4, RESKEYPAD_4X4_SIDE_A); // SIDE Aに部品を実装した場合はこの行を有効にする
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
    s  _ M _ _ _ _ _ _,
    s  M _ _ _ _ _ _ _,
    s  _ M M M M M M _,
    s  _ _ _ _ _ _ _ M,
    s  _ _ _ _ _ _ M _
  },
  { // 81
    s  _ _ M _ _ _ _ _,
    s  _ M _ _ _ _ _ _,
    s  M M M M M M M M,
    s  _ _ _ _ _ _ _ M,
    s  _ _ _ _ _ _ _ M
  },
  { // 82
    s  _ _ _ _ _ _ _ _,
    s  _ M _ _ _ _ _ _,
    s  _ M _ _ _ _ _ _,
    s  M _ _ _ _ _ _ _,
    s  _ M M M M M M M
  },
};

#undef s
#undef M
#undef _

static mglcd_SG12864 MGLCD(PinAssignTable);

void setup() {
  Serial.begin(9600);

  while (MGLCD.Reset()); //LCDの初期化
  MGLCD.UserChars(UserChars, sizeof(UserChars) / 5);

}

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
    char pressedKey = keys[key];
    Serial.println(pressedKey); // もし何かキーが押されていたら、対応する文字をシリアル出力

    // Check if the key is 'i' (or any key you want to map to the symbols)
    if (pressedKey == 'k') {
      MGLCD.print("\x80"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == 'r') {
      MGLCD.print("\x81"); // Print the custom symbol corresponding to 'i'
    }
  }
}
