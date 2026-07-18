# ESP32 Controller
ESP32を有線/無線で操作する汎用コントローラークラス

## ファイル構成
```
esp32-controller/
├─ examples/
│  ├─ blink_espnow/
│  │  └─ blink_espnow_receiver.ino  # ESP-NOW経由でLEDを付けたり消したりするサンプル
│  └─ closterium_ps4/
│     └─ closterium_ps4.ino         # ロッカーボギー機構がついた6輪ロボットをPS4コントローラーで動かすサンプル
├─ extras/
│  └─ serial_rimocon_raspi/
│     ├─ serial_rimocon.py          # Raspberry Piから構造体ベースのシリアル通信をするプログラム
│     └─ ESP32.json                 # 通信用設定ファイル
├─ src/
│  ├─ ESP32_Controller_BaseClass.h  # 基底クラスのヘッダファイル  直接使うことはない
│  ├─ ESP32_Controller_PS4.h        # PS4コントローラーとBluetoothで通信するクラスのヘッダファイル
│  ├─ ESP32_Controller_Serial.h     # シリアル通信(UART)で通信するクラスのヘッダファイル　双方向verもある
│  ├─ ESP32_Controller_I2C.h        # I2C通信で通信するクラスのヘッダファイル　双方向verもある
│  ├─ ESP32_Controller_BluetoothSerial.h # Bluetooth Classicで通信するクラスのヘッダファイル　双方向verもある
│  └─ ESP32_Controller_ESPNOW.h     # ESP-NOWで通信するクラスのヘッダファイル  双方向verもある
├─ library.properties
├─ LICENSE
└─ README.md

```

## 依存関係
ArduinoIDEのESP32を想定しています。  
- ArduinoIDE ver.2.3.10
- ESP32 ver.3.3.0
  
各外部ライブラリは適宜ライブラリマネージャーでインストールしてください。  
- [PS4Controller](https://github.com/pablomarquez76/PS4_Controller_Host) ver.1.1.0
- (RemoteXY ver. )

## ライブラリの入れ方
step1. ブラウザでこのページを開いて、緑の四角から[Download ZIP]を選択してzipファイルをダウンロード  
step2. ArduinoIDEの上部メニューから [スケッチ] ＞ [ライブラリをインクルード] ＞ [.ZIP形式のライブラリをインストール...] をクリック。  
step3. ダウンロードしたZIPファイル（解凍しなくてOK）を選択し、[開く] を押す。  
画面下に「ライブラリがインストールされました」と出たら成功！  
(ArduinoIDEやESP32のボード、外部ライブラリのバージョンに注意)  
  
## 使い方
step1. ``#include <ESP32_Controller_{操作方法}.h>``でインクルード  
step2. やり取りしたい変数を格納するための操作用構造体を宣言して実体化  
step3. 設定用の構造体(``Config_{操作方法}``)を実体化  
step4. ``Controller<操作用構造体の型名> コントローラーオブジェクト(設定用構造体の実体名,操作用構造体の実体名);``で宣言  
step5. setup関数内で``コントローラーオブジェクト.begin();``で初期化  
step6. loop関数内で``コントローラーオブジェクト.update();``で値の更新  
step7. ``コントローラーオブジェクト.get_input().やり取りしたい変数名``で値を取得  
※ 詳細はexampleフォルダ内のサンプルスケッチを参照してください。

## License
This project is licensed under the GNU General Public License v3.0 (GPLv3) - see the [LICENSE](LICENSE) file for details.

This is required because the project depends on [PS4_Controller_Host](https://github.com/pablomarquez76/PS4_Controller_Host) which is licensed under GPLv3.

---
作成者:Tomoooji  
最終更新:2026/07/18  

see also:
- [Planaria Renewal](https://github.com/Tomoooji/Planaria_renewal/blob/example/tomoooji/planaria_renewal/Controller_PS4.h)
- [Raspi Controller](https://github.com/Tomoooji/raspi-controller/blob/dev/src/ESP32_PranariaTest/SerialController.h)
- [ESPNOW Rimocon](https://github.com/Tomoooji/ESPNOW_Rimocon/blob/main/src/ESPNOW_Rimocon.h)
- [Rimocon RemoteXY](https://github.com/AiMEiBA-KwanseiGakuin/Mitochondria-KansaiHaru2026/blob/archive/Final/Rimocon_RemoteXY.h)