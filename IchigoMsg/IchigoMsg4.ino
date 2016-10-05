/*
 * IchigoMsg v 0.93  8x8ドットマトリックスLEDメッセージ arduino pro mini バージョン
 *  2016/9/30 by たま吉さん 
 *
 
<ライセンスにおける注意事項>
  本ライブラリのフォントデータは IchigoJam 1.2.1 のフォントデータを使用しています。
  IchigoJam のフォントが CC BY ライセンスを明示しています。（CC BY IchigoJam)
  そのため、本ライブラリもこのライセンスを継承し、CC BY IchigoJam で提供いたします。
  営利目的も含めて自由にご利用いただけますが（書籍などでのご利用も構いません）、
  ライセンスを含めた著作表記は「CC BY IchigoJam」です。
  紙面など URL をリンクできない場合は、リンクを URL 表記にして下さい。
  「CC BY IchigoJam http://ichigojam.net/ 」
 
 *
 */

#include <avr/interrupt.h>
//#include <avr/wdt.h>
#include "misakiUTF16.h"
#include "LED8x8Matrix.h"
#include "libBitmap.h"
#include "msgdata.h"
#include "command.h"
#include "ichigojamFont.h"

#define TIMERTICK 330         // ダイナミック駆動リフレッシュ周期(μ秒)
#define BPS       115200      // シリアル通信速度
#define MY_SIGNATUR "IMG094"  // 内部EEPROM先頭シグニチャ

//***********************************************************
//   LED用出力ピンの定義
//   LEDマトリックス製品対応
//    MNA20SR092G, OSL641501-ARA,LD-1088AS (カソードコモン)
//    OSL641501-BRA,1588BS,HSN-0788UR (アノードコモン)
//***********************************************************

// LEDマトリックス ピン定義

// row側LEDタイプ(LED_ANODE or LED_CATHODE)
#define LED_TYPE LED_ANODE

// 横(COL)にArduino ピンNoの設定
#define COL1  13
#define COL2  7
#define COL3  6
#define COL4  16
#define COL5  4
#define COL6  15
#define COL7  11
#define COL8  10

// 縦(ROW)にArduino ピンNoの設定
#define ROW1  17
#define ROW2  12
#define ROW3  2
#define ROW4  14
#define ROW5  9
#define ROW6  3
#define ROW7  8
#define ROW8  5

//*****************************
// グローバル変数
//*****************************

// COL,ROWのピン割り付けテーブル
uint8_t col[8] = {COL1,COL2,COL3,COL4,COL5,COL6,COL7,COL8};
uint8_t row[8] = {ROW1,ROW2,ROW3,ROW4,ROW5,ROW6,ROW7,ROW8};

// LEDマトリックスオブジェクトの宣言 colピン配列, row ピン配列, LED row側タイプ, リフレッシュ間隔(usec)
LED8x8Matrix mx(col, row, LED_TYPE, TIMERTICK);

MsgData md;           // メッセージデータ管理オブジェクトの宣言
command cm(md, mx) ;  // コマンドオブジェクトの宣言

//*********************************
// メイン処理
//*********************************
void setup() {
  randomSeed(analogRead(5));
  Serial.begin(BPS);
  
  // デフォルト設定値を一旦ロードする
  md.defaultConfig();   
  // 設定値の取得
  if ( md.checkSignature(MY_SIGNATUR) ) {
    // 設定値が保存してある場合は、設定値をロードする
    md.loadConfig();
  } else {
    // 設定値が未保存の場合は、EEPROMの初期設定を行う
    md.defaultConfig();              // デフォルト値をロードする
    md.saveSignature(MY_SIGNATUR);   // シグニチャの登録
    md.saveConfig();                 // 設定値の初期設定
  }

  mx.start();  // LEDマトリックスの駆動開始
  cm.logo();   // 起動時ロゴメッセージ表示
}

//***********************************
// ループ処理
//***********************************
void loop() { 
  cm.run();  
}

