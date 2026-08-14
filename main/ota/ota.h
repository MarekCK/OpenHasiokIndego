#pragma once
#include "esp_http_server.h"

static const char *TAG = "OHI_WEB";
esp_err_t ota_get_handler(httpd_req_t *req);
esp_err_t ota_post_handler(httpd_req_t *req);