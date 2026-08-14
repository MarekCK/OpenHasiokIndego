#include "webserver.h"

#include "esp_http_server.h"
#include "esp_log.h"

#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include <sys/param.h>

static const char *TAG = "OHI_WEB";


static esp_err_t root_get_handler(httpd_req_t *req) {
const char *resp =
    "<html>"
    "<head>"
    "<title>OpenHasiokIndego</title>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "</head>"

    "<body style=\"text-align:center;font-family:Arial;\">"

    "<h1 style=\"font-size:18px;\">OpenHasiokIndego</h1>"
    "<h2 style=\"font-size:18px;\">Uratowany z hasioka ;-)</h2>"
    "<a href=\"/ota\" "
    "style=\"position:fixed;"
    "bottom:20px;"
    "right:20px;"
    "font-size:14px;"
    "padding:20px;"
    "background:#4CAF50;"
    "color:white;"
    "text-decoration:none;"
    "border-radius:10px;\">"
    " OTA "
    "</a>" 
    "</body>"
    "</html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t ota_get_handler(httpd_req_t *req)
{

    const char *resp =
        "<html>"
        "<body>"

        "<input type='file' id='fw'>"
        "<button onclick='upload()'>UPDATE</button>"

        "<script>"
        "async function upload(){"
        "const file=document.getElementById('fw').files[0];"
        "await fetch('/update',{method:'POST',body:file});"
        "alert('Upload done');"
        "}"
        "</script>"

        "</body>"
        "</html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t ota_post_handler(httpd_req_t *req) {

    esp_ota_handle_t ota_handle = 0;

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
    esp_restart();
    httpd_resp_sendstr(req, "OTA OK - rebooting");
    vTaskDelay(pdMS_TO_TICKS(1000));
    return ESP_OK;
}    


void ohi_webserver_start(void)
{
    httpd_handle_t server = NULL;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
        .user_ctx = NULL
    };

    httpd_uri_t ota_page = {
        .uri = "/ota",
        .method = HTTP_GET,
        .handler = ota_get_handler,
        .user_ctx = NULL
    };

    httpd_uri_t ota_upload = {
        .uri = "/update",
        .method = HTTP_POST,
        .handler = ota_post_handler,
        .user_ctx = NULL
    };

    config.lru_purge_enable = true;

    ESP_LOGI(TAG,
             "Starting HTTP server on port %d",
             config.server_port);

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &ota_page);
        httpd_register_uri_handler(server, &ota_upload);
        ESP_LOGI(TAG, "HTTP server started");
    }
    else
    {
        ESP_LOGI(TAG, "HTTP server start failed");
    }
}
