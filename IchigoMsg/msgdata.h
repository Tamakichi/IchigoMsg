/*
 * メッセージデータ管理
 * 2016/09/30 by たま吉さん
 */

#ifndef ___msgdata_h___
#define ___msgdata_h___

#include <Arduino.h>
#include <EEPROM.h>

#define EM_SIGNATURE_SIZE 6     // 署名サイズ
#define EM_MAX_SRAM_MSGNUM 2    // SRAM内登録可能メッセージ数
#define EM_MAX_MSGNUM     6     // EEPROM内登録可能メッセージ数
#define EM_MAX_ALLMSGNUM  (EM_MAX_SRAM_MSGNUM+EM_MAX_MSGNUM)  // 利用可能メッセージ数(EEPROM+SRAM)
#define EM_MAX_MSGLIST    8     // メッセージリスト最大登録数
#define EM_FONT_SIZE      8     // フォントサイズ
#define EM_MAX_FONTNUM    24    // 登録可能フォント数
#define EM_MAX_MSGLEN     128   // 最大メッセージ長
#define EM_NULL           0xff  // 未定義
#define EM_MAX_VNUM       10    // 変数利用可能数
#define EM_VALUE_LEN      16    // 変数長

// EEPROM メモリアドレス(0～0x3ff)
#define EM_SIGNATURE_ADR  0x000                                        // "LMXxxx" 6バイト
#define EM_CONFIG_ADR     (EM_SIGNATURE_ADR+EM_SIGNATURE_SIZE)         // 設定情報 14バイト
#define EM_FONT_ADR       (EM_CONFIG_ADR+sizeof(Config))               // ユーザーフォント 8x24(192バイト)
#define EM_MESSAGE_ADR    (EM_FONT_ADR+EM_FONT_SIZE*EM_MAX_FONTNUM)    // メッセージデータ 128x6
#define EM_FRRE_ADR       (EM_MESSAGE_ADR+EM_MAX_MSGLEN*EM_MAX_MSGNUM) // 未使用領域先頭

//再生設定
#define EM_PLAY_STOP      0 // 停止
#define EM_PLAY_SEQ_ONCE  1 // シーケンシャル(1回のみ)
#define EM_PLAY_SEQ_EVER  2 // シーケンシャル(繰り返し)
#define EM_PLAY_RND_EVER  3 // ランダム繰り返し

// 設定情報(22バイト)
typedef struct _Config {
  uint8_t   startUpMode ; // 起動直後動作モード(0:起動メッセージなし 1:起動メッセージ表示)
  uint8_t   rotate;       // 回転補正設定(B00 なし, B01 反時計90° B10 反時計180° B11 反時計270° )
  uint8_t   scroll;       // スクロール方向(なしB0000, B0001 左 ,B0010 右, B0100 上, B1000 下 … OR で同時指定可能)
  uint16_t  speed;        // スクロールディレイ(msec)
  uint8_t   playmode;     // 再生設定(0:停止, 1:シーケンシャル(1回のみ) 2:シーケンシャル(繰り返し) 3:ランダム繰り返し)
  uint16_t  wait;        // メッセージ間隔(msec)
  uint8_t   startmsg;     // 起動時メッセージ番号
  //uint32_t  bps;          // シリアル通信速度bps
  uint8_t   playlist[EM_MAX_MSGLIST]; // 再生メッセージリスト(メッセージ番号 0～7,FF:未定義)
  uint8_t   playPos;      // 再生メッセージ位置
  uint16_t  charwait;     // 文字間隔(msec)
  uint8_t   codetype;     // 利用文字コード (0:UTF-8, 1:IchigoJam Ascii)
  uint8_t   ledType;      // LED row側タイプ(ANODE or CATHODE)
} Config;

// メッセージデータ管理クラス
class MsgData {
  // メンバー変数定義
  public:
    Config conf;  // 設定値情報
  private:    
    char ramMessage[EM_MAX_SRAM_MSGNUM][EM_MAX_MSGLEN]; // RAM上一時メッセージ
    char strValue[EM_MAX_VNUM][EM_VALUE_LEN];           // 変数テーブル

  // メンバー関数
  public:
    MsgData() {};  
    void loadSignature(uint8_t* sign);
    void saveSignature(uint8_t* sign);
    uint8_t checkSignature(uint8_t* sign);
    void loadConfig();
    void saveConfig();
    void defaultConfig();
    void loadFontData(uint8_t* font, uint8_t no);
    void saveFontData(uint8_t* font, uint8_t no);
    void loadMessage(char* msg, uint8_t no);
    void saveMessage(char* msg, uint8_t no);
    void saveAppendMessage(char* msg, uint8_t no);
    void loadLogoMessage(uint8_t*font , uint8_t n);
    void printMessage(uint8_t no);
    uint8_t setValue(uint8_t no, char* str);
    char* getValue(uint8_t no) ;
    void clearEEPROM();
};
#endif


