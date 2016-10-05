//
// 汎用LED8x8マトリックス制御用クラス
// 2016/09/20 by たま吉さん
//
#include <avr/interrupt.h>
#include <TimerOne.h>
#include "LED8x8Matrix.h"
#include "libBitmap.h"

//
// digitalWrite高速化版
//
void LED8x8Matrix::new_digitalWrite(uint8_t pin, uint8_t val) {
  uint8_t bit = digitalPinToBitMask(pin);
  volatile uint8_t *out = portOutputRegister(digitalPinToPort(pin));
  if (val)
    *out |= bit;  
  else
    *out &= ~bit;
}

// 点灯する行の選択
// y: 行(0〜7)
void LED8x8Matrix::selectRow(uint8_t y) {
  for(uint8_t i=0; i < DOTNUM; i++)
    new_digitalWrite(row[i], row_led_off); // 指定行以外は消灯に
  new_digitalWrite(row[y], row_led_on);    // 指定行を点灯可能に
}

// 1行分データの出力
// d:1行分(8ドット)パターン
void LED8x8Matrix::setData(uint8_t d) {
  uint8_t msk = B10000000;
  for (uint8_t i = 0; i< DOTNUM; i++) {
    new_digitalWrite(col[i], (msk & d) ? col_led_on: col_led_off); // LED消灯    
    msk>>=1;
  }  
}

// ドット単位のダイナミック駆動
void LED8x8Matrix::update_dot() {
  setData(0);
  selectRow(line);
  setData(pdata[line] & (B10000000>>colno));
  colno++;
  if (colno == DOTNUM ) {
    colno =0;
    line++;
    if (line == DOTNUM) 
      line = 0;  
  }
}

// ドットマトリックスの表示OFF
void LED8x8Matrix::matrix_off() {
  for (uint8_t i = 0; i < DOTNUM; i++) 
    new_digitalWrite(col[i], col_led_off);
  for (uint8_t i = 0; i < DOTNUM; i++) 
    new_digitalWrite(row[i], row_led_off);
}

// バッファクリア
void LED8x8Matrix::clear_buf() {
  memset(pdata,0,DOTNUM);
}

// LEDタイプ設定
void LED8x8Matrix::setType(uint8_t _type) {
  type = _type; 
  if (type == LED_ANODE) {
    row_led_on  = HIGH;
    row_led_off = LOW;
    col_led_on  = LOW;
    col_led_off = HIGH;
  } else {
    row_led_on  = LOW;
    row_led_off = HIGH;
    col_led_on  = HIGH;
    col_led_off = LOW;            
  }
}

// 初期化
void LED8x8Matrix::init(uint8_t*_col, uint8_t* _row, uint8_t _type, uint16_t _tm) {
  line = 0;
  colno = 0;
  clear_buf();
  attachPins(_col, _row);
  setType(_type);
  setIntervalTime(_tm);
  
  for (uint8_t i=0; i < DOTNUM; i++) {
    // ピンモードの設定
    pinMode(col[i], OUTPUT);
    pinMode(row[i], OUTPUT);
  }
}

static LED8x8Matrix* _obj;
static void isr() {
  _obj->update_dot();
}

// 駆動開始
void LED8x8Matrix::start() {
  _obj = this;
  Timer1.initialize(intervalTime); 
  Timer1.attachInterrupt(isr);
}

// ビットマップのスクロル挿入表示
void LED8x8Matrix::scrollIn(uint8_t* _bmp, uint8_t _mode, uint16_t _dt) {
  scrollInFont(pdata, DOTNUM, DOTNUM, _bmp, DOTNUM, DOTNUM, _dt, _mode);
}

// アクション付き表示
// 引数
//  _bmp  : 表示ビットマップ
//  rotate: ビットマップ回転補正
//  scroll: スクロールモード(0～15)
//  pitch : スクロールピッチ(msec)
//  cwait : 文字表示ウエイト(msec)
//
void LED8x8Matrix::actionDraw(uint8_t* _bmp, uint8_t rotate, uint8_t scroll, uint8_t pitch, uint16_t cwait) {
  rotateBitmap(_bmp, DOTNUM, DOTNUM, rotate);
  scrollIn(_bmp, scroll, pitch); 
  if (cwait)
    delay(cwait);
}

