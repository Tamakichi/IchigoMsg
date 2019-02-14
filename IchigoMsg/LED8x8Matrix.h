//
// 汎用LED8x8マトリックス制御用クラス
// 2016/09/20 by たま吉さん
//

#ifndef ___LED8x8Matrix_h___
#define ___LED8x8Matrix_h___

#include <Arduino.h>
#define LED_ANODE   0
#define LED_CATHODE 1

#define DOTNUM 8  // 縦横ドット数
class LED8x8Matrix {
  public:
    uint8_t   pdata[8];   // 表示用フレームバッファ
    
  private:
    // バッファ制御用
    uint8_t   line;       // ダイナミック駆動_行
    uint8_t   colno;      // ダイナミック駆動_列    

    // ハードウェア定義(ピン割り付け)
    uint8_t*  col;        // columnピン割り付けテーブルポインタ
    uint8_t*  row;        // rowピン割り付けテーブルポインタ
    uint8_t   type;       // LEDタイプ

    // LED制御設定用
    uint8_t   row_led_on;   // HIGH or LOW
    uint8_t   row_led_off;  // HIGH or LOW
    uint8_t   col_led_on;   // HIGH or LOW
    uint8_t   col_led_off;  // HIGH or LOW

  public:
    uint16_t  intervalTime;  // 割り込み間隔

  private:
    void selectRow(uint8_t y);
    void setData(uint8_t d);
    void attachPins(uint8_t*_col, uint8_t* _row)  // ピン設定のアタッチ
     { col = _col; row = _row; }
    void setType(uint8_t _type);                  // LEDタイプ設定
    void setIntervalTime(uint16_t _tm)            // リフレッシュ間隔指定
     { intervalTime = _tm;}
       
  public:
    void update_dot();
    static void new_digitalWrite(uint8_t pin, uint8_t val);

  public:
    // コンストラクタ
    LED8x8Matrix() {}     
    LED8x8Matrix(uint8_t*_col, uint8_t* _row, uint8_t _type, uint16_t _tm)
    {init(_col, _row, _type, _tm); }     

    // 初期化 
    void init(uint8_t*_col, uint8_t* _row, uint8_t _type, uint16_t _tm);
    // 駆動開始
    void start(); 
    
    void matrix_off();
    void clear_buf();

    // ビットマップのスクロル挿入表示
    void scrollIn(uint8_t* bmp, uint8_t mode, uint16_t dt);
    // ビットマップのアクション表示
    void actionDraw(uint8_t* _bmp, uint8_t rotate, uint8_t scroll, uint8_t pitch, uint16_t cwait);
};
#endif
