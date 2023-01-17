# EtherCAT-Simulate-System
EtherCAT-Simulate-SystemはオープンソースのEtherCATスレーブスタック[SOES](https://github.com/OpenEtherCATsociety/SOES "SOES")を編集して，EtherCATのプロセスデータ通信を用いて，DOBOT Magicianのロボットアームとコンベアを制御するシステムである．

## Overview

### EtherCAT通信処理
EtherCAT-Simulate-SystemはRaspberry PiとEVB-LAN9252-SPIを利用して，EtherCATスレーブを構成している．
SOESは複数のシステムで動作可能であり，本模擬システムではSOESのRasberry PiとLAN9252を対応したアプリケーションを利用した．
Raspberry PiはSOESのraspberry_lan9252demoを利用して，SPI経由でEtherCAT Slave ControllerであるLAN9252と接続する．

### DOBOT Magician制御処理
DOBOTの公式サイトではDOBOT Magicianを制御するDOBOT APIを公開しているが，Raspberry Piに対応していない．
DOBOT製品の日本代理TechShare社がRaspberry Piの[ARMプロセッサ対応のDLL](https://techshare.co.jp/faq/dobot/magician/dobotmagician%E3%82%92raspberry-pi%E3%81%A7%E5%8B%95%E3%81%8B%E3%81%97%E3%81%9F%E3%81%84.html "dll")を提供している．
Raspberry Piはコンパイル済みのDLLを利用して，Raspberry PiとUSBで繋いだDOBOT Magicianのロボットアームを制御する．
また，このDLL一式はC++のライブラリなので，Cのライブラリに変換する必要がある．
[Dobot Magician C言語プログラミングガイド](https://www.physical-computing.jp/product/2136 "dobot_c")ではDOBOT MagicianをC言語で動かすサンプルプログラムとライブラリを提供している．
EtherCAT-Simulate-Systemは本で提供したライブラリを使用した．

### 各EtherCATスレーブ構成リスト
- SLAVE A
  - SLAVE AのESIファイルはslave_A.xmlである．
  - SLAVE AのPDO AssignmentやPDO Mappingなどのオブジェクト情報はSOES_A/applications/raspberry_lan9252demo/[slave_objectlist.c](https://github.com/akiyama-lab/EtherCAT-Simulate-System/blob/main/SOES_A/applications/raspberry_lan9252demo/slave_objectlist.c "a")にある．
  - ロボットアームAを制御するアプリケーションはSOES_A/applications/raspberry_lan9252demo/[main.c](https://github.com/akiyama-lab/EtherCAT-Simulate-System/blob/main/SOES_A/applications/raspberry_lan9252demo/main.c "a")にある．
- SLAVE B
  - SLAVE BのESIファイルはslave_B.xmlである．
  - SLAVE BのPDO AssignmentやPDO Mappingなどのオブジェクト情報はSOES_B/applications/raspberry_lan9252demo/[slave_objectlist.c](https://github.com/akiyama-lab/EtherCAT-Simulate-System/blob/main/SOES_B/applications/raspberry_lan9252demo/slave_objectlist.c "b")にある．
  - ロボットアームBとコンベアを制御するアプリケーションはSOES_B/applications/raspberry_lan9252demo/[main.c](https://github.com/akiyama-lab/EtherCAT-Simulate-System/blob/main/SOES_B/applications/raspberry_lan9252demo/main.c "b")にある．
- SLAVE C
  - SLAVE CのESIファイルはslave_C.xmlである．
  - 色識別pythonスクリプトはcolorDetect.pyである．
  - SLAVE CのPDO AssignmentやPDO Mappingなどのオブジェクト情報はSOES_C/applications/raspberry_lan9252demo/[slave_objectlist.c](https://github.com/akiyama-lab/EtherCAT-Simulate-System/blob/main/SOES_C/applications/raspberry_lan9252demo/slave_objectlist.c "c")にある．
  - 識別した結果を処理するアプリケーションはアプリケーションはSOES_C/applications/raspberry_lan9252demo/[main.c](https://github.com/akiyama-lab/EtherCAT-Simulate-System/blob/main/SOES_C/applications/raspberry_lan9252demo/main.c "c")にある．

### 使用方法
#### 環境準備
ハードウェア：Raspberry Pi 3台；EVB-LAN9252-SPI 3台
ソフトウェア：BCM2835ライブラリ；Dobot Magician C言語ライブラリ

##### SOESのRaspberry Piのアプリケーションをビルドするための設定
SOES_X/cmake/[Linux.cmake](https://github.com/akiyama-lab/EtherCAT-Simulate-System/blob/main/SOES_A/cmake/Linux.cmake)ファイルは以下のように設定する．
ここのSOES_XはSLAVE AまたはSLAVE BまたはSLAVE Cである．
```
set (SOES_DEMO applications/raspberry_lan9252demo)
set(HAL_SOURCES
${SOES_SOURCE_DIR}/soes/hal/raspberrypi-lan9252/esc_hw.c
${SOES_SOURCE_DIR}/soes/hal/raspberrypi-lan9252/esc_hw.h
)
```

##### DOBOT APIをSOESに入れてビルドするための設定
ダウンロードしたDobot Magician C言語ライブラリをSOES_X/applications/raspberry_lan9252demoディレクトリに置いて，SOES_X/applications/raspberry_lan9252demo/[CMakeLists.txt](https://github.com/akiyama-lab/EtherCAT-Simulate-System/blob/main/SOES_A/applications/raspberry_lan9252demo/CMakeLists.txt "cmake")ファイルを以下のように設定する．
ここのSOES_XはSLAVE AまたはSLAVE Bである．SLAVE Cはロボットアームを制御しないため，設定する必要がない．

```
target_link_libraries(soes-demo LINK_PUBLIC soes bcm2835 pthread LINK_PRIVATE /home/raspi_ctl_02/SOES/applications/raspberry_lan9252demo/libDobotDll.so)
```

#### 手順
以下の設定はRaspberry Piで行う
##### ビルド
```
~ $ cd EtherCAT-Simulate-System/SOES_X
~/EtherCAT-Simulate-System/SOES_X $ mkdir build
~/EtherCAT-Simulate-System/SOES_X/build $ cmake ..
~/EtherCAT-Simulate-System/SOES_X/build $ make
```

##### SOESの実行
```
~/EtherCAT-Simulate-System/SOES_X/build $ cd applications/raspberry_lan9252demo/
~/EtherCAT-Simulate-System/SOES_X/build/applications/raspberry_lan9252demo/ $ sudo ./soes-demo
```
ここのSOES_XはSLAVE AまたはSLAVE BまたはSLAVE Cである．
