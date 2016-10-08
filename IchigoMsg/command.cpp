/*
 * コマンド管理クラス
 * 2016/09/30 by たま吉さん
 *
 */

#include <string.h>
#include "config.h"
#include "command.h"
#include "msgdata.h"
#include "libBitmap.h"
#include "LED8x8Matrix.h"
#include "misakiUTF16.h"

#if CONF_USE_ICHIGOFONT==1
  #include "ichigojamFont.h"
#endif

#define ICHIHO_LINE_DELAY 400 // IchigoJam用 シリアル出力行 DELAY 

// コマンドコード(最後のCOMMANDNUM は登録数として利用)
typedef enum _cmdCode {
  CMD_SETMSGA, CMD_SETMSG, CMD_DELMSG, CMD_SETPLIST, CMD_OUT, CMD_GETMSG,CMD_SCRL, 
  CMD_ROTATE, CMD_STOP, CMD_SETFONT, CMD_PRINT, CMD_SPRINT, CMD_SAVE, CMD_LOAD,CMD_PLAY, CMD_GETFONT,
  CMD_MWAIT, CMD_CWAIT, CMD_DEFAULT, CMD_SETCODE, CMD_OUTPUT,CMD_SETVALUE,
  CMD_GETVALUE, CMD_CLSROM, CMD_LOGO, CMD_CLS,
  COMMANDNUM
} ECmdCode;

typedef struct {
  char*   name;    // コマンド名
  uint8_t code;    // コマンドコード
  int8_t  param;   // パラメタ数  0:引数なし, 1:整数1, 2:整数2 , -10:整数0or1, -1: 文字列1つ, -2:整数1,文字1 
} CmdList ;

CmdList cmdlist[] = {
  { "?",        CMD_SPRINT,   -1 }, // 即時メッセージ表示
  { "print",    CMD_PRINT,    -1 }, // 即時メッセージ表示
  { "play",     CMD_PLAY,    -10 }, // 再生設定
  { "stop",     CMD_STOP,      0 }, // 再生停止
  { "setlist",  CMD_SETPLIST, -1 }, // 再生リストの登録
  { "scrl",     CMD_SCRL,      2 }, // スクロール設定
  { "rotate",   CMD_ROTATE,    1 }, // 回転補正設定
  { "setmsga",  CMD_SETMSGA,  -2 }, // メッセージ追記登録
  { "setmsg",   CMD_SETMSG,   -2 }, // メッセージ登録
  { "delmsg",   CMD_DELMSG,    1 }, // メッセージ削除
  { "out",      CMD_OUT,      -1 }, // ダイレクトフォント表示 NG
  { "getmsg",   CMD_GETMSG,  -10 }, // 登録メッセージ取得
  { "setfont",  CMD_SETFONT,  -2 }, // フォント登録
  { "getfont",  CMD_GETFONT, -10 }, // 登録フォント取得
  { "save",     CMD_SAVE,      0 }, // 設定値保存
  { "load",     CMD_LOAD,      0 }, // 設定値取得
  { "mwait",    CMD_MWAIT,     1 }, // メッセージ表示間隔時間設定
  { "cwait",    CMD_CWAIT,     1 }, // 文字間隔時間設定
  { "setcode",  CMD_SETCODE,   1 }, // 文字コード系設定
  { "setout",   CMD_OUTPUT,    1 }, // 出力形式設定
  { "setvalue", CMD_SETVALUE, -2 }, // 変数値設定
  { "getvalue", CMD_GETVALUE,-10 }, // 変数値取得
  { "logo",     CMD_LOGO,    -10 }, // ロゴの表示・起動時ロゴ表示設定
  { "default",  CMD_DEFAULT,   0 }, // デフォルト設定値のロード
  { "clsrom",   CMD_CLSROM,    0 }, // EEPROMの初期化
  { "cls",      CMD_CLS,       0 }, // 表示のクリア
};

// スクロール方向
PROGMEM static const uint8_t drct[] = {
    B0000,B0100, B0110, B0010, B1010, B1000, B1001, B0001, B0101, 
    B0111,B1011,B1101,B1110, B1111,B0011,B1100,
};

//
// コマンド登録番号の取得
// 戻り値 0～:コマンドコード 0xff:該当なし
//
uint8_t command::getCmommandCode(char *str) {
  uint8_t code = EM_NULL;
  for (uint8_t i = 0; i < COMMANDNUM; i++) {
    if ( !strncasecmp( str, cmdlist[i].name, strlen(cmdlist[i].name) ) ) {
      code = i;
//      Serial.print(i, DEC);Serial.write(':');
//      Serial.println(cmdlist[i].name);
      break;
    }
  }
  return code;
}

// ロゴの表示
uint8_t command::logo() {
  if (md->conf.startUpMode) {
    playLogo();
    mx->matrix_off();
  }
  return md->conf.startUpMode;
}

// コマンド実行
uint8_t command::run() {
  uint8_t mno;    // 再生メッセージ番号
  uint8_t cnt;    // 再生メッセージリスト登録件数
  uint8_t rc;     
  char* p;

  // 再生表示メッセージの設定
  buf[0] = 0;
  if ( md->conf.playmode == EM_PLAY_SEQ_ONCE || 
       md->conf.playmode == EM_PLAY_SEQ_EVER ) {
     // ** シーケンシャル表示 *************************************************************************
     mno = md->conf.playlist[md->conf.playPos]; // メッセージリストからメッセージ番号取得
     if (mno != EM_NULL)
        md->loadMessage(buf,mno); // メッセージデータ取得
  } else if (md->conf.playmode == EM_PLAY_RND_EVER ) {
    // ** ランダム表示 *******************************************************************************
    cnt=0; while (md->conf.playlist[cnt] != EM_NULL && cnt < EM_MAX_MSGLIST ) cnt++; // 登録メッセージ数取得
    if (cnt) {
      mno = md->conf.playlist[random(0,cnt)]; // ランダムに再生メッセージを決定
      md->loadMessage(buf, mno); // メッセージデータ取得
    }
  }/* else if (md->conf.playmode == EM_PLAY_STOP) {
      // ** 停止状態 **********************************************************************************
      buf[0] = 0; // 空メッセージを設定
  }*/
 
  // 再生メッセージ表示処理(ループ)
  // (1文字表示毎にシリアル経由のコマンド受信チェック&コマンド実行を行う)
  pUTF8 = buf;
  if (md->conf.playmode != EM_PLAY_STOP) {
    while(*pUTF8) {
      // シリアル経由のコマンドリクエスト処理
      flgRestart = false;
      if ((rc = checkRequest()) == CMD_DONE_OK_RST) {
         // 要リスタート
        flgRestart = true;
        break;
      } else if (rc == CMD_DONE_OK_STOP) {
        // 要停止
        break;
      }
      // 埋め込み制御命令の処理
      p = pUTF8;
      pUTF8 =doCtrlCommand(pUTF8);  // 埋め込み
      if (pUTF8 != p) continue;
  
      // LEDマトリックス制御
      pUTF8 = DrawLEDMatrix(pUTF8) ;
    }
  } 

  // 再生停止時or再生対象メッセージが空の場合の
  // シリアル経由のコマンドリクエスト処理
  if (!*pUTF8 || md->conf.playmode != EM_PLAY_STOP) {
      if ((rc = checkRequest()) == CMD_DONE_OK_RST)
        flgRestart = true;
  }
  
  // メッセージ間待ち
  if (md->conf.wait)
    delay(md->conf.wait);
  
  // 次の再生表示メッセージ表示の決定
  if (flgRestart) {
    // リスタート
    md->conf.playPos = 0;
    flgRestart = false;
  } else {
    // シーケンシャル再生 
    if (md->conf.playmode == EM_PLAY_SEQ_EVER || md->conf.playmode == EM_PLAY_SEQ_ONCE) {
      md->conf.playPos++;
      if (md->conf.playPos == EM_MAX_MSGLIST || md->conf.playlist[md->conf.playPos] == EM_NULL) {
        md->conf.playPos = 0;
        if (md->conf.playmode == EM_PLAY_SEQ_ONCE) 
          md->conf.playmode = EM_PLAY_STOP;
      }
    }
  }  
}

// IchigoJam用コメントアウト出力
void command::iHead() {
  if (flgSilent == 2) Serial.write('\'');
}

// コマンド実行の応答
void command::response(char* str) {
  if (flgSilent) iHead(), Serial.println(str);
}

//
// フォントデータのシリアル出力
//

void command::printFontData(uint8_t no) {
  uint8_t font[EM_FONT_SIZE];
  md->loadFontData(font, no);
  for (uint8_t i = 0; i < EM_FONT_SIZE; i++)
    printHex2(font[i]);
  Serial.println();
}

// データ出力
void command::printOutData(uint8_t no, uint8_t type) {
  if (type == CMD_GETFONT) 
    printFontData(no);
  else if (type == CMD_GETMSG)
    md->printMessage(no);
  else if (type == CMD_GETVALUE)
    Serial.println(md->getValue(no)); 
}

//
// 1行読み取り(@で始まる行のみ有効)
//   戻り値 0:読み取り未完、 1:読み取り完了 2:バッファオーバー
//
uint8_t command::readLine() {
  uint8_t rc =0;
  static uint8_t sts = 1; // 受信状態: 1)初期 2)コマンド読み込み中 3)非コマンドスキップ中 4)コマンド確定 5)非コマンドスキップ完了
  static uint8_t pos = 0; // バッファ格納位置
  char c;                 // 読み取りデータ

  while ( Serial.available() ) {
    c = Serial.read();  
/* 
    Serial.print("sts=");Serial.print(sts);
    Serial.write(' ');Serial.write(c);Serial.write(':');
    printHex2(c);
    Serial.println();
*/   
    // コマンド開始、改行のチェック
    if (c == '@') {
      if (sts == 1) 
        sts = 2; // [1)初期] => [2)コマンド読み込み中]に移行
    } else if (c == 0x0d || c == 0x0a) {
      if (sts == 2) {
        // [2)コマンド読み込み中] => [4)コマンド確定] => [1)初期]に移行
          line[pos] = 0;
          pos = 0;
          sts = 1;
          rc = 1;
      } else if (sts == 3) {
        // [3)非コマンドスキップ中] => [5)非コマンドスキップ完了] => [1)初期] に移行
        sts = 1;
      }
    } else {
      if (sts == 1)
        sts = 3;
    }
    //[2)コマンド読み込み中]の処理
    if (sts == 2) { 
      if (pos >= MAXLINELEN-1) {
        // バッファーオーバー : [2)コマンド読み込み中] => [3)非コマンドスキップ中]に移行
        pos = 0; // バッファポインタ初期化
        sts = 3;
        rc = 2;
      } else {
        line[pos++] = c; // バッファに格納     
      }
    }
  }
  return rc;    
}

// コマンド受信チェック
// 戻り値
//  0:受信なし(CMD_REQ_NONE)
//  1:コマンド処理正常(CMD_DONE_OK)
//  2:コマンド処理正常_要リスタート(CMD_DONE_OK_RST)
//  3:コマンド処理正常_要停止(CMD_DONE_OK_STOP)
// -1:コマンドでない
// -2:未定義コマンド
// -3:引数エラー
// -4:バッファオーバー
//
int8_t command::checkRequest() {
  uint8_t rc;
  uint8_t cmd;
  rc = readLine();  // 1行読み取り
  if (rc == 2) {
    // バッファーオーバー（コマンド破棄)
    response("NG");
    return -4;    
  } else if (rc == 1) {
    // コマンド行取得
    rc = readCommand(cmd); // prvCmdにコマンドコードを返す
    if (rc == 0) {
      response("OK");
      if (cmd == CMD_STOP)
        return 3;
      if (cmd == CMD_SETPLIST || cmd == CMD_LOAD || cmd == CMD_DEFAULT || cmd == CMD_PLAY)
        return CMD_DONE_OK_RST;
      else
        return CMD_DONE_OK; 
    } else {
      response("NG");
    }
    return -rc;
  } 
  return CMD_REQ_NONE;
}

//
// コマンドの読み込み
// 引数
// code(out) 取得したコード
// 戻り値
// 0:正常 1:コマンドでない 2:未定義コマンド 3:引数エラー
// 

uint8_t command::readCommand(uint8_t& code) {
  uint8_t font[EM_FONT_SIZE];    // フォントデータ
  uint8_t list[EM_MAX_MSGLIST];  // メッセージリスト
  uint8_t listlen;               // メッセージリスト長
  char *str ;                    // 解析対象文字列ポインタ
  uint8_t no;
  uint8_t index;
  int8_t param;
  uint16_t value, value2;
  
  // [コマンドチェック]
  str = line;                       // 解析対象文字列ポインタに行バッファを指定
  str++;                            // '@'文字のスキップ
  index = getCmommandCode(str);     // コマンド番号取得

  //Serial.print("cmd=");
  //Serial.println(cmdlist[index].name);
  
  if (index == EM_NULL) return 2;      // 未定義コマンド(エラー)
  str+= strlen(cmdlist[index].name);   // 解析対象文字列ポインタを引数部に移動
  code = cmdlist[index].code;
  param = cmdlist[index].param;
  
//[各コマンドの処理] 
  // 引数無しコマンド STOP, SAVE , LOAD, DEFAULT, CLSROM
  if (param == 0) {
    // 引数無しコマンドの文法チェック処理 
    if (*(str = skipSpace(str))) return 3;

    if (code == CMD_STOP)                 // 再生停止
      md->conf.playmode = EM_PLAY_STOP;    
    else if (code == CMD_SAVE)            // 設定値保存
      md->saveConfig();   
    else if (code == CMD_LOAD)            // 設定値ロード
      md->loadConfig();    
    else if (code == CMD_DEFAULT)         // 設定値デフォルトロード
      md->defaultConfig();
    else if (code == CMD_CLSROM)          // EEPROM初期化
      md->clearEEPROM();
    else if (code == CMD_CLS)             // 表示のクリア
      mx->clear_buf();    
    return 0;
  }
//Serial.println("S1");
  // 第1引数が整数型コマンドの共通文法チェック処理
  if (param == 1 || param == 2 || param == -2) 
    if (!(str = str2Uint(&value, str))) return 3;    

//Serial.println("S2");
  // 第1引数が文字列型りコマンドの共通文法チェック処理
  if (param == -1 ) 
    if (!*(str = skipSpace(str))) return 3;

 // 比較的優先度の高いコマンド
 if (code == CMD_PRINT || code == CMD_SPRINT ) { // 即時メッセージ表示
    playLEDMatrix(str) ;
  } else if ( code == CMD_SETPLIST && (listlen = getMsglisetParam(list, str)) ) {  // メッセージリスト登録
    memcpy(md->conf.playlist, list, EM_MAX_ALLMSGNUM);
  } else if (code == CMD_OUT && getFontDataParam(font, str) ) {  // ダイレクトフォント表示
      drawBitmap(font);
  }

//Serial.println("S3");
  // 整数型引数1のコマンド ROTATE, DELMSG, MWAIT, CWAIT, SETCODE, OUTPUT
  if (param == 1) {
    if (*(str = skipSpace(str))) return 3;
    
    if (code == CMD_ROTATE && value < 4) {  // 回転補正
      md->conf.rotate = value;    
    } else if (code == CMD_MWAIT) {         // メッセージウェイト
      md->conf.wait = value;
    } else if (code == CMD_CWAIT) {         // 文字ウェイト
      md->conf.charwait = value;
    } else if (code == CMD_SETCODE && value < 2) {  // コード系指定
      md->conf.codetype = value;
    } else if (code == CMD_OUTPUT && value < 3) {   // 出力指定
      flgSilent = value;
    } else if (code == CMD_DELMSG && value < EM_MAX_ALLMSGNUM ) {   // メッセージ削除
      md->saveMessage(str, value);      // メッセージ登録(削除)
    } else {
      return 3; // 引数エラー
    }
    return 0;
  }

//Serial.println("S4");
  // 第1引数整数型1+ 第2引数ありのコマンド SETMSG, SETMSGA, SCRL, SETFONT, SETVALUE
  if (param == 2 || param == -2) {
    str = skipComma(str);
    if (!str || !*str) 
      return 3; // 引数エラー(第2位引数なし)
  
    if (code == CMD_SETMSG && value < EM_MAX_ALLMSGNUM) {          // メッセージ登録
      md->saveMessage(str, value);      // メッセージ登録(削除)            
    } else if (code == CMD_SETMSGA && value < EM_MAX_ALLMSGNUM) {  // メッセージ追記登録
      md->saveAppendMessage(str, value); // メッセージ追記登録   
    } else if (code == CMD_SCRL && value < 16 ) {                  // スクロール設定
      str = str2Uint(&value2, str); // スクロールウェイト取得
      if (!str || (value2 > MAXDELAYTM) ) return 3;
      if (*(str = skipSpace(str))) return 3;
      md->conf.scroll = pgm_read_byte(drct + value);
      md->conf.speed = value2;
    } else if (code == CMD_SETFONT && value < EM_MAX_FONTNUM) {   // フォントの登録 
      if (!(str = getFontDataParam(font, str)) ) return 3;
      if (*(str = skipSpace(str))) return 3;
      md->saveFontData(font, value);  
    } else if ( code == CMD_SETVALUE && value < EM_MAX_VNUM) {    // 変数の設定
      if (md->setValue(value, str)) return 3;
    } else {
      return 3; // 引数エラー
    }
    return 0;
  }

//Serial.println("S5");  
  // 引数整数型省略可能のコマンド GETMSG, GETFONT, GETVALUE, PLAY, LOGO
  if (param == -10) {
    if (!*(str = skipSpace(str))) { 
      // 引数無しの場合      
      if (code == CMD_PLAY) {
        md->conf.playmode =EM_PLAY_SEQ_ONCE; // シーケンシャル再生
      } else if (code == CMD_LOGO) {
        playLogo();      
      } else {
        if (flgSilent) {
          uint8_t count = 0;
          if (code == CMD_GETMSG) 
            count = EM_MAX_ALLMSGNUM;
          else if (code == CMD_GETFONT)   
            count = EM_MAX_FONTNUM;
          else if (code == CMD_GETVALUE)
            count = EM_MAX_VNUM;
          for (uint8_t i = 0; i < count; i++) {
            iHead();Serial.print(i,DEC); Serial.write(':');
            printOutData(i, code); 
            if (flgSilent == 2) delay(ICHIHO_LINE_DELAY);
          }
        }
      }      
      return 0;
    } else {
      // 引数ありの場合
      str = str2Uint(&value, str); if (*(str = skipSpace(str))) return 3; // 引数エラー      
    
      if ( (code == CMD_GETMSG &&  value < EM_MAX_ALLMSGNUM) ||
           (code == CMD_GETFONT && value < EM_MAX_FONTNUM) ||
           (code == CMD_GETVALUE && value < EM_MAX_VNUM) 
       ) {
        if (flgSilent) {
          iHead(); printOutData(value, code);
        }
      } else if (code == CMD_PLAY && value < 3) {
        md->conf.playmode = value;  // 再生モード設定
      } else if (code == CMD_LOGO && value < 2) {
        md->conf.startUpMode = value;
      } else {
        return 3; // 引数エラー      
      }
      return 0;
    }
  }

//Serial.println("S6");
  return 0;
}

//
// 空白文字のスキップ
//
char* command::skipSpace(char* str) {
  while ( isWhitespace(*str) ) str++;    
  return str;
}

//
// カンマのスキップ
// エラー時(カンマなし)はNULLを返す
//
char* command::skipComma(char* str) {
  str = skipSpace(str);
  if (*str == 0 || *str != ',') return NULL;
  str++;
  str = skipSpace(str);
  return str;
}

//
// メッセージリストパラメタ取得
// 戻り値 1～8:リスト長 0:パラメタ異常
//
uint8_t command::getMsglisetParam(uint8_t* list, char* str, char ed) {
  uint8_t len = 0;

  if (!*(str = skipSpace(str)) ) 
    return len;
  memset(list, EM_NULL, EM_MAX_MSGLIST);
  
  for(;;) {
    if (*str < '0' || *str > '9') 
      break;
    list[len] = *str - '0';
    len++,  str++;
    if (len >= EM_MAX_MSGLIST)
      break;
  }
  str = skipSpace(str);
  if (*str != ed) 
    len = 0;
  return len;
}

//
// 文字列数字変換(変換範囲 0～32767)
// 引数
//  value(OUT) 変換した値
//  str(IN) 変換対象文字列
// 戻り値
//  正常: 次の文字列 NULL 引数エラー
//
char* command::str2Uint(int16_t* value, char* str) {
  int32_t n = 0;
  int32_t base = 1;
  uint8_t cnt = 0;
  char * p = str;
  
//  cnt=0;
//  base = 1;
//  n = 0;

  p = skipSpace(p);
  while (*p >='0' && *p <= '9')
    cnt++, p++;
    
  if (cnt > 5 || cnt == 0) return NULL;
  str = p;
  p--;
  for (uint8_t i = 0; i < cnt; i++) {
    n += (*p - '0') * base;
    p--;
    base*=10;      
  }
  
  if (n > 32767) return NULL;
  *value = n;
  return str;
}

//
// 16進数文字 数値変換
// 引数     c(IN) 対象文字
// 戻り値   成功 0～15、 失敗 EM_NULL(0xff)
uint8_t command::hextoi(uint8_t c) {
  uint8_t rc = EM_NULL;
    if      (c >= '0' && c <= '9')  rc = c - '0';
    else if (c >= 'a' && c <= 'f')  rc = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')  rc = c - 'A' + 10;
  return rc;
}

//
// フォント定義パラメタ取得
// 引数
//  font(OUT) : フォントデータ(8バイト)
//  str       : 評価対象文字列 (16桁HEX文字)
// 戻り値
//  次の文字位置:正常 、NULL:異常
//
char* command::getFontDataParam(uint8_t* font, char* str) {
  uint8_t cnt = 0;
  char* p;
  uint8_t n;

  if (!*( str = skipSpace(str) ) ) return NULL;
  p = str;
  for (uint8_t i = 0; i < FONTHEXLEN; i++) {
    if (hextoi(*p) != EM_NULL)
        cnt++, p++;
    else
      break;
  }

  if (cnt != 16) return NULL;
  p = str;
  for (uint8_t i = 0; i < FONTHEXLEN; i++) {    
    n = hextoi(*p);    
    font[i>>1] = i&1 ? font[i>>1]+n : n<<4 ;
    p++;
  }
  return p;
}

//
// UTF16指定パラメタからフォント定義取得
// 引数
//  font(OUT) : フォントデータ(8バイト)
//  str       : 評価対象文字列 (16桁HEX文字)
// 戻り値
//  次の文字位置:正常 、NULL:異常
//
char* command::getUTF16FontDataParam(uint8_t* font, char* str) {
  uint8_t cnt = 0;
  char* p;
  uint8_t n;
  uint16_t code = 0;

  //memset(font, 0,8);
  if (!*( str = skipSpace(str) ) ) return NULL;
  p = str;
  for (uint8_t i = 0; i < 4; i++) {
    if (hextoi(*p) != EM_NULL)
      cnt++, p++;
    else
      break;
  }

  if (cnt != 4) return NULL;
  p = str;
  for (uint8_t i = 0; i < 4; i++) {    
    n = hextoi(*p);    
    code += n<<(12-i*4);
    p++;
  }
  getFontDataByUTF16(font, code);  
  return p;
}

//
// 2桁16進数桁数指定出力
//
void command::printHex2(uint8_t v) {
  if (v < 0x10)
    Serial.write('0');
  Serial.print(v, HEX);  
}

//
// 埋め込み制御コマンドのスキップ
//
char* command::skipCtrCommand(char* str) {
  for(;;) {
    if (*str == 0 || *str == '}') break;
    str++;
  }
  if (*str == '}') str++;
  return str;
}

// スクロール方向設定値変換
uint8_t command::getDrctNo(uint8_t d) {
  for (uint8_t i =0; i < 16 ; i++) { 
      if (pgm_read_byte(drct + i) == d)  
      return i;
  }
  return 0;  
}

//
// 埋め込みコマンド処理
// 戻り値 次の文字列ポインタ(埋め込みコマンドがエラーの場合、スキップする)
// 
char* command::doCtrlCommand(char* str) {
  uint16_t value;
  uint8_t font[EM_FONT_SIZE];
  char* pUTF8;
  uint8_t c;

  if (*str == '{') {
    str++;
    if (*str >='A' && *str <='Z')
      *str = *str - 'A' +'a';
// 登録フォント/IchigoJamフォント/utf16フォント/ビットマップ埋め込み********   
#if CONF_USE_ICHIGOFONT==1
    if (*str == 'f' || *str == 'a' || *str == 'u' || *str == 'o') {
#else
    if (*str == 'f' || *str == 'u' || *str == 'o') {
#endif
      c = *str;  
      str++; 
      if ( c == 'u') { // utf16
        if (!(str = getUTF16FontDataParam(font, str)) ) return skipCtrCommand(str);
      } else if ( c == 'o') { // bitmap
        if (!(str = getFontDataParam(font, str)) ) return skipCtrCommand(str);        
      } else { // 登録フォント or IchigoJamフォント
        if ( !(str = str2Uint(&value, str)) )  return skipCtrCommand(str);
        if (c == 'f') { // 登録フォント
          if (value >= EM_MAX_FONTNUM) return skipCtrCommand(str);
          md->loadFontData(font, value);       
#if CONF_USE_ICHIGOFONT==1
        } else { // IchigoJamフォント
          if (value >= 256) return skipCtrCommand(str);    
          getIchigoFont(font, value);
#endif
        }
     }      
    if( *str != '}') return skipCtrCommand(str);
    str++;
    drawBitmap(font);
    return str;
// 半角フォント2文字埋め込み ****************************************************
    } else if (*str == 'h') {
      uint8_t c1,c2,f1[EM_FONT_SIZE], f2[EM_FONT_SIZE];  
      str++;  if ( *str == '}' || *str < 0x20 || *str > 0x7d) return skipCtrCommand(str);
      c1 = *str; 
      str++;  if ( *str == '}' || *str < 0x20 || *str > 0x7d) return skipCtrCommand(str);
      c2 = *str; 
      str++;  if( *str != '}') return skipCtrCommand(str);
      str++;
      getFontDataByUTF16(f1, c1);
      getFontDataByUTF16(f2, c2);      
      for (uint8_t i=0; i < EM_FONT_SIZE; i++) {
        f1[i] |= f2[i]>>4;
      }
      drawBitmap(f1);
      return str;
// ドットの描画 ***************************************************************    
    } else if (*str >= '0' && *str <='7') {
      uint8_t d[8],x,y;
      c = getMsglisetParam(d , str, '}') ;
      if (c != 3) return skipCtrCommand(str);
      str+=3;
      if( *str != '}') return skipCtrCommand(str);         
      // rotate: B00 なし, B01 反時計90° B10 反時計180° B11 反時計270°  
      if (md->conf.rotate == 1) {
        x = 7-d[1];  y = d[1];
      } else if (md->conf.rotate == 2) {
        x = 7 - d[0];  y = 7 - d[1];                
      } else if (md->conf.rotate == 3) {
        x = d[1]; y = 7-d[0];
      }      
      setDotAt(mx->pdata, 8, 8, x, y, d[2]);
      str++;    
      return str;
// 回転 *******************************************************************
    } else if (*str == 'r') { 
      str++;
      if (*str == '+') {
        value = md->conf.rotate < 3 ? md->conf.rotate+1 :0; str++;
      } else if (*str == '-') {
        value = md->conf.rotate > 0 ? md->conf.rotate-1 :3; str++;        
      } else {
        if ( !(str = str2Uint(&value, str)) )  return skipCtrCommand(str);
        if (value > 3) return skipCtrCommand(str);        
      }
      if( *str != '}') return skipCtrCommand(str); 
      md->conf.rotate = value;
      str++;
      return str;
/* メモリ不足のため機能を封印
//即時回転 *******************************************************************
    } else if (*str == '/') { 
      str++;
      if (*str == '+') {
        value = 1; str++;
      } else if (*str == '-') {
        value = 3; str++;
      } else {
        if ( !(str = str2Uint(&value, str)) )  return skipCtrCommand(str);
        if (value > 3) return skipCtrCommand(str);
      }
      if( *str != '}') return skipCtrCommand(str); 
      rotateBitmap(mx->pdata, DOTNUM, DOTNUM, value); 
      str++;
      return str; 
*/
// スクロール方向指定 **************************************************************
    } else if (*str == 's') { 
      str++;
      value = getDrctNo((md->conf.rotate));
      if (*str == '+') {
        value = value < 8 ? value+1 :0; str++;
      } else if (*str == '-') {
        value = value > 0 ? value-1 :8; str++;        
      } else {
        if ( !(str = str2Uint(&value, str)) )  return skipCtrCommand(str);
        if (value > 15) return skipCtrCommand(str);        
      }      
      if( *str != '}') return skipCtrCommand(str); 
      md->conf.scroll = pgm_read_byte(drct + value);   
      str++;
      return str;
// スクロール速度指定/即時ディレイ/文字送り間隔/メッセージ間隔  ********************
    } else if (*str == 't' || *str == 'd' || *str == 'c'|| *str == 'w') { 
      c = *str;
      str++; if ( !(str = str2Uint(&value, str)) )  return skipCtrCommand(str);
      if (value > MAXDELAYTM) return skipCtrCommand(str);
      if( *str != '}') return skipCtrCommand(str);
      str++;
      if (c == 't') {
        md->conf.speed = value;
      } else if (c == 'c') {
        md->conf.charwait = value;
        //Serial.println(conf.charwait,DEC);
      } else if (c == 'w') {
        md->conf.wait = value;
      } else {
        delay(value);      
      }
      return str;      
// IchigoJamフォントを利用(asciiコード系)/デフォルトフォント(utf-8系)を利用/1ドットスクロール/初期化 *****
    } else if (*str == 'i' || *str == 'n' || *str == '>' || *str=='!') { 
      c = *str;      
      str++;
      if( *str != '}') return skipCtrCommand(str);
      str++;
      if (c == '>') {
        memset(font,0,EM_FONT_SIZE);
        drawBitmap(font, 0x80);
      } else if (c == '!') {
        md->loadConfig();
      } else {
        md->conf.codetype = (c == 'i')?1:0;
      }
      return str;            
// 文字列変数埋め込み/メッセージ埋め込み *****************************************
    } else if (*str == 'v' || *str == 'm') {
      c = *str;
      str++; if ( !(str = str2Uint(&value, str)) )  return skipCtrCommand(str);
      if (c == 'v') {
        if (value >= EM_MAX_VNUM) return skipCtrCommand(str);
      } else {
        if (value >= EM_MAX_ALLMSGNUM) return skipCtrCommand(str);
      }
      if( *str != '}') return skipCtrCommand(str);
      str++;      

      if (c == 'v') {
        pUTF8 = md->getValue(value); playLEDMatrix(pUTF8) ;
      } else {
        char m[EM_MAX_MSGLEN];
        md->loadMessage(m, value);
        playLEDMatrix(m);
      }
      return str;
// 表示のクリア *****************************************************************
    } else if (*str == '}') {
      str++; mx->clear_buf();
    } else {
      str = skipCtrCommand(str);
    }
  } 
  return str;
}

// 指定ビットマップデータの再生
void command::drawBitmap(uint8_t* pf, uint8_t flg) {
  mx->actionDraw(pf, md->conf.rotate, md->conf.scroll|flg, md->conf.speed, md->conf.charwait);   
}

//
// LEDマトリックスへの1文字フォントの出力
// 引数 pUTF8 表示文字列
// 戻り値 次の文字列位置
//
char* command::DrawLEDMatrix(char* pUTF8) {
  uint8_t font[EM_FONT_SIZE];
  char* p = pUTF8;
#if CONF_USE_ICHIGOFONT==1
  if (!md->conf.codetype) {
    if (!(pUTF8 = getFontData(font,pUTF8 , true))) {
      // 該当フォントが無い場合はIchigoJamフォントを利用する
      pUTF8 = p;
      getIchigoFont(font, (uint8_t)*pUTF8);
      pUTF8++;            
    } 
  } else {
    getIchigoFont(font, (uint8_t)*pUTF8);
    pUTF8++;
  }
#else
  if (!(pUTF8 = getFontData(font,pUTF8 , true))) {
    // 該当フォントが無い場合
    pUTF8 = p;
    uint16_t c;
    uint8_t n = charUFT8toUTF16(&c, pUTF8);
    if (!n) {
      // 変換不能文字
      pUTF8++;
    } else {
       pUTF8+=n;
    }
    getFontDataByUTF16(font, 0x25A1); // フォントに豆腐を指定
  }
#endif
  drawBitmap(font);
  return pUTF8;

}

//
// 文字列メッセージ出力(即時表示用)
//
void command::playLEDMatrix(char* pUTF8) {
  static uint8_t cnt=0; // 間接ネスト防止
  char* t;
  if (cnt == 2) {
    //Serial.print("@ERR deep nested:");Serial.println(pUTF8);
    return;
  }
  cnt++;
  while(*pUTF8) {
    if(*pUTF8 == '\"') {
      pUTF8++;
      continue;
    }
    t = pUTF8;
    pUTF8 =doCtrlCommand(pUTF8); // 制御コード処理
    if (t != pUTF8)  continue;
    pUTF8 = DrawLEDMatrix(pUTF8);
  }  
  cnt--;
}

// ロゴの表示(7文字分表示(
void command::playLogo() {
  uint8_t font[EM_FONT_SIZE];
  for (uint8_t i=0; i < 8; i++) {
    md->loadLogoMessage(font, i);
    drawBitmap(font);
  }
}

