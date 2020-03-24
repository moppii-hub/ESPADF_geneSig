readme now preparing...  
readme 製作中。。。


# Summary / 概要



# Required / 必要環境
## Hardware / ハード
* ESP32 board (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* MAX98357A I2S amplifer
* Speaker unit

## Sofware / ソフト
* ESP-IDF v3.3
* ESP-ADF


# Pin Assgin / ピンアサイン
| pin name (MAX98357A board print) | function | gpio_num |
|:---:|:---:|:---:|
| WS (LRC) | word select (L/R Clock) | GPIO_NUM_25 |
| SCK (BCLK) | continuous serial clock (Block CLocK) | GPIO_NUM_5 |
| SD (DIN) | serial data (Data IN) | GPIO_NUM_26 |

![Wiring](https://github.com/moppii-hub/ESPADF_geneSig/blob/master/ESP32_I2S_example_modified_wiring.png)


# Usage / 使用方法
## prepare for dev-env / 環境構築
see https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html for install ESP-ADF.  
If you don't have installed ESP-IDF v3.3.1, see https://docs.espressif.com/projects/esp-idf/en/v3.3.1/get-started/index.html to get it.  
  
example commands(for mac):  
```
# First, need to download https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz to ~/Downloads
mkdir -p ~/esp
cd ~/esp
tar -xzf ~/Downloads/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz
git clone -b v3.3.1 --recursive https://github.com/espressif/esp-idf.git
git clone --recursive https://github.com/espressif/esp-adf.git


# First, need to set up some Env-variables.
python -m pip install --user -r $IDF_PATH/requirements.txt
```


## download projects(git clone) / プロジェクトのダウンロード(git clone)
```
git clone https://github.com/moppii-hub/ESPADF_geneSig.git
```


## flash program / プログラムの書き込み
```
make menuconfig
make flash
```

# Reference / 参考
https://docs.espressif.com/projects/esp-adf/en/latest/index.html

