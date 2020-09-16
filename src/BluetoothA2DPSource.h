// C++ Class Implementation for a A2DP Source to be used as Arduino Library
// The original ESP32 implementation can be found at https://github.com/espressif/esp-idf/tree/master/examples/bluetooth/bluedroid/classic_bt/a2dp_source
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Copyright 2020 Phil Schatzmann
// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD

#ifndef __A2DP_SOURCE_H__
#define __A2DP_SOURCE_H__

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/xtensa_api.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#include "esp32-hal-bt.h"
#endif


typedef void (* bt_app_cb_t) (uint16_t event, void *param);
typedef  int32_t (* music_data_cb_t) (uint8_t *data, int32_t len);

/* message to be sent */
typedef struct {
    uint16_t             sig;      /*!< signal to bt_app_task */
    uint16_t             event;    /*!< message event id */
    bt_app_cb_t          cb;       /*!< context switch callback */
    void                 *param;   /*!< parameter area needs to be last */
} bt_app_msg_t;

typedef void (* bt_app_copy_cb_t) (bt_app_msg_t *msg, void *p_dest, void *p_src);

/**
 *  Utility structure that can be used to split a int32_t up into 2 separate channels with int16_t data.
 */
struct Channels {
  int16_t channel1;
  int16_t channel2;
};

class BluetoothA2DPSource {
  public: 
    /**
     * Constructor
     */
    BluetoothA2DPSource();
    /**
     * name: Bluetooth name of the device to connect to
     * callback: function that provides the audio stream -
     * The supported audio codec in ESP32 A2DP is SBC. SBC audio stream is encoded
     * from PCM data normally formatted as 44.1kHz sampling rate, two-channel 16-bit sample data
     * is_ssp_enabled: Flag to activate Secure Simple Pairing 
     */ 

    void start(char* name, music_data_cb_t callback, bool is_ssp_enabled = false);

    void setPinCode(char* pin_code, esp_bt_pin_type_t pin_type=ESP_BT_PIN_TYPE_VARIABLE);


    /**
     *  The following mthods are called by the framework. They are public so that they can
     *  be executed from a extern "C" function.
     */
     // handler for bluetooth stack enabled events
    void bt_av_hdl_stack_evt(uint16_t event, void *p_param);
    void bt_app_task_handler(void *arg);
    void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
    /// callback function for AVRCP controller
    void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);
    void a2d_app_heart_beat(void *arg);
    /// callback function for A2DP source
    void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);
    /// callback function for A2DP source audio data stream
    int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len);
    /// A2DP application state machine
    void bt_app_av_sm_hdlr(uint16_t event, void *param);
    /// avrc CT event handler
    void bt_av_hdl_avrc_ct_evt(uint16_t event, void *p_param);
    // callback for data
    music_data_cb_t data_stream_callback;


  private:
    bool ssp_enabled;
    char* bt_name;
    esp_bt_pin_type_t pin_type;
    esp_bt_pin_code_t pin_code;
    uint32_t pin_code_len;

    esp_bd_addr_t s_peer_bda;
    uint8_t s_peer_bdname[ESP_BT_GAP_MAX_BDNAME_LEN + 1];
    int s_a2d_state;
    int s_media_state;
    int s_intv_cnt;
    int s_connecting_intv;
    uint32_t s_pkt_cnt;
    //static esp_avrc_rn_evt_cap_mask_t s_avrc_peer_rn_cap;
    TimerHandle_t s_tmr;
    xQueueHandle s_bt_app_task_queue;
    xTaskHandle s_bt_app_task_handle;

    bool bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback);
    void bt_app_task_start_up(void);
    void bt_app_task_shut_down(void);
    void bt_app_av_media_proc(uint16_t event, void *param);

    /* A2DP application state machine handler for each state */
    void bt_app_av_state_unconnected(uint16_t event, void *param);
    void bt_app_av_state_connecting(uint16_t event, void *param);
    void bt_app_av_state_connected(uint16_t event, void *param);
    void bt_app_av_state_disconnecting(uint16_t event, void *param);
     //void bt_av_notify_evt_handler(uint8_t event, esp_avrc_rn_param_t *param);

    // void bt_av_volume_changed(void);

    bool bt_app_send_msg(bt_app_msg_t *msg);
    void bt_app_work_dispatched(bt_app_msg_t *msg);

    char *bda2str(esp_bd_addr_t bda, char *str, size_t size);
    bool get_name_from_eir(uint8_t *eir, uint8_t *bdname, uint8_t *bdname_len);
    void filter_inquiry_scan_result(esp_bt_gap_cb_param_t *param);


};

#endif