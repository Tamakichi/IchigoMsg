# IchigoMsg
![トップ画像](./img/top.jpg)  

## 概要  
シリアル通信経由で表示内容の制御が簡単に行えるLEDドットマトリックスモジュールです。  

## システム構成  
![システム構成](./img/system.jpg)

## ハードウェア  

### 回路図  
### 部品一覧  
### 接続・結線    


## シリアル通信仕様（条件）    
<通信条件>  
 - 通信速度 115200bps  
 - データ長 8ビット
 - パリティ なし  
 - ストップビット 1  

<通信プロトコル使用>  
- ホストからのコマンド送信  
  - 形式  
   **@コマンド名 引数 [,引数] 改行**  
   ※ 改行は **CR+LF** or **CR** or **LF** の形式が可能  

  - 応答  
   正常： **OK** 改行  
   異常： **NG** 改行  
    
  ※ 改行は **CR+LF** 
  応答とは別にデータを出力するコマンドあり。
 

## 利用可能なコマンド  
<メッセージの管理>    
- メッセージの登録  
  @setmsg  __メッセージ番号__, __メッセージ文__  
- メッセージの追記  
  @setmsga __メッセージ番号__, __メッセージ文__  
- メッセージの確認  
  @getmsg __[メッセージ番号]__  
- メッセージの削除  
  @delmsg __メッセージ番号__  

<直接メッセージ出力>  
- メッセージ出力  
  @print __メッセージ文__    
- メッセージ出力(短縮形)  
  @? __メッセージ文__   
- 表示の消去  
  @cls  

<自動再生の管理>	
- メッセージリストの設定  
  @setlist __メッセージ番号[メッセージ番号][メッセージ番号][メッセージ番号]...__ (最大8個まで)  
- メッセージリストの再生	 	
  @play __プレイモード (0:停止 1:順次再生(1回のみ) 2:順次再生(繰り返し) 3:ランダム再生(繰り返し)__ 
- メッセージリスト再生停止	
  @stop  

<フォントの管理>  
- フォントの登録  
  @setfont __フォント番号__, __フォント定義(16桁16進数 8バイト分)__  
- フォントの確認  
  @getfont __[フォント番号(0～23)]__  
- フォントデータの直接表示  
  @out __フォント定義(16桁16進数 8バイト分)__  

<変数の管理>
- 変数への値設定				
  @setvalue __変数番号(0～9)__, __設定値(文字列16バイト迄)__  
- 変数の値確認  		
  @getvalue __[変数番号(0～9)]__  

<動作設定>  
- スクロール動作の設定  
  @scrl __+|-|動作コード(0～15 デフォルトは5)__, __速度(0～32767msec デフォルトは 60)__  
- メッセージ間ウエイト設定  
  @mwait	__間隔(0～32767 msec デフォルトは0)__    
- 文字間ウェイト設定  
  @cwait	__間隔(0～32767 msec デフォルトは0)__    
- 文字の回転補正  
  @rotate	__+ |- | 向き(0～3 デフォルトは3)__  
- 文字コード系設定  
  @setcode __コード系(0:UTF-8 1:IchigoJam ascii デフォルトは1)__  
- コマンド実行結果出力設定  
  @setout	__モード(0 通常出力 1 IchigoJam用に先頭に'を付加 2:出力なし デフォルトは1)__    
- 起動時ロゴ表示設定  
  @logo __[モード(0 表示なし 1 表示あり デフォルトは1 指定なしで起動時ロゴ表示)]__  

<設定値の保存・読込>
- 設定値の保存				@save
- 設定値の読込				@load
- デフォルト値の読込			@default
- EEPROMの初期化				@clsrom


## <ライセンスにおける注意事項>  
[![CC BY](https://image.jimcdn.com/app/cms/image/transf/none/path/s21a6c180c821a02c/image/i6ce073b1f2ea2d26/version/1432132230/image.png)](https://creativecommons.org/licenses/by/4.0/deed.ja)  [CC BY](https://creativecommons.org/licenses/by/4.0/) [IchigoJam](http://ichigojam.net/) / [Tamakichi-San](https://github.com/Tamakichi)

本 プログラムのフォントデータの一部に IchigoJam 1.2.1 のフォントデータを使用しています。  
IchigoJam のフォントが [CC BY](https://creativecommons.org/licenses/by/4.0/) ライセンスを明示しています。（[CC BY](https://creativecommons.org/licenses/by/4.0/) [IchigoJam](http://ichigojam.net/))  
そのため、本プログラムもこのライセンスを継承し、[CC BY](https://creativecommons.org/licenses/by/4.0/)で公開いたします。  
営利目的も含めて自由にご利用いただけますが（書籍などでのご利用も構いません）、  
ライセンスを含めた著作表記は「[CC BY](https://creativecommons.org/licenses/by/4.0/) [IchigoJam](http://ichigojam.net/) / [Tamakichi-San](https://github.com/Tamakichi)」です。  
紙面など URL をリンクできない場合は、リンクを URL 表記にして下さい。  
「CC BY IchigoJam http://ichigojam.net/ Tamakichi-San https://github.com/Tamakichi」  
