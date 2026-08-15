#include "webserver.h"

#include "esp_http_server.h"
#include "esp_log.h"

#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include <sys/param.h>
#include "ota.h"
#include "command.h"

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

    "<button "
    // "onclick=\"fetch('/up')\" "
    "ontouchstart=\"fetch('/up')\" "
    "ontouchend=\"fetch('/stop')\" "
    "ontouchcancel=\"fetch('/stop')\" "
    "onmousedown=\"fetch('/up')\" "
    "onmouseup=\"fetch('/stop')\" "
    "onmouseleave=\"fetch('/stop')\" "    
    "style=\"position:fixed;"
    "top:100px;"
    "left:160px;"
    "font-size:14px;"
    "padding:30px;"
    "background:#4CAF50;"
    "color:white;"
    "border:none;"
    "border-radius:10px;\">"
    "^"
    "</button>"

    "<button "
    // "onclick=\"fetch('/down')\" "
    "ontouchstart=\"fetch('/down')\" "
    "ontouchend=\"fetch('/stop')\" "
    "ontouchcancel=\"fetch('/stop')\" "
    "onmousedown=\"fetch('/down')\" "
    "onmouseup=\"fetch('/stop')\" "
    "onmouseleave=\"fetch('/stop')\" "    
    "style=\"position:fixed;"
    "top:320px;"
    "left:160px;"
    "font-size:14px;"
    "padding:30px;"
    "background:#4CAF50;"
    "color:white;"
    "border:none;"
    "border-radius:10px;\">"
    "v"
    "</button>"

    "<button "
    // "onclick=\"fetch('/left')\" "
    "ontouchstart=\"fetch('/left')\" "
    "ontouchend=\"fetch('/stop')\" "
    "ontouchcancel=\"fetch('/stop')\" "
    "onmousedown=\"fetch('/left')\" "
    "onmouseup=\"fetch('/stop')\" "
    "onmouseleave=\"fetch('/stop')\" "    
    "style=\"position:fixed;"
    "top:210px;"
    "left:50px;"
    "font-size:14px;"
    "padding:30px;"
    "background:#4CAF50;"
    "color:white;"
    "border:none;"
    "border-radius:10px;\">"
    "<"
    "</button>"

    "<button "
    // "onclick=\"fetch('/right')\" "
    "ontouchstart=\"fetch('/right')\" "
    "ontouchend=\"fetch('/stop')\" "
    "ontouchcancel=\"fetch('/stop')\" "
    "onmousedown=\"fetch('/right')\" "
    "onmouseup=\"fetch('/stop')\" "
    "onmouseleave=\"fetch('/stop')\" "
    "style=\"position:fixed;"
    "top:210px;"
    "left:270px;"
    "font-size:14px;"
    "padding:30px;"
    "background:#4CAF50;"
    "color:white;"
    "border:none;"
    "border-radius:10px;\">"
    ">"
    "</button>"

    // "<button "
    // "onclick=\"fetch('/stop')\" "
    // "style=\"position:fixed;"
    // "top:210px;"
    // "left:160px;"
    // "font-size:14px;"
    // "padding:30px;"
    // "background:#4CAF50;"
    // "color:white;"
    // "border:none;"
    // "border-radius:10px;\">"
    // "||"
    // "</button>"

    "</body>"
    "</html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t up_get_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "UP");
    ohi_cmd = CMD_FORWARD;
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

static esp_err_t down_get_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "DOWN");
    ohi_cmd = CMD_BACKWARD;
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

static esp_err_t left_get_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "LEFT");
    ohi_cmd = CMD_LEFT;
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

static esp_err_t right_get_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "RIGHT");
    ohi_cmd = CMD_RIGHT;
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

static esp_err_t stop_get_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "STOP");
    ohi_cmd = CMD_STOP;
    httpd_resp_sendstr(req, "OK");
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

httpd_uri_t up = {
    .uri = "/up",
    .method = HTTP_GET,
    .handler = up_get_handler,
};

httpd_uri_t down = {
    .uri = "/down",
    .method = HTTP_GET,
    .handler = down_get_handler,
};

httpd_uri_t left = {
    .uri = "/left",
    .method = HTTP_GET,
    .handler = left_get_handler,
};

httpd_uri_t right = {
    .uri = "/right",
    .method = HTTP_GET,
    .handler = right_get_handler,
};

httpd_uri_t stop = {
    .uri = "/stop",
    .method = HTTP_GET,
    .handler = stop_get_handler,
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
        httpd_register_uri_handler(server, &up);
httpd_register_uri_handler(server, &down);
httpd_register_uri_handler(server, &left);
httpd_register_uri_handler(server, &right);
httpd_register_uri_handler(server, &stop);
        ESP_LOGI(TAG, "HTTP server started");
    }
    else
    {
        ESP_LOGI(TAG, "HTTP server start failed");
    }
}
