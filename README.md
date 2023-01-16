# EtherCAT-Simulate-System
EtherCAT-Simulate-SystemはオープンソースのEtherCATスレーブスタック[SOES](https://github.com/OpenEtherCATsociety/SOES "SOES")を編集して，EtherCATのプロセスデータ通信を用いて，DOBOT Magicianのロボットアームとコンベアを制御するシステムである．

### 構成リスト
<dl>
  <dt>SLAVE A</dt>
    <dd>ロボットアームAを制御するアプリケーションはSOES_A/applications/raspberry_lan9252demo/main.cにある．</dd>
  <dt>SLAVE B</dt>
    <dd>ロボットアームBとコンベアを制御するアプリケーションはSOES_B/applications/raspberry_lan9252demo/main.cにある．</dd>
  <dt>SLAVE C</dt>
    <dd>色識別pythonスクリプトはSOES_C/build/applications/raspberry_lan9252demo/colorDetect.pyである．</dd>
    <dd>識別した結果を処理するアプリケーションはアプリケーションはSOES_C/applications/raspberry_lan9252demo/main.cにある．</dd>
</dl>
