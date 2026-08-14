
#include "ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include <sys/param.h>
#include"led.h"

esp_err_t ota_get_handler(httpd_req_t *req)
{

    const char *resp =
        "<html>"
        "<body>"

        "<input type='file' id='fw'>"
        "<button onclick='upload()'>UPDATE</button>"

        "<script>"
        "async function upload(){"
        "const file=document.getElementById('fw').files[0];"
        "await fetch('/update',{method:'POST',body:file})"
        ".then(() => {"
        "setTimeout(() => {"
        "window.location.href = \"/\";"
        "}, 5000);"
        "});"
        "alert('Upload done');"
        "}"
        "</script>"

        "</body>"
        "</html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}


esp_err_t ota_post_handler(httpd_req_t *req) {

    int8_t red = 10;
    esp_ota_handle_t ota_handle = 0;
    led_set(red, 10, 0);
    const esp_partition_t *update_partition =
            esp_ota_get_next_update_partition(NULL);

    ESP_LOGI(TAG,
             "Writing to partition subtype=%d",
             update_partition->subtype);
    esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "esp_ota_begin failed: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }

    char ota_buff[512];

    int remaining = req->content_len;

    while (remaining > 0)
    {
        int received = httpd_req_recv(req, ota_buff, MIN(remaining, sizeof(ota_buff)));
        if (received <= 0) { 
            esp_ota_end(ota_handle); ESP_LOGI(TAG, "Receive failed"); 
            return ESP_FAIL;
        }
        if (red > 0)
            red = 0;
        else
            red = 30;
        led_set(red, 10, 0);
        err = esp_ota_write(ota_handle, ota_buff, received);
        if (err != ESP_OK) {
            esp_ota_end(ota_handle);
            ESP_LOGI(TAG, "OTA write failed: %s", esp_err_to_name(err));
            return ESP_FAIL;
        }
        remaining -= received;
    }

    err = esp_ota_end(ota_handle);

    if (err != ESP_OK) {
        ESP_LOGI(TAG, "OTA end failed: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }
    
    err = esp_ota_set_boot_partition(update_partition);

    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Set boot partition failed: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }
    httpd_resp_sendstr(req, "OK");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
}    
