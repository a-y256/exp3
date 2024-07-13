
# 理系のための左手デバイス
 
理系の作業を効率化するためにArduinoを用いて開発された左手デバイスのソースコードです。

<img src="https://github.com/user-attachments/assets/b983c4e5-0416-40fc-8d79-f8e1a0795413" width="500">
 
# ファイル構成
 
* mathで数式の計算のプログラム
* graphicでディスプレイへの数式の描画、
* wordでディスプレイに出力された数式をWordに出力
* mainnewに1~4が統合されたプログラム

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
 
