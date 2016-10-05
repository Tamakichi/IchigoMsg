/*
 * メッセージデータ管理
 * 2016/09/30 by たま吉さん
 */


#include <avr/pgmspace.h>
#include "msgdata.h"
#include "LED8x8Matrix.h"

// 起動時メッセージ定義テータ (8x8 = 64バイト)
static const char startMessage[] PROGMEM  =   {
  0x25,0x5A,0x81,0xA5,0x81,0x81,0x42,0x3D,
  0xF0,0x08,0x04,0x07,0x04,0x04,0x08,0xF0,
  0x00,0x00,0x08,0xF0,0x00,0x00,0x00,0x00,
  0x84,0x84,0xB7,0xA5,0xA5,0xA5,0xB5,0x00,
  0x00,0x40,0x1D,0x55,0x55,0x5D,0x45,0x1C,
  0x1F,0x15,0xD5,0x55,0x55,0x55,0xD1,0x00,
  0x00,0x00,0x77,0x45,0x75,0x17,0x71,0x07,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

//
// シグニチャのロード
//
void MsgData::loadSignature(uint8_t* sign) {
  for (uint8_t i=0; i< EM_SIGNATURE_SIZE; i++) 
    sign[i] = EEPROM.read( EM_SIGNATURE_ADR + i);
}

//
// シグニチャの保存
//
void MsgData::saveSignature(uint8_t* sign) {
  for (uint8_t i = 0; i< EM_SIGNATURE_SIZE; i++) 
    EEPROM.write(EM_SIGNATURE_ADR + i, sign[i]);
}

//
// シグニチャの比較チェック
// 引数:sign 比較したいシグニチャ
// 戻り値 0:不一致 1:一致
// 
uint8_t MsgData::checkSignature(uint8_t* sign) {
  //uint8_t em_sign[EM_SIGNATURE_SIZE];
  //loadSignature(em_sign);
  for (uint8_t i = 0; i < EM_SIGNATURE_SIZE; i++) {
    //if (em_sign[i] != sign[i])
     if (EEPROM.read( EM_SIGNATURE_ADR + i)  != sign[i] )
      return 0; // 不一致
  }
  return 1;

}

//
// 設定情報のロード
//
void MsgData::loadConfig() {
  EEPROM.get(EM_CONFIG_ADR, conf );
}

//
// 設定情報の保存
//
void MsgData::saveConfig() {
  EEPROM.put(EM_CONFIG_ADR, conf );
}

//
// デフォルト設定情報の取得
//
void MsgData::defaultConfig() {
  conf.startUpMode = 1;         // 起動直後動作モード(0:起動メッセージなし 1:起動メッセージ表示)
  conf.rotate = B11;            // 回転補正設定(B00 なし, B01 反時計90° B10 反時計180° B11 反時計270)
  conf.scroll = B1000;          // スクロール方向(なしB0000, B0001 左 ,B0010 右, B0100 上, B1000 下 … OR で同時指定可能)
  conf.speed  = 60;             // スクロールディレイ(msec)
  conf.playmode =EM_PLAY_STOP;  // 再生順番設定(停止)
//  conf.wait = 1000;           // メッセージ間隔(msec)
  conf.wait = 0;                // メッセージ間隔(msec)
  conf.startmsg = 6;            // 起動時メッセージ番号
  //conf.bps = 115200;          // シリアル通信速度bps
  conf.charwait = 0;            // 文字間隔(msec)
  conf.codetype = 0;            // 利用文字フォント(美咲フォント UTF-8)

  // デフォルトメッセージの設定
  conf.playPos = 0;     // メッセージ再生位置

  // メッセージリスト初期化
  memset(conf.playlist, EM_NULL, EM_MAX_MSGLIST);

  // 変数の初期化
  memset(strValue, 0, EM_VALUE_LEN*EM_MAX_VNUM);
}

//
// ユーザフォントデータのロード
//
void MsgData::loadFontData(uint8_t* font, uint8_t no) {
  for (uint8_t i = 0; i < EM_FONT_SIZE; i++) {
    font[i] = EEPROM.read( EM_FONT_ADR + EM_FONT_SIZE * no + i);
  }  
}

//
// ユーザフォントデータの保存
//
void MsgData::saveFontData(uint8_t* font, uint8_t no) {
  for (uint8_t i = 0; i < EM_FONT_SIZE; i++) {
    EEPROM.write( EM_FONT_ADR + EM_FONT_SIZE * no + i, font[i]);
  }  
}

//
// メッセージデータのシリアル出力
//
void MsgData::printMessage(uint8_t no) {
  char c;
  if (no < EM_MAX_MSGNUM) {
    // EEPROM上データ
    for (uint8_t i = 0; i < EM_MAX_MSGLEN; i++) {
      if (!(c = EEPROM.read( EM_MESSAGE_ADR + EM_MAX_MSGLEN * no + i)) ) 
        break;
      Serial.write(c);
    }
  } else {
  // SRAM上データ
    Serial.print(ramMessage[no-EM_MAX_MSGNUM]);
/*
    for (uint8_t i = 0; i < EM_MAX_MSGLEN; i++) {
      if (!(c = ramMessage[no-EM_MAX_MSGNUM][i]) )
      break;
      Serial.write(c);
    }        
*/
  }
  Serial.println();
}

//
// メッセージデータのロード
//
void MsgData::loadMessage(char* msg, uint8_t no) {
  if (no < EM_MAX_MSGNUM) {
    // EEPROM上データ
    for (uint8_t i = 0; i < EM_MAX_MSGLEN; i++) {
      if (!(msg[i] = EEPROM.read( EM_MESSAGE_ADR + EM_MAX_MSGLEN * no + i)))
        break;
    }    
  } else {
    // SRAM上データ
    memcpy(msg, ramMessage[no-EM_MAX_MSGNUM], EM_MAX_MSGLEN);
/*
    for (uint8_t i = 0; i < EM_MAX_MSGLEN; i++) {
      if (!(msg[i] = ramMessage[no-EM_MAX_MSGNUM][i]))
        break;
    }        
*/
  }
}

//
// メッセージデータの保存
//
void MsgData::saveMessage(char* msg, uint8_t no) {
  if (no < EM_MAX_MSGNUM) {
    // EEPROM上データ    
    for (uint8_t i = 0; i< EM_MAX_MSGLEN; i++) {
      EEPROM.write( EM_MESSAGE_ADR + EM_MAX_MSGLEN * no + i, msg[i]);
      if (!msg[i])
        break;
    }    
  } else {
    // SRAM上データ
/*  
    for (uint8_t i = 0; i < EM_MAX_MSGLEN; i++) {
      if (!(ramMessage[no-EM_MAX_MSGNUM][i] = msg[i]))
        break;
    }
*/
  memcpy(ramMessage[no-EM_MAX_MSGNUM], msg, EM_MAX_MSGLEN);
  }
}

//
// メッセージデータの追記保存
//
void MsgData::saveAppendMessage(char* msg, uint8_t no) {
  uint8_t pos = 0;
  uint8_t i;
  uint8_t p;
  
  if (no < EM_MAX_MSGNUM) {
    // EEPROM上データ

    // 末尾の検索
    for (i = 0; i < EM_MAX_MSGLEN; i++) {
      if (EEPROM.read ( EM_MESSAGE_ADR + EM_MAX_MSGLEN * no + i) == 0)
        break;
    }
    pos = i;
    if (pos >= EM_MAX_MSGLEN) 
      return;
        
    p = 0;
    for (i = pos; i< EM_MAX_MSGLEN; i++) {
      EEPROM.write( EM_MESSAGE_ADR + EM_MAX_MSGLEN * no + i,msg[p]);
      if (!msg[p])
        break;
      p++;
    }    
  } else {
    // SRAM上データ
    // 末尾の検索
    for (i = 0; i < EM_MAX_MSGLEN; i++) {
      if (ramMessage[no-EM_MAX_MSGNUM][i] == 0)
        break;
    }
    pos = i;
    if (pos >= EM_MAX_MSGLEN) 
      return;

    p = 0;
    for (i = pos; i < EM_MAX_MSGLEN; i++) {
      if (!(ramMessage[no-EM_MAX_MSGNUM][i] = msg[p]))
        break;
      p++;
    }
  }
}

//
// ロゴメッセージデータのロード
//
void MsgData::loadLogoMessage(uint8_t*font , uint8_t n) {
  for (uint8_t i = 0;  i < 8; i++)
    //font[i] = pgm_read_byte_near(startMessage + n * 8 + i);
    font[i] = pgm_read_byte(startMessage + n * 8 + i);
}

//
// 変数への値登録
//
uint8_t MsgData::setValue(uint8_t no, char* str) {
  if (strlen(str) >= EM_VALUE_LEN) 
    return 1;
  strcpy(&strValue[no][0], str);
  return 0;
}

//
// 変数値の取得
//
char* MsgData::getValue(uint8_t no) {
  return &strValue[no][0];
}

//
// EEPROMの初期化
//
void MsgData::clearEEPROM() {
 for (int i = 0 ; i < EEPROM.length() ; i++) 
    EEPROM.write(i, 0);
}

