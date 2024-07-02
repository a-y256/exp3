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

#undef s
#undef M
#undef _

static mglcd_SG12864 MGLCD(PinAssignTable);

void setup() {
  Serial.begin(9600);

  while (MGLCD.Reset()); //LCDの初期化
  MGLCD.UserChars(UserChars, sizeof(UserChars) / 5);
  char str[40];
  MGLCD.Locate(0,1);

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
    x = MGLCD.GetX();
    y = MGLCD.GetY(); // 次に表示する文字の座標を取得する

    // 数字の入力
    if (pressedKey == '0') {
      MGLCD.print("0"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '1') {
      MGLCD.print("1"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '2') {
      MGLCD.print("2"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '3') {
      MGLCD.print("3"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '4') {
      MGLCD.print("4"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '5') {
      MGLCD.print("5"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '6') {
      MGLCD.print("6"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '7') {
      MGLCD.print("7"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '8') {
      MGLCD.print("8"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '9') {
      MGLCD.print("9"); // Print the custom symbol corresponding to 'i'
    }

    // 四則演算
    if (pressedKey == '+') {
      MGLCD.print("+"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '-') {
      MGLCD.print("-"); // Print the custom symbol corresponding to 'i'
    }
    if (pressedKey == '*') {
      MGLCD.print("9"); // Print the custom symbol corresponding to 'i'
    }
    
    // すべて消す
    if (pressedKey == 'A') {
      MGLCD.Reset();
    }
    // 一文字消す
    if (pressedKey == 'D'){
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

    // 積分
    if (pressedKey == 'k') {
      MGLCD.print("\x81");
      MGLCD.Locate(x,y-1);
      MGLCD.print("\x80 b");
      MGLCD.Locate(x,y+1);
      MGLCD.print("\x82 a");
      x = MGLCD.GetX();
      y = MGLCD.GetY();
      MGLCD.Locate(x+1,y-1);
      MGLCD.print("dx");
    }

    // 平方根
    if (pressedKey == 'r') {
      MGLCD.print("\x83"); // Print the custom symbol corresponding to 'i'
    }
    
  }
}
