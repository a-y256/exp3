#include <ResKeypad.h>
#include <mglcd.h>
#include <Arduino.h>

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
      strcat(enteredDigits, pressedKey);
      enteredDigits[length] = digit;
      enteredDigits[length + 1] = '\0';// リストに追加
    }
}

void Digit(String digit){
  MGLCD.print(pressedKey);
}

// 積分
void integrate(String input){
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

    if(func == "k"){
      if(n == 0){
        MGLCD.Locate(xa+1, ya);
        xa = MGLCD.GetX();
        ya = MGLCD.GetY();
      }else if(n == 1){
        MGLCD.Locate(xb+1, yb);
        xb = MGLCD.GetX();
        yb = MGLCD.GetY();
      }else if(n == 2){
        if (xa <= xb){
          MGLCD.Locate(xb, yb+1);
        }else{
          MGLCD.Locate(xa, ya+1);
        }
        n = 3;
      }
      if (str == "="){
        MGLCD.print(" dx");
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
      integrate(input);
      func = str;
    }else if (pressedKey == "r") {
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
    else{
      Digit(str);
    }
    i += 1;
  }
}
