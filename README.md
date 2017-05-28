Raspberry Pi ターゲット依存部 for TOPPERS/ASP3
==============================================

概要
----

Raspberry Pi ターゲット依存部は，  
ARM をベースとした Raspberry Pi Zero/1/2/3 システムをサポートしている．  

カーネルの使用リソース
----------------------

カーネルは以下のリソースを使用する．  

* RAM  
256MByte RAM にコードとデータを配置する．  
 
* System Timer  
高精度タイマに用いる．  
     
* mini UART  
コンソールの入出力に使用．  

* 割り込みコントローラ  
割り込み優先度実装に使用．  
割り込みレベルはソフトウェアで実装．

実行環境
--------

動作確認を行った実行環境は，  
Ubuntu 16.04 LTS 64bit である．

開発環境
--------

Eclipse+OpenOCD+JTAG，arm-non-eabi-gdb  
を用いて開発・デバッグを行った．

コンパイラ
----------

動作確認した GCC は，  
4.9.3 20150529 (prerelease) (15:4.9.3+svn231177-1)  
である．

ビルドと実行
------------

TOPPERS/ASP3カーネル 簡易版  
https://www.toppers.jp/asp3-e-download.html

「Core Tile for ARM11 MPCore（ARM）簡易パッケージ」を取得し、  
（例：asp3_ct11mpcore_gcc-20160515.tar.gz）  
本ソース差分を上書きする。

user.txtに従ってビルド環境を作成する．  
<ターゲット略称> は，rp1_gcc または rp2_gcc または rp3_gcc となる．

FAT32でフォーマットしたSDカードに，  
bootcode.bin, start.elf (Raspbianのfirmwareより取得) と，  
ビルドにて生成された kernel.img の３つのファイルを入れる．

Raspbian firmware  
https://github.com/raspberrypi/firmware/tree/master/boot

SDカードをRaspberry Piの本体に挿入して,電源を入れる．

シリアル接続は，Raspberry PiのGPIOピンの，  
TXD・RXD・GNDのUART線をＰＣに接続する．  
（6pin:GND・8pin:TXD・10pin:RXD）  
115200 8N1である．

Sample1のシステムログの出力
---------------------------

通常のままビルド・実行しても何も出力されないので、  
ビルド前にSample1を変更して、ログ出力を有効にすると良い。

* syssvc/banner.c

[変更前]

    void
    print_banner_copyright(intptr_t exinf)
    {
        syslog_3(LOG_NOTICE, banner,  

[変更後]

    void
    print_banner_copyright(intptr_t exinf)
    {
        syslog_msk_log(LOG_UPTO(LOG_DEBUG), LOG_UPTO(LOG_DEBUG));  
        syslog_3(LOG_NOTICE, banner,  

* sample/sample1.c

[変更前] main_task()内

    HRTCNT	hrtcnt1, hrtcnt2;

    SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
    syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

[変更後]

    HRTCNT	hrtcnt1, hrtcnt2;

    //SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
    syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

