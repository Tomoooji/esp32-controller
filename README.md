# ESP32 Controller
ESP32を有線/無線で操作する汎用コントローラークラス
## ファイル構成



ブラウザでこのページを開いて、緑の四角から[Download ZIP]を選択してzipファイルをダウンロード  
ArduinoIDEの上部メニューから [スケッチ] ＞ [ライブラリをインクルード] ＞ [.ZIP形式のライブラリをインストール...] をクリック。  
ダウンロードしたZIPファイル（解凍しなくてOK）を選択し、[開く] を押す。  
画面下に「ライブラリがインストールされました」と出たら成功！  
(⬇️ArduinoIDEやESP32のボード、外部ライブラリのバージョンに注意)  

## 依存関係
ArduinoIDEのESP32を想定しています。  
- ArduinoIDE ver.
- ESP32 ver.
  
各外部ライブラリは適宜ライブラリマネージャーでインストールしてください。  
- PS4Controller ver.
- (RemoteXY ver. )
  
## 使い方
step1. あらかじめ``#define CONTROLLER_TYPE コントローラーの種類``を設定しておく  
step2. ``#include <ESP32_Controller.h>``でインクルード  
step3. やり取りしたい変数を格納するための構造体を宣言して実体化  
step4. ``Controller<構造体の型名> コントローラーオブジェクト(構造体の実体名);``で宣言  
step5. setup関数内で``コントローラーオブジェクト.begin();``で初期化  
step6. loop関数内で``コントローラーオブジェクト.update();``で値の更新  
step7. ``コントローラーオブジェクト.get_input().やり取りしたい変数名``で値を取得  
※ 詳細はexampleフォルダ内のサンプルスケッチを参照してください。


---
作成者:金栄  
最終更新日:2026/07/  

see also:
- [Planaria Renewal](https://github.com/Tomoooji/Planaria_renewal/blob/example/tomoooji/planaria_renewal/Controller_PS4.h)
- [Raspi Controller](https://github.com/Tomoooji/raspi-controller/blob/dev/src/ESP32_PranariaTest/SerialController.h)
- [ESPNOW Rimocon](https://github.com/Tomoooji/ESPNOW_Rimocon/blob/main/src/ESPNOW_Rimocon.h)
- [Rimocon RemoteXY](https://github.com/AiMEiBA-KwanseiGakuin/Mitochondria-KansaiHaru2026/blob/archive/Final/Rimocon_RemoteXY.h)