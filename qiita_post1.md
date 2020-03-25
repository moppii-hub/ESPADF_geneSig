# はじめに

　[先日の投稿](https://qiita.com/moppii/items/e109324d21429f12e2bd)では、ESP-IDF(IoT Development Framework)を使ってESP32に外部接続したI2S-DACから正弦波を出力しました。しかし、より実用的に使う（例えばPCM音源を再生したり、合成等の信号処理をしたりする）ためには、要素機能毎にタスクを分割し、マルチタスクで同時実行したり、各タスク間で信号を受け渡したりする必要があります。  

　通常、こういった機能は自前で実装するものかもしれませんが、ESP32の開発元であるEspressif社の提供しているESP-ADF(Audio Development Framework)というライブラリが、まさに上述の機能を有していることが分かりましたので、これを活用することにしました。このESP-ADFは専用ボード（[ESP32-LyraT](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/get-started-esp32-lyrat.html)など）での使用を想定しているようですが、単体のESP32でも使用可能ですので、本投稿では前回同様にESP32と外部I2S-DACとスピーカを使用して、簡単な正弦波を出力してみます。

　今回の記事は、まず動作させるまでを説明します。プログラムやライブラリ仕様の説明は、後日投稿します。



# 準備するもの
- PC(筆者はMac)
- ESP32開発ボード(NodeMCU, ESP-WROOM-32等)
- MAX98357A I2S DACモジュール(Adafruit製や類似品)
- スピーカユニット（使用するDACに適したもの。0.5～3W程度の小さいもの）



# 環境構築
## ESP-ADFのセットアップ
　公式サイト（[Get Started](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html)）の指示通りにESP-IDFとESP-ADFを導入します。1点、[先日の投稿](https://qiita.com/moppii/items/e109324d21429f12e2bd)とはESP-IDFのバージョンが異なるため、注意が必要です（前回はv4でしたが、今回はv3.3.1を使います）。私はesp_v3とesp_v4の2つ似フォルダを分けて両立させています（環境変数も切り替えています）が、よく分からなければespフォルダを削除または退避して、一から再セットアップすればOKです。  
　一応、一からセットアップする際のコマンドは、以下のようになります。（macの場合）  

```bash
# 事前に https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz をDownloadsフォルダにダウンロードしておく。
mkdir -p ~/esp
cd ~/esp
tar -xzf ~/Downloads/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz
git clone -b v3.3.1 --recursive https://github.com/espressif/esp-idf.git
git clone --recursive https://github.com/espressif/esp-adf.git

# pipでパッケージをインストールする。（pipが無ければ別途インストールして下さい）
python -m pip install --user -r $IDF_PATH/requirements.txt
```



## Githubからプログラムを入手（git clonoe）



## 回路の製作



# とりあえず動作させる

# さいごに

