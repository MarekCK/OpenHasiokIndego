#include "webserver.h"

#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "OHI_WEB";



static esp_err_t root_get_handler(httpd_req_t *req)
{
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

static esp_err_t ota_get_handler(httpd_req_t *req) {


    const char *resp =
        "<html>"
        "<head>"
        "<title>OpenHasiokIndego</title>"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "</head>"

        "<body style=\"text-align:center;font-family:Arial;\">"

        "<h1>OpenHasiokIndego</h1>"
        "<h2>OTA</h2>"

        // "\"/update\""
        "<a href=\"/update\" "
        "<br>"
        "<input type=\"file\" name=\"update\" "
        "style=\"font-size:20px;\">"

        "<br><br>"

        "<button type=\"submit\" "
        "style=\"font-size:14px;"
        "padding:10px;"
        "width:100px;"
        "background:#2E8B57;"
        "color:white;"
        "border:none;"
        "border-radius:10px;\">"
        "UPDATE"
        "</button>"

        "</form>"
"</a>"
        "</body>"
        "</html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t ota_post_handler(httpd_req_t *req)
{
    const char *resp =
        "<html>"
        "<head><title>OpenHasiokIndego</title></head>"
        "<body>"
        "<h1>OpenHasiokIndego</h1>"
        "<h2>POST</h2>"
        "<span style=\"font-size: medium\">"
        "<p>POST request received</p>"
        "</span>"
        "</body>"
        "</html>";

    ESP_LOGI(TAG, "OTA POST RECEIVED");
    httpd_resp_sendstr(req, "post works");
    // httpd_resp_send(req, "post works");

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
        ESP_LOGE(TAG, "HTTP server start failed");
    }
}
