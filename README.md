# iraira_bo_up_down_mod

## 概要

- スレーブモジュールの共通部分だけ書いたコード
- …を修正した、上下モジュール専用のプログラム  

## ピン関係

|ピン番号|役割|
|-|-|
|4|モジュール通過判定|
|5|コース接触判定|
|6|DIPスイッチbit0|
|7|DIPスイッチbit1|
|8|DIPスイッチbit2|
|9|DIPスイッチbit3|
|A4(18)|D-sub2番ピン(SDA)|
|A5(19)|D-sub3番ピン(SCL)|
|GND|D-sub1番ピン|
|3|モータ制御ピン1|
|10|モータ制御ピン2|
|13|LED1|
|14|LED2|
|15|LED3|
|16|LED4|
|17|LED5|

## D-sub関係

- スレーブのアドレス指定：スタートモジュールに近い順に1,2,3...  
- D-sub関係はI2C(2,3番ピン)のみに集約し、D-subの4,5番ピン(ゴール通知・コース接触通知)は不要となった
- ただし、上記対応により各モジュールの通過判定が必須になった
  - どのモジュールと通信すればよいかわからないとI2C通信できないため

## モジュール共通部分の処理について

- D-sub関係の処理は`DsubSlaveCommunicator`クラス内の処理で完結する
- `setup()`内で`DsubSlaveCommunicator`クラスのインスタンス`*dsubSlaveCommunicator`を生成する
- DsubSlaveCommunicatorクラスは、マスタから通信開始通知を受け取ると、アクティブ状態になる
- アクティブ状態でなければ、モジュールは何もする必要がない
  - `loop()`内の以下のコードでアクティブ状態かどうか確認している
    ```c++
    if(DsubSlaveCommunicator::is_active()){
    ```
- D-sub関係処理は以下の部分で処理している
  ```c++
  dsubSlaveCommunicator->handle_dsub_event();
  ```
  - コース接触判定、モジュール通過判定もここで行っている
  - この関数は定期的に呼ぶ必要がある

## コンパイル時の注意事項

- 以下の外部ライブラリが必要
  - [ArduinoSTL](https://www.arduinolibraries.info/libraries/arduino-stl)
    - c++の標準ライブラリが使えるようになる
      - c++の標準ライブラリ(STL)については[Wikipedia](https://ja.wikipedia.org/wiki/Standard_Template_Library)参照
    - インストール方法
      - [こちらのページ](https://www.arduinolibraries.info/libraries/arduino-stl)から`ArduinoSTL-1.1.0.zip`をダウンロード
      - 解凍したフォルダを`C:\Users\your-username\Documents\Arduino\libraries`(特に設定変えていなければここでよいはず)に置く

## 本プログラムのマニュアルについて
- 本プログラムは[Doxygen](http://www.doxygen.nl/index.html)を使ってドキュメント化しています
- マニュアルを見るにはプログラムダウンロード後、html/index.htmlをブラウザで開いてください
