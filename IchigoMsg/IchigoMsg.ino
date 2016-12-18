/*
 * IchigoMsg v 0.95  8x8ドットマトリックスLEDメッセージ arduino pro mini バージョン
 *  2016/9/30 by たま吉さん 
 *  2016/10/06 LEDドットマトリックスのピン設定部の修正
 *  2016/12/18 未登録フォントコード指定時に豆腐(□)が表示されない不具合対応
 *
 
<ライセンスにおける注意事項>
 本プログラムのフォントデータの一部に IchigoJam 1.2.1 のフォントデータを使用しています。
 IchigoJam のフォントが CC BY ライセンスを明示しています。（CC BY IchigoJam)
 そのため、本プログラムもこのライセンスを継承し、CC BYで公開いたします。
 営利目的も含めて自由にご利用いただけますが（書籍などでのご利用も構いません）、
 ライセンスを含めた著作表記は「CC BY IchigoJam / Tamakichi-San」です。
 紙面など URL をリンクできない場合は、リンクを URL 表記にして下さい。
 「CC BY IchigoJam http://ichigojam.net/ Tamakichi-San https://github.com/Tamakichi」
 
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


//*****************************
// グローバル変数
//*****************************

// LEDマトリックス ピン定義

// HSN-0788UR (アノードコモン) Arduino puro mini 裏面取り付け

#define LED_TYPE LED_ANODE // row側LEDタイプ(LED_ANODE or LED_CATHODE)
uint8_t col[8] = {13,  7, 6, A2, 4, A1, 11, 10};  // COL 1-8 へのArduino 割り付けピン定義
uint8_t row[8] = {A3, 12, 2, A0, 9,  3,  8,  5};  // ROW 1-8 へのArduino 割り付けピン定義

/*
// HSN-0788UR (アノードコモン) Arduino puro mini 表面取り付け
#define LED_TYPE LED_ANODE // row側LEDタイプ(LED_ANODE or LED_CATHODE)
uint8_t col[8] = {6,  12, 13,  3, A1,  4,  8,  9};  // COL 1-8 へのArduino 割り付けピン定義
uint8_t row[8] = {2,   7, A3,  5, 10, A2, 11, A0};  // ROW 1-8 へのArduino 割り付けピン定義

/*
// MNA20SR092G (カソードコモン) Arduino puro mini 表面取り付け
#define LED_TYPE LED_CATHODE // row側LEDタイプ(LED_ANODE or LED_CATHODE)
uint8_t col[8] = {6,  12, 13,  3, A1,  4,  8,  9};  // COL 1-8 へのArduino 割り付けピン定義
uint8_t row[8] = {2,   7, A3,  5, 10, A2, 11, A0};  // ROW 1-8 へのArduino 割り付けピン定義
*/

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

