# はじめに

　先日、[ESP-IDF(ESP32) v4 で外部I2S DACを使って正弦波を鳴らす](https://qiita.com/moppii/items/e109324d21429f12e2bd)という投稿をしました。この記事では、[ESP-IDF(IoT Development Framework)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)を使ってESP32に外部接続したI2S-DACから正弦波を出力しました。しかし、より実用的に使う（例えばPCM音源を再生したり、合成等の信号処理をしたりする）ためには、要素機能毎にタスクを分割し、マルチタスクで同時実行したり、各タスク間で信号を受け渡したりする必要があります。  

　通常、こういった機能は自前で実装するものかもしれませんが、ESP32の開発元であるEspressif社の提供している[ESP-ADF(Audio Development Framework)](https://docs.espressif.com/projects/esp-adf/en/latest/)というライブラリに、まさにこれらの機能が実装されていることが分かりましたので、これを活用することにしました。このESP-ADFは専用ボード（[ESP32-LyraT](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/get-started-esp32-lyrat.html)など）での使用を想定しているようですが、単体のESP32でも使用可能ですので、本投稿では前回同様にESP32と外部I2S-DACとスピーカを使用して、簡単な正弦波を出力してみます。  

　[公式サンプル](https://github.com/espressif/esp-adf/tree/master/examples)との違いは、出力する信号波形を自分で(sin関数を使って)計算している点です。公式のサンプルにはmp3やwavファイルを読み取って再生するものがありますが、信号波形を直接触ることはほぼしておらず、ドキュメントにもあまりその方法の説明がありません。そこで今回は、信号波形の生データを自分で生成してI2S出力に渡す処理を、分かりやすく説明できるようなプログラムを作成しました。

　今回の投稿では、まず動作させるまでを説明します。プログラムやライブラリ仕様の説明は、後日投稿します。



# 準備するもの
- PC(筆者はMac)
- ESP32開発ボード(NodeMCU, ESP-WROOM-32等)
- MAX98357A I2S DACモジュール(Adafruit製や類似品)
- スピーカユニット（使用するDACに適したもの。0.5～3W程度の小さいもの）



# 環境構築
## ESP-ADFの導入
　公式サイト（[Get Started](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html)）の説明通りにESP-IDFとESP-ADFを導入します。なお[先日の投稿](https://qiita.com/moppii/items/e109324d21429f12e2bd)とはESP-IDFのバージョンが異なるため、注意が必要です（前回はv4でしたが、今回はv3.3.1を使います）。私はesp_v3とesp_v4の2つ似フォルダを分けて両立させています（環境変数も切り替えています）が、よく分からなければ既存のespフォルダを削除または退避して、改めて再導入すればOKです。  

　新規導入する場合の手順は、以下のようになります。（macの場合）  

```bash
# 事前に https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz を~/Downloadsへダウンロードしておく。
mkdir -p ~/esp
cd ~/esp
tar -xzf ~/Downloads/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz
git clone -b v3.3.1 --recursive https://github.com/espressif/esp-idf.git
git clone --recursive https://github.com/espressif/esp-adf.git

# pipでパッケージをインストールする。（pipが無ければ別途インストール必要）
# また、環境変数$IDF_PATHを設定する必要あり。
python -m pip install --user -r $IDF_PATH/requirements.txt
```



## Githubからプログラムを入手（git clonoe）
　今回もgithubに一式を用意しましたので、git cloneでダウンロードするだけでOKです。  

```bash
cd ~/esp
git clone https://github.com/moppii-hub/ESPADF_geneSig.git
```



## 回路の製作
　[先日の投稿](https://qiita.com/moppii/items/e109324d21429f12e2bd)の回路と同じです。再掲します。  

![Wiring](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/603551/20ed0e84-7233-4fa5-71ab-d0998ecd68af.png)



# とりあえず動作させる
　まず、`make menuconfig`を実行し、シリアルポートの設定をします。  

```bash
cd ~/esp/ESPADF_geneSig/
make menuconfig
```

　設定方法は[公式の説明](https://docs.espressif.com/projects/esp-idf/en/v3.3.1/get-started/index.html#configure)の通りですが、menuconfigの画面が出たら`Serial flasher config` > `Default serial port` と選び、ポート名を入力し、save, exitします。なおポート名は、以下の方法で調べることができます。私の環境(mac)では、`/dev/tty.SLAB_USBtoUART`です。  

```bash
ls /dev/tty.*
```  

　ポートの設定が完了したら、ESP32をPCへ接続し、プログラムを書き込みます。  

```bash
make flash
```

　プログラムの書き込みが終わり、スピーカから正弦波が鳴れば成功です。  


# さいごに
　簡単ですが、今回はここまでとします。次回は、今回動作させたプログラムの説明と、ESP-ADFライブラリの使用方法の説明をします。