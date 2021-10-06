# 複雑系科学実験用モジュール

## 必要機器
- raspberry pi (raspberry pi 3 model bでの動作は検証済み)
- microSD(最低8GB)
- LCDディスプレイ()
- mcp3208(a/d converter)
- タクトスイッチ x3
- ロータリーエンコーダ
- 抵抗(10kΩ) x2
- コネクタ

## OS
[Raspberry Pi OS with desktop](
https://downloads.raspberrypi.org/raspios_armhf/images/raspios_armhf-2021-05-28/2021-05-07-raspios-buster-armhf.zip)

[公式ページ](https://www.raspberrypi.org/software/operating-systems/)

## GPIO
今回使用しているGPIOはADC用に22-25番, 電源ボタン用に3番、ロータリーエンコーダ用に22,10番、残りのボタン用に9番, 11番を使用している  
うまいこと繋いでつかってくれ

## セットアップ手順
1. OSのインストール

    使用するOSは[raspbian](##OS)を想定している  
    [BalenaEtcher](https://www.balena.io/etcher/)を使い[公式ページ](https://www.raspberrypi.org/software/operating-systems/)からダウンロードしたOSをmicroSDにかきこむ
    (このとき書き込んだosのboot直下にsshという名前でファイルを作ると、raspberry piのssh機能が有効になるため便利、ちなみにraspberry piのssh初期ip: raspberrypi.local, port: 22, id: pi, pw: raspberry)

2. スリープの無効

    起動させたraspberry piに入りスリープを無効にする
    /etc/xdg/lxsession/LXDE/autostartに
    
    ```
    @xset s off
    @xset s noblank
    @xset -dpms
    ```

    /etc/lightdm/lightdm.confに
    ```
    [SeatDefaults]
    xserver-command=X -s 0 -dpms
    ```
    を追記する
    その後再起動をすることで画面のスリープがなくなる
    
    [こちらのサイト](https://rikoubou.hatenablog.com/entry/2020/06/11/133716)を参考にするとよいだろう
    
3. プログラムの入手

    githubよりプログラムを入手してもらう  
    これはどこかのサイトを調べて自分でやってくれ
    レポジトリは[こちら](https://github.com/3846504/sakulab_ex_module)
    ディレクトリは/optの下に入れると後が楽だろう

4. 解像度の変更

    解像度を変更するためにはboot直下のconfig.txtを少し変更すればよい  
    そのままの解像度だと少し見づらいため変更するとよいと思われる
    必要でないならば飛ばしても構わない  
    初期状態のconfig.txtから[こちら](./boot/config.txt)に書き換えるとよいだろう  
    420:380の解像度に変更される

5. プログラムの展開

    ダウンロードしたプログラムの一通りをraspberry piの中の`/opt`ディレクトリに配置する
    その後serviceファイルを設定することでその後プログラムが電源投入時に自動起動するようになる  
    [詳しくはこちら](https://qiita.com/G-san/items/b0f9a340601cdb4a068f)  
    serviceファイルを配置しなければ自動起動することはない
    一部ファイルの権限を変更しておく必要がある
    ```
    sudo chmod 755 /src/start_program.sh
    sudo chmod 755 /bin/chua_mm
    sudo chmod 755 /bin/pulse_meter
    sudo chmod 755 /bin/tp_mm
    ```
    を実行しておいてね

6. 必要ライブラリの入手

    今回メニューの表示にfbiを使っている  
    初期状態では入っていないため自分で入れる必要がある  
    まず
    ```
    sudo apt-get update
    ```
    でapt-getをアップデートしその後
    ```
    sudo apt-get install fbi
    ```
    でfbiを入手できる

以上で動くようになるはずなので頑張って


動かなかったらしらん