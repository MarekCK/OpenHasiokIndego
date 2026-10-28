#include "wifi_ap.h"

#include <string.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "motor.h"
#include "command.h"

#define OHI_WIFI_SSID      "OHI_AP"
#define OHI_WIFI_PASS      "exl#123456"
#define OHI_WIFI_CHANNEL   1
#define OHI_MAX_STA_CONN   2

#if CONFIG_ESP_GTK_REKEYING_ENABLE
#define OHI_GTK_REKEY_INTERVAL CONFIG_ESP_GTK_REKEY_INTERVAL
#else
#define OHI_GTK_REKEY_INTERVAL 0
#endif

static const char *TAG = "OHI_WIFI";

static int8_t blade_cmd = CMD_BLADE_OFF;
static int8_t prev_sw = 1;

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t sw;
} rc_packet_t;

volatile rc_packet_t rc_data = {0};

static void espnow_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len == sizeof(rc_packet_t)) 
        memcpy((void*)&rc_data, data, sizeof(rc_packet_t));

    if ((rc_data.sw == 0) && (prev_sw == 1)) {
        blade_cmd = (blade_cmd == CMD_BLADE_ON) ? CMD_BLADE_OFF : CMD_BLADE_ON;

        ohi_cmd = blade_cmd;
    }

    prev_sw = rc_data.sw;

    int left_speed = rc_data.x + rc_data.y;
    int right_speed = rc_data.x - rc_data.y;
    int8_t left_direction = (left_speed >= 0) ? MOTOR_FORWARD : MOTOR_REVERSE;
    int8_t right_direction = (right_speed >= 0) ? MOTOR_FORWARD : MOTOR_REVERSE;
    left_speed = abs(left_speed);
    right_speed = abs(right_speed);
    motor_set(LEFT_MOTOR, left_direction, left_speed);
    motor_set(RIGHT_MOTOR, right_direction, right_speed);

    // printf("\rL: %d R: %d ", left_speed, right_speed);
}


static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d reason=%d", MAC2STR(event->mac), event->aid, event->reason);
    }
}

void ohi_wifi_init(void) {

    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = OHI_WIFI_SSID,
            .ssid_len = strlen(OHI_WIFI_SSID),
            .channel = OHI_WIFI_CHANNEL,
            .password = OHI_WIFI_PASS,
            .max_connection = OHI_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = true,
            },
            .gtk_rekey_interval = OHI_GTK_REKEY_INTERVAL,
        },
    };

    if (strlen(OHI_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

    ESP_LOGI(TAG, "AP started. SSID:%s CH:%d", OHI_WIFI_SSID, OHI_WIFI_CHANNEL);
}