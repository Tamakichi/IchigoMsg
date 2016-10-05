/*
 * コマンド管理クラス
 * 2016/09/30 by たま吉さん
 *
 */
 
#ifndef ___command_h___
#define ___command_h___

#include <Arduino.h>
#include "msgdata.h"
#include "LED8x8Matrix.h"

// コマンドリクエストチェックコード
#define CMD_REQ_NONE      0   // リクエスト無し
#define CMD_DONE_OK       1   // リクエスト処理正常
#define CMD_DONE_OK_RST   2   // コマンド処理正常(要リスタート)
#define CMD_DONE_OK_STOP  3   // コマンド処理正常(要停止)

#define MAXLINELEN  64       // シリアル受信最大長
#define FONTHEXLEN  16       // HEX形式フォントデータ長
#define MAXDELAYTM  32767    // 最大待ち時間(文字、１文用）

class command {
  // メンバー変数
  private:
    MsgData* md;          // メッセージデータ管理の参照
    LED8x8Matrix* mx;     // LEDマトリックス制御の参照
    char buf[129];        // メッセージ表示用バッファ
    char *pUTF8;          // メッセージ文参照位置
    uint8_t flgRestart;   // 再スタート要求不フラグ
    
    char line[MAXLINELEN];// 1行読み込みバッファ
    uint8_t flgSilent;    // 出力モード( 0:サイレント 1:通常(デフォルト) 2:IchigoJamモード
  
  // メンバー関数
  private:
    uint8_t getCmommandCode(char *str) ;
    char * skipSpace(char* str) ;
    char* skipComma(char* str);
    void printHex2(uint8_t v);
    void iHead();
    void response(char* str);
    uint8_t hextoi(uint8_t c) ;
    uint8_t getDrctNo(uint8_t d);
    char* str2Uint(int16_t* value, char* str) ;
    char* skipCtrCommand(char* str);
    void drawBitmap(uint8_t* pf,uint8_t flg = 0) ;  
    uint8_t getMsglisetParam(uint8_t* list, char* str, char ed = 0) ;
    char* getFontDataParam(uint8_t* font, char* str);
    char* getUTF16FontDataParam(uint8_t* font, char* str);
    uint8_t readLine();
    int8_t checkRequest();
    uint8_t readCommand(uint8_t& cmdCode);
    char* doCtrlCommand(char* str);
    char* DrawLEDMatrix(char* pUTF8) ;
    void playLEDMatrix(char* pUTF8);
    void printFont(uint8_t n);
    void playLogo();
    void printFontData(uint8_t no);
    void printOutData(uint8_t no, uint8_t type);    
  public:
    command(MsgData& _md, LED8x8Matrix& _mx) {
      md =&_md; mx = &_mx;
      flgSilent = 2;
      flgRestart = false;
    };
    
    uint8_t run();  
    uint8_t logo();
};

#endif
