#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "audio_pipeline.h"
#include "i2s_stream.h"
#include "board.h"
#include <math.h>

#define WAVE_FREQ_HZ    (442.0)
#define MAXVOL          (5000)          //[0, 32767]
#define PI              (3.14159265)

static const char *TAG = "geneSig";
static int i_time_global = 0;

static esp_err_t _geneSig_open(audio_element_handle_t self);
static esp_err_t _geneSig_destroy(audio_element_handle_t self);
static esp_err_t _geneSig_close(audio_element_handle_t self);
static int _geneSig_read(audio_element_handle_t self, char *buffer, int len, TickType_t ticks_to_wait, void *context);
static int _geneSig_process(audio_element_handle_t self, char *in_buffer, int in_len);
int _geneSig_write(audio_element_handle_t el, char *buf, int buf_len, TickType_t ticks_to_wait, void *ctx);  //Not to use in this program.


void app_main(void){
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_writer, generator;

    //ESP-IDF Log setting(no need)
//    esp_log_level_set("*", ESP_LOG_INFO);
//    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    //Board init for ADF-board(no need for single ESP32 board)
//    audio_board_handle_t board_handle = audio_board_init();
//    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    //Create pipeline
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    //Create audio-element for I2S output
    i2s_stream_cfg_t i2s_cfg_write = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg_write.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg_write);


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

    //inf-loop (can listen event messages from audio-elements)
    while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if (msg.cmd == AEL_MSG_CMD_ERROR) {
            ESP_LOGE(TAG, "[ * ] Action command error: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
                     msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);
        }

        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
    }

    //---- If break inf-loop(error happened), run the following process ----

    //Terminate pipeline
    audio_pipeline_terminate(pipeline);

    //Unregister audio-elements
    audio_pipeline_unregister(pipeline, generator);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);

    //Remove listener
    audio_pipeline_remove_listener(pipeline);
    audio_event_iface_destroy(evt);

    // Release all resources (audio-elements, pipeline)
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(generator);
    audio_element_deinit(i2s_stream_writer);
}

static esp_err_t _geneSig_open(audio_element_handle_t self){
    audio_element_set_input_timeout(self, 10 / portTICK_RATE_MS);
    return ESP_OK;
}

static esp_err_t _geneSig_destroy(audio_element_handle_t self){
    return ESP_OK;
}

static esp_err_t _geneSig_close(audio_element_handle_t self){
    return ESP_OK;
}

static int _geneSig_read(audio_element_handle_t self, char *buffer, int len, TickType_t ticks_to_wait, void *context){
    //generate signals and put into (char *)buffer.
    float f_signal = 0.0, f_time;
    int i_fs, i_sample_per_wave;
    uint16_t i_signal = 0;
    uint16_t *sample;
    size_t bytes_read = 0;

    //get sample-rates from audio-element-info.
    audio_element_info_t info;
    audio_element_getinfo(self, &info);
    i_fs = info.sample_rates;
    i_sample_per_wave = i_fs / WAVE_FREQ_HZ;

    //alloc signal sample buffer.
    sample = calloc(len/2, sizeof(uint16_t));

    for(int i=0;i<len/2;i+=2){

        //Increment global time-step. (Unit:step, 1step = 1/44100 seconds)
        i_time_global++;

        //calc real-time. (Unit:seconds)
        f_time = (float)(i_time_global) / (float)(i_sample_per_wave);

        //calc signal amplitude
        f_signal = 1 + sin(2 * PI * f_time);        //float:[0, 2]
        i_signal = (uint16_t)(MAXVOL * f_signal);      //uint16_t:[0, 65535]

        //copy to sample-buffer(on memory).
        //Left channel(2bytes) and Right channel(2bytes)
        sample[i] = i_signal;
        sample[i+1] = i_signal;

        //For return value
        bytes_read += 4;

        //For prevent from overflow global-time steps, decrease time.
        if(i_time_global > i_sample_per_wave) i_time_global -= i_sample_per_wave;
    }

    //memory copy to buffer from sample
    memcpy(buffer, sample, len);

    //release sample buffer
    free(sample);

    return bytes_read;
}

static int _geneSig_process(audio_element_handle_t self, char *in_buffer, int in_len){
    int r_size = 0;
    int w_size = 0;

    //Run audio_element_input()
    //    This function will call call-back funtion or
    //    read buffers from ringbuffer(self->in.input_rb).
    //    This "geneSig" element should call call-back function(_geneSig_write).
    r_size = audio_element_input(self, in_buffer, in_len);

    if (r_size == AEL_IO_TIMEOUT) {
        //If process was timeout, copy zeros to buffer.
        memset(in_buffer, 0x00, in_len);
        r_size = in_len;
    }
    if ((r_size > 0)) {
        //Run audio_element_output()
        //    This function will call call-back funtion or
        //    write buffers to ringbuffer(self->out.output_rb).
        //    This "geneSig" element should write to ringbuffer automatically.
        //    (Detail is in ringbuf.c)
        w_size = audio_element_output(self, in_buffer, r_size);
    }else{
        w_size = r_size;
    }
    return w_size;
}

//Not to use in this program.
int _geneSig_write(audio_element_handle_t el, char *buf, int buf_len, TickType_t ticks_to_wait, void *ctx){
    return 0;
}

