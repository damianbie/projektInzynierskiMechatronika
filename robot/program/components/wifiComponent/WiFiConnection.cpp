#include "WiFiConnection.hpp"
#include <stdio.h>

WiFiConnection::WiFiConnection()
{
    TAG = "WIFI_CONNECTION";
    s_wifi_event_group = nullptr;
    _isConnected = false;
    instance_wifi_disconnected = 0;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}
WiFiConnection& WiFiConnection::instance()
{
    static WiFiConnection _instance;
    return _instance;
}
void WiFiConnection::connect(const char* ssid, const char* pass)
{
    s_wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        WiFiConnection::event_handler,
                                                        this,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        WiFiConnection::event_handler,
                                                        this,
                                                        &instance_got_ip));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    strcpy((char*)(&wifi_config.sta.ssid), ssid);
    strcpy((char*)(&wifi_config.sta.password), pass);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_set_ps(wifi_ps_type_t::WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGV(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));

    vEventGroupDelete(s_wifi_event_group);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, WiFiConnection::wifi_disconect_event, this, &(this->instance_wifi_disconnected));
}
void WiFiConnection::event_handler(void* arg, esp_event_base_t event_base,
                int32_t event_id, void* event_data)
{
    WiFiConnection* _this = (WiFiConnection*) arg;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupSetBits(_this->s_wifi_event_group, WIFI_FAIL_BIT);
        wifi_event_sta_disconnected_t *reason = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGE(_this->TAG,"connect to the AP fail err code %s", reason->ssid);
        esp_wifi_set_mode(wifi_mode_t::WIFI_MODE_NULL);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        _this->_isConnected = true;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(_this->TAG, "Successfully connected with ip " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(_this->s_wifi_event_group, WIFI_CONNECTED_BIT);
    }   
}
void WiFiConnection::wifi_disconect_event(void* arg, esp_event_base_t event_base, 
                                            int32_t event_id, void* event_data)
{
    WiFiConnection* _this = (WiFiConnection*) arg;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *reason = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGE(_this->TAG,"disconnected with AP, reason: %s", reason->ssid);
        _this->_isConnected = false;
    }
}
bool WiFiConnection::isConnected()
{
    return this->_isConnected;
}