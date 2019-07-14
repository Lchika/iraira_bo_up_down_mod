/**
 * @file iraira_bo_up_down.ino
 * @brief 上下モジュールプログラム
 * @author T.Suzuki K.Ohira
 * @date 2019/06/21
 */

#include <MsTimer2.h>
#include "debug.h"
#include "dsub_slave_communicator.hpp"

//#define DEBUG_NOW_STATE

#define PIN_GOAL_SENSOR     5   //  通過/ゴールしたことを検知するセンサのピン
#define PIN_HIT_SENSOR      4   //  当たったことを検知するセンサのピン
#define PIN_DIP_0           6   //  DIPスイッチbit0
#define PIN_DIP_1           7   //  DIPスイッチbit1
#define PIN_DIP_2           8   //  DIPスイッチbit2
#define PIN_DIP_3           9   //  DIPスイッチbit3
#define PIN_AIN_1           3   //  モータ制御ピン1
#define PIN_AIN_2           10  //  モータ制御ピン2
#define PIN_LED_1           13  //  LED制御ピン1
#define PIN_LED_2           14  //  LED制御ピン2
#define PIN_LED_3           15  //  LED制御ピン3
#define PIN_LED_4           16  //  LED制御ピン4
#define PIN_LED_5           17  //  LED制御ピン5

#define MOTOR_POWER         128 //  モータに印加する電圧の大きさ[0-255]
#define INTARVAL_LED_FLASH  500 //  点灯させるLEDを変化させる間隔[ms]

/* 変数宣言 */
unsigned char slv_address;      //  スレーブアドレス(0はゴールモジュール固定、それ以外は1~)
DsubSlaveCommunicator *dsubSlaveCommunicator = NULL;

/**
 * @brief セットアップ関数
 * @param None
 * @return None
 */
void setup(void) {
  /* ここから各スレーブ共通コード */
  BeginDebugPrint();                //  デバッグ用
  /* ディップスイッチを入力として設定 */
  pinMode(PIN_DIP_0, INPUT);
  pinMode(PIN_DIP_1, INPUT);
  pinMode(PIN_DIP_2, INPUT);
  pinMode(PIN_DIP_3, INPUT);

  slv_address = ReadDipSwitch();    //  SLVアドレスを設定
  /* D-sub通信管理用インスタンスの生成 */
  dsubSlaveCommunicator = new DsubSlaveCommunicator(PIN_GOAL_SENSOR,
                              PIN_HIT_SENSOR, slv_address, true, true);
  /* ここまで各スレーブ共通コード */

  /* ここから各モジュール独自コード */
  //  LED関係ピンを出力として設定
  DebugPrint("set led pin out");
  pinMode(PIN_LED_1, OUTPUT);
  pinMode(PIN_LED_2, OUTPUT);
  pinMode(PIN_LED_3, OUTPUT);
  pinMode(PIN_LED_4, OUTPUT);
  pinMode(PIN_LED_5, OUTPUT);
  
  //  LED制御を定時タスクとしてセット
  DebugPrint("set led control task");
  MsTimer2::set(INTARVAL_LED_FLASH, flash_led);
  /* ここまで各モジュール独自コード */
}

/**
 * @brief ループ関数
 * @param None
 * @return None
 */
void loop(void) {
  /* ここから各スレーブ共通コード */
  static bool pre_active = false;
//  デバッグ設定が有効の場合、シリアル入力からnow_stateを変更できるようにする
#ifdef DEBUG_NOW_STATE
  static bool now_active = false;
  if (Serial.available() > 0){
    Serial.read();
    now_active = !now_active;
  }
#else
  bool now_active = DsubSlaveCommunicator::is_active();
#endif
  Serial.println(digitalRead(PIN_HIT_SENSOR));
  //  プレイヤーがこのモジュール上で遊んでいるとき
  if(now_active){
    //  D-sub関係イベント処理
    dsubSlaveCommunicator->handle_dsub_event();

    //  前回不活性状態だった場合
    //  (このモジュールに到達したタイミング)
    if(!pre_active){
      //  LED制御タスク開始
      DebugPrint("start led control task");
      MsTimer2::start();
      //  モータ正転開始
      DebugPrint("start motor ccw");
      rot_motor_cw(MOTOR_POWER);
    }
  //  プレイヤーがこのモジュール上で遊んでいないとき
  }else{
    //  前回活性状態だった場合
    //  (このモジュールを終了したタイミング)
    if(pre_active){
      //  LED制御を停止
      DebugPrint("stop led control task");
      MsTimer2::stop();
      //  LED全消灯
      init_led();
      //  モータ停止
      DebugPrint("stop motor");
      brake_motor();
    }
  }
  pre_active = now_active;
}

/**
 * @brief DIPスイッチ読み取り関数
 * @param None
 * @return unsigned char  DIPスイッチの値
 */
unsigned char ReadDipSwitch(void) {
  unsigned char value = 0;

  if(digitalRead(PIN_DIP_0) == HIGH) {
    value += 1;
  }
  if(digitalRead(PIN_DIP_1) == HIGH) {
    value += 2;
  }
  if(digitalRead(PIN_DIP_2) == HIGH) {
    value += 4;
  }
  if(digitalRead(PIN_DIP_3) == HIGH) {
    value += 8;
  }

  return value;
}

/**
 * @brief モータ正転関数
 * @param[in] duty  モータに印加する電圧の大きさ[0-255]
 * @return None
 */
void rot_motor_cw(int duty) {
  analogWrite(PIN_AIN_1, 0);
  analogWrite(PIN_AIN_2, duty);
}
  
/**
 * @brief モータ逆転関数
 * @param[in] duty  モータに印加する電圧の大きさ[0-255]
 * @return None
 */
void rot_motor_ccw(int duty) {
  analogWrite(PIN_AIN_1, duty);
  analogWrite(PIN_AIN_2, 0);
}

/**
 * @brief ブレーキ関数
 * @param None
 * @return None
 */
void brake_motor() {
  analogWrite(PIN_AIN_1, 255);
  analogWrite(PIN_AIN_2, 255);
}

/**
 * @brief モータ解放関数
 * @param None
 * @return None
 */
void free_motor(){
  analogWrite(PIN_AIN_1, 0);
  analogWrite(PIN_AIN_2, 0);
}

/**
 * @brief LED制御関数
 * @param None
 * @return None
 */
void flash_led() {
  static int iteration = 0;
  int led_num = iteration % 5 + 1;
  
  switch(led_num) {
    case 1:
      digitalWrite(PIN_LED_1, HIGH);
      digitalWrite(PIN_LED_2, LOW);
      digitalWrite(PIN_LED_3, LOW);
      digitalWrite(PIN_LED_4, LOW);
      digitalWrite(PIN_LED_5, LOW);
      break;

    case 2:
      digitalWrite(PIN_LED_1, LOW);
      digitalWrite(PIN_LED_2, HIGH);
      digitalWrite(PIN_LED_3, LOW);
      digitalWrite(PIN_LED_4, LOW);
      digitalWrite(PIN_LED_5, LOW);
      break;

    case 3:
      digitalWrite(PIN_LED_1, LOW);
      digitalWrite(PIN_LED_2, LOW);
      digitalWrite(PIN_LED_3, HIGH);
      digitalWrite(PIN_LED_4, LOW);
      digitalWrite(PIN_LED_5, LOW);
      break;

    case 4:
      digitalWrite(PIN_LED_1, LOW);
      digitalWrite(PIN_LED_2, LOW);
      digitalWrite(PIN_LED_3, LOW);
      digitalWrite(PIN_LED_4, HIGH);
      digitalWrite(PIN_LED_5, LOW);
      break;

    case 5:
      digitalWrite(PIN_LED_1, LOW);
      digitalWrite(PIN_LED_2, LOW);
      digitalWrite(PIN_LED_3, LOW);
      digitalWrite(PIN_LED_4, LOW);
      digitalWrite(PIN_LED_5, HIGH);
      break;

    default:
      break;
  }

  iteration++;
}

/**
 * @brief LED初期化関数
 * @param None
 * @return None
 */
void init_led(void){
  digitalWrite(PIN_LED_1, LOW);
  digitalWrite(PIN_LED_2, LOW);
  digitalWrite(PIN_LED_3, LOW);
  digitalWrite(PIN_LED_4, LOW);
  digitalWrite(PIN_LED_5, LOW);
}