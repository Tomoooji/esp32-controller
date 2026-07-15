# ESP32 Controller
ESP32を有線/無線で操作する汎用コントローラークラス
## ファイル構成
```
closteriumesp32-controller/
├─ example/
│  └─ closterium_ps4/
│     └─ closterium_ps4.ino     # ロッカーボギー機構がついた6輪ロボットをPS4コントローラーで動かすサンプル
├─ src/
│  ├─ ESP32_Controller.h        # メインのヘッダファイル  ここから各ヘッダに分岐
│  ├─ Controller_BaseClass.h    # 基底クラスのヘッダファイル  直接使うことはない
│  ├─ Controller_PS4.h          # PS4コントローラーとBluetoothで通信するクラスのヘッダファイル
│  ├─ Controller_Serial.h       # シリアル通信(UART)で通信するクラスのヘッダファイル
│  ├─ Controller_I2C.h          # I2C通信で通信するクラスのヘッダファイル
│  └─ Controller_ESPNOW.h       # ESP-NOWで通信するクラスのヘッダファイル  双方向verもある。
└─ README.md
```

## 依存関係
ArduinoIDEのESP32を想定しています。  
- ArduinoIDE ver.
- ESP32 ver.
  
各外部ライブラリは適宜ライブラリマネージャーでインストールしてください。  
- PS4Controller ver.
- (RemoteXY ver. )

## ライブラリの入れ方
step1. ブラウザでこのページを開いて、緑の四角から[Download ZIP]を選択してzipファイルをダウンロード  
step2. ArduinoIDEの上部メニューから [スケッチ] ＞ [ライブラリをインクルード] ＞ [.ZIP形式のライブラリをインストール...] をクリック。  
step3. ダウンロードしたZIPファイル（解凍しなくてOK）を選択し、[開く] を押す。  
画面下に「ライブラリがインストールされました」と出たら成功！  
(ArduinoIDEやESP32のボード、外部ライブラリのバージョンに注意)  
  
## 使い方
step1. あらかじめ``#define CONTROLLER_TYPE コントローラーの種類``を設定しておく  
step2. ``#include <ESP32_Controller.h>``でインクルード  
step3. やり取りしたい変数を格納するための構造体を宣言して実体化  
step4. ``Controller<構造体の型名> コントローラーオブジェクト(構造体の実体名);``で宣言  
step5. setup関数内で``コントローラーオブジェクト.begin();``で初期化  
step6. loop関数内で``コントローラーオブジェクト.update();``で値の更新  
step7. ``コントローラーオブジェクト.get_input().やり取りしたい変数名``で値を取得  
※ 詳細はexampleフォルダ内のサンプルスケッチを参照してください。

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3) - see the [LICENSE](LICENSE) file for details.

This is required because the project depends on [PS4_Controller_Host](https://github.com/pablomarquez76/PS4_Controller_Host) which is licensed under GPLv3.

---
作成者:金栄  
最終更新日:2026/07/  

see also:
- [Planaria Renewal](https://github.com/Tomoooji/Planaria_renewal/blob/example/tomoooji/planaria_renewal/Controller_PS4.h)
- [Raspi Controller](https://github.com/Tomoooji/raspi-controller/blob/dev/src/ESP32_PranariaTest/SerialController.h)
- [ESPNOW Rimocon](https://github.com/Tomoooji/ESPNOW_Rimocon/blob/main/src/ESPNOW_Rimocon.h)
- [Rimocon RemoteXY](https://github.com/AiMEiBA-KwanseiGakuin/Mitochondria-KansaiHaru2026/blob/archive/Final/Rimocon_RemoteXY.h)