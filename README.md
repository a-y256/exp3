
# 理系のための左手デバイス
 
理系の作業を効率化するためにArduinoを用いて開発された左手デバイスのソースコードです。

<img src="https://github.com/user-attachments/assets/f3693222-d255-4b8f-88cf-7ca83e6b91e5" width="500">

# ファイル構成
 
* mathで数式の計算のプログラム
* graphicでディスプレイへの数式の描画、
* wordでディスプレイに出力された数式をWordに出力
* mainnewに1~4が統合されたプログラム

# 動作

キーパッドに関数電卓のように数式を入力し＝を押すと計算できます。またその計算結果を出力ボタンを押すことでWordへ数式のフォーマットのまま出力することが出来ます。

<img src="https://github.com/user-attachments/assets/7d88f2fd-5b94-41c4-9493-82caebd68bd9" width="500">


# 要件
 
* Arduino IDE 1.8.19
* AutoHotkey 1.1
 
# 使用ライブラリ
 
```bash
#include <ResKeypad.h>
#include <MGLCD.h>
#include <MGLCD_SPI.h>
#include <Arduino.h>
#include <HID-Project.h>
#include <math.h>
```

# 著者
* JoJo616161 (math担当)
* haNA-873 (graphic担当)
* a-y256 (word及びマージ作業担当)
 
