# はじめに（前回のおさらい）
　[前回](https://qiita.com/moppii/items/6fea3ca907f3f9631efc)の続きです。今回はプログラムの中身を説明します。
プログラム本体は[ESPADF_geneSig/main/geneSig_main.c](https://github.com/moppii-hub/ESPADF_geneSig/blob/master/main/geneSig_main.c)です。


# プログラムの説明
## 全体の流れ
1. audio pipelineを作成する
2. 2つのaudio elementを作成する(信号計算する要素と、I2S出力をする要素の2つ)
3. 2つのaudio elementをaudio pipelineに登録し、接続する。
4. event listenerを準備する
5. audio pipelineを起動する
6. 無限ループ(audio elementから停止・エラー等のメッセージを受け取ったら、無限ループを抜ける)
7. ループを抜けたら、pipelineを停止し、各変数を解放する。


## audio pipelineとaudio elementについて
　プログラムの説明をする前に、まずESP-ADFにおける重要な概念であるaudio pipelineとaudio elementについて簡単に紹介しておきます。  

　audio pipelineとは、パイプラインという名前の通り、一つの管、線のようなものです。audio elementはこのパイプラインにの中にある要素で、audio pipelineは複数のaudio elementを接続する形で構成されます。[公式説明](https://docs.espressif.com/projects/esp-adf/en/latest/api-reference/index.html)の図を貼っておきます。  

![audio pipelineの例](https://docs.espressif.com/projects/esp-adf/en/latest/_images/blockdiag-5c07de543c2e374ca051b1a185278c255ba43554.png)  

　この例では、MP3ファイルを開く機能(Read MP3 file)、MP3ファイルから音声データを取り出す機能(MP3 decoder)、音声データをI2S出力する機能(I2S stream)の3つのaudio elementを接続して、1つのaudio pipelineを構成しています（※一番右のCodec chipというのはプログラム上では存在しないものなので、今は無視してください）。  

　今回のプログラムでは、2つのaudio elementを作成しています。1つは信号波形を生成するelementです。もう1つは、上の図と同じようにI2S出力をするelement(I2S stream)です。これを、左から順に接続し、1つめのelementが生成した信号をI2S出力するelementに受け渡す動作になっています。  

　さて、ここからはプログラムを見ながら説明します。


## pipelineの作成
　まず、37~40行目のaudio pipelineの作成です。  

```C
    //Create pipeline
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);
```

　`audio_pipeline_cfg_t`というのは、`audio_pipeline_cfg`という構造体のポインタ型(typedefされたもの)です。ESP-ADFではこういった専用の型が大量に出てきますが、中身を理解する必要はありませんので、読み飛ばしてしまいましょう。
　この3行の意味は要するに、
- 標準設定(config)を準備して、
- audio pipelineを初期化(init)して、
- 初期化の結果を確認(mem_assert)している、
というだけです。これ以上の理解は今回は不要です。



## audio elementの作成
　次に、42~45行目は、I2S出力用のaudio elementを作成しています。  

```C
    //Create audio-element for I2S output
    i2s_stream_cfg_t i2s_cfg_write = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg_write.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg_write);
```

　これも、先ほどのaudio pipelineの作成と似ています。この3行の意味は、
- 標準設定(config)を準備する
- 設定の一項目(i2s_cfg_write.type)を微調整する
- audio elementを初期化(init)する
というだけです。今回はなぜか初期化の結果の確認はしていませんが、気にする必要はありません。  

　ところで、このaudio elementは、いろんな種類のものがあります。上のi2s_streamというのはその1つで、これはi2sの入出力をする専用のaudio elementです（入力、つまり録音もできるのですが、今回は出力するため、`i2s_cfg_write.type = AUDIO_STREAM_WRITER;`としています）。この他にも、各種オーディオファイルの読み取り機能(mp3_decoder, wav_decoder等)やファイルの読み取り機能(fatfs_stream_reader, http_stream_reader等)のような専用elementがあります。  

　しかし、自分で信号を生成して次のelementに受け渡すような専用elementは残念ながらありません。そこで、汎用の(基本的な|素っ裸の)audio elementを使って、自分で実装する必要があります。それが48~76行目です。  

```C
    //---- Create audio-element for generate signal(sine wave) ----

    //prepare config structure
    audio_element_cfg_t fg_cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    fg_cfg.open = _geneSig_open;
    fg_cfg.close = _geneSig_close;
    fg_cfg.process = _geneSig_process;
    fg_cfg.destroy = _geneSig_destroy;
    fg_cfg.read = _geneSig_read;
    fg_cfg.write = NULL;                //if NULL, execute "el->write_type = IO_TYPE_RB;"
    fg_cfg.task_stack = (3072+512);
    fg_cfg.task_prio = 5;
    fg_cfg.task_core = 0;
    fg_cfg.out_rb_size = (8*1024);
    fg_cfg.multi_out_rb_num = 0;
    fg_cfg.tag = "geneSig";
    fg_cfg.buffer_len = 2048;

    //create audio-element
    generator = audio_element_init(&fg_cfg);

    //prepare audio_element_info and set to audio-element
    audio_element_info_t info = {0};
    info.sample_rates = 44100;
    info.channels = 2;
    info.bits = 16;
    audio_element_setinfo(generator, &info);

    // --------
```

　これも、簡単に言えば、以下の処理をしているだけです。
- fg_cfgという設定を準備する
- audio_element_initでelementを初期化する
- 初期化のあと、一部の設定を補足設定する(audio_element_setinfo)
　初期化のあとに補足設定をしているのは、恐らくこの順序でなければ設定ができないからで、特に大きな意味はありません（i2s_stream_init関数の中身と同じ順序になっています）。  

　ここで、1つ重要な点があります。それは、`fg_cfg`の要素`open`, `close`, `process`, `destroy`, `read`, `write`の部分です。これらは関数ポインタ型の変数で、それぞれ名前の通りの処理をする関数を指定する変数になっています。write以外はすべて`_geneSig_***`という関数名を代入していますが、これらは135行目以降の関数を指しています。今回は、これらのうち、read(_geneSig_read)という関数で信号を生成して、process(_geneSig_process)という関数で信号を次のaudio element（つまりi2s stream）へ渡します。信号生成については後述します。  



## pipelineの実行
　次に、78～91行目では、pipelineの準備・動作開始をしています。

```C
    //Register audio-elements to pipeline
    audio_pipeline_register(pipeline, generator, "geneSig");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s_write");

    //Link audio-elements (generator-->i2s_stream_writer)
    audio_pipeline_link(pipeline, (const char *[]) {"geneSig", "i2s_write"}, 2);

    //Set up event listener
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);
    audio_pipeline_set_listener(pipeline, evt);

    //Run pipeline
    audio_pipeline_run(pipeline);
```

　これは、つまり以下の処理をしています。
- pipelineにelementを登録し、
- audio pipelineの中でelement同士を接続し、
- event listenerというのを設定して、
- audio pipelineを起動している。
　これらは、audio pipelineを使う時の定型文です。最後のaudio_pipeline_run()関数を実行すると、各audio elementが1つのタスクを生成します。ここでいうタスクとは、FreeRTOSのタスクで、つまりxTaskCreatePinnedToCore関数が実行されます。FreeRTOSの説明については割愛しますが、ESP-IDFはFreeRTOSベースで設計されており、マルチタスクやメモリ管理など、一般的なFreeRTOSと同等の機能を有しているようです。とにかく、各audio elementが動作開始(readやwrite等の関数実行)し、element間で信号を受け渡しし始めます。



## event処理
　93~114行目は、無限ループになっています。  




## 後処理（メモリ解放）


## 信号の生成（callback関数内の処理）
　

# ADFの仕様の説明
## audio element

## audio pipeline

## event interface

## 他
　ESP-ADFでは、この各audio elementがそれぞれ独立したプロセス（タスク）として動作します。各audio elementは3つのデータバッファ（入力受け取り用のバッファ、途中計算用のバッファ、出力受け渡し用のバッファ）を持っており、audio pipelineへ登録・接続すると、自動的に前後のaudio elementの入力受け取り用バッファと出力受け渡し用バッファをリンク(同じメモリ番地を指すように)します。そして、各audio elementは7つの関数(コールバック関数)を持っており、audio pipelineを実行すると、最後尾のaudio elementからそのコールバック関数が実行され、一つずつ手前のaudio elementに対し出力を要求（後ろのaudio elementから見ると入力処理）をします。



# さいごに

