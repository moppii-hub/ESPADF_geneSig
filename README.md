# Summary / 概要

This project is an example using ESP-ADF(Audio Development Framework) audio element to output sounds through external I2S-DAC.  
To explain audio elements usage, it generate a simple sine wave in an audio element instance, output to I2S writer through audio pipeline.  
If you want to do some processing to raw signal datas using ESP-ADF, this might be for reference.  

---

このプログラムはESP-ADF(Audio Development Framework)のaudio element機能を使って外部I2S-DACから音声出力をするサンプルプログラムです。  
audio elementの使い方を説明するため、１つのaudio elementの中で単純な正弦波を生成し、audio pipelineを通してI2S_writerへ信号出力しています。  
もしESP-ADFを使って音声信号の生データを扱いたい場合には、参考になるかと思い作成しました。  

---

这个程序是一个样本，就是使用audio element功能通过外接I2S-DAC输出声音的。  
为了说明audio element功能的用法，在一个audio element功能中产生一个单纯的正弦波, 把这个波形通过audio pipeline输出到I2S-writer。  
如果使用ESP-ADF想把生的声音数据做一些处理的话，我觉得这个有可能作为参考。  


# Required / 必要環境
## Hardware / ハード
* ESP32 board (ESP32-DevKitC, ESP-WROVER-KIT, etc.) or ESP-ADF boards
* MAX98357A I2S amplifer (If you use ESP-ADF boards, it already has ES8388 codec-chip so no need.)
* Speaker unit


## Sofware / ソフト
* [ESP-IDF v3.3](https://docs.espressif.com/projects/esp-idf/en/v3.3.1/index.html) ([Github](https://github.com/espressif/esp-idf))
* [ESP-ADF](https://docs.espressif.com/projects/esp-adf/en/latest/) ([Github](https://github.com/espressif/esp-adf))


# Pin Assgin / ピンアサイン
| pin name (MAX98357A board print) | function | gpio_num |
|:---:|:---:|:---:|
| WS (LRC) | word select (L/R Clock) | GPIO_NUM_25 |
| SCK (BCLK) | continuous serial clock (Block CLocK) | GPIO_NUM_5 |
| SD (DIN) | serial data (Data IN) | GPIO_NUM_26 |

![Wiring](https://github.com/moppii-hub/ESPADF_geneSig/blob/master/ESP32_I2S_example_modified_wiring.png)


# Usage / 使用方法
## Prepare for dev-env / 環境構築

See [Get Started page](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html) for install ESP-ADF.  
If you don't have installed ESP-IDF v3.3.1, see [ESP-IDF v3.3.1 Get Started page](https://docs.espressif.com/projects/esp-idf/en/v3.3.1/get-started/index.html) to get it.  
  
Example commands(for mac):  
```bash
# First, need to download https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz to ~/Downloads
mkdir -p ~/esp
cd ~/esp
tar -xzf ~/Downloads/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz
git clone -b v3.3.1 --recursive https://github.com/espressif/esp-idf.git
git clone --recursive https://github.com/espressif/esp-adf.git


# First, need to set up some Env-variables.
python -m pip install --user -r $IDF_PATH/requirements.txt
```


## Download projects(git clone) / プロジェクトのダウンロード(git clone)
```bash
cd ~/esp
git clone https://github.com/moppii-hub/ESPADF_geneSig.git
```


## Flash program / プログラムの書き込み
```bash
make menuconfig
# For flashing, it is need to setup correct USB-serial port in menuconfig.

make flash
```

