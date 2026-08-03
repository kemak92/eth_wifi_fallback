/*************************************************************************/
// This ESPHome component wraps around the repo by @kemak92:
// https://github.com/kemak92/eth_wifi_fallback
//
// Ethernet primary + WiFi Client fallback (same or different static IP).
// by @kemak92 - heungelectric, 2026
/*************************************************************************/

#include "eth_wifi_fallback.h"
#include "esphome/core/log.h"
#include "esphome/components/ethernet/ethernet_component.h"

namespace esphome {
namespace eth_wifi_fallback {

static const char *const TAG = "eth_wifi_fallback";

void EthWifiFallback::setup() {
  ESP_LOGI(TAG, "EthWifiFallback ready (check every %u ms)", this->check_interval_);
}

void EthWifiFallback::loop() {
  const uint32_t now = millis();
  if (now - this->last_check_ < this->check_interval_) {
    return;
  }
  this->last_check_ = now;

  bool eth_connected = false;
  if (ethernet::global_eth_component != nullptr) {
    eth_connected = ethernet::global_eth_component->is_connected();
  }

  switch (this->state_) {
    case FallbackState::IDLE:
      if (!eth_connected) {
        ESP_LOGW(TAG, "Ethernet lost → starting WiFi Client");
        this->state_ = FallbackState::STARTING_WIFI;
        this->wifi_start_time_ = now;
        this->start_wifi_();
      }
      break;

    case FallbackState::STARTING_WIFI:
      if (this->is_wifi_connected_()) {
        ESP_LOGI(TAG, "WiFi connected successfully");
        this->log_wifi_ip_();
        this->state_ = FallbackState::WIFI_ACTIVE;
      } else if (now - this->wifi_start_time_ > 20000) {
        ESP_LOGW(TAG, "WiFi connect timeout, retrying...");
        esp_wifi_connect();
        this->wifi_start_time_ = now;
      }
      break;

    case FallbackState::WIFI_ACTIVE:
      if (eth_connected) {
        ESP_LOGI(TAG, "Ethernet recovered → stopping WiFi");
        this->state_ = FallbackState::STOPPING_WIFI;
        this->stop_wifi_();
      } else if (!this->is_wifi_connected_()) {
        ESP_LOGW(TAG, "WiFi disconnected, reconnecting...");
        esp_wifi_connect();
      }
      break;

    case FallbackState::STOPPING_WIFI:
      this->state_ = FallbackState::IDLE;
      break;
  }
}

bool EthWifiFallback::is_wifi_connected_() {
  wifi_ap_record_t ap_info;
  return (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
}

void EthWifiFallback::log_wifi_ip_() {
  if (this->wifi_netif_ == nullptr)
    return;

  esp_netif_ip_info_t ip_info;
  if (esp_netif_get_ip_info(this->wifi_netif_, &ip_info) == ESP_OK) {
    ESP_LOGI(TAG, "WiFi IP: " IPSTR, IP2STR(&ip_info.ip));
    ESP_LOGI(TAG, "Gateway: " IPSTR, IP2STR(&ip_info.gw));
    ESP_LOGI(TAG, "Netmask: " IPSTR, IP2STR(&ip_info.netmask));
  } else {
    ESP_LOGW(TAG, "Could not get WiFi IP info");
  }
}

void EthWifiFallback::start_wifi_() {
  if (!this->wifi_initialized_) {
    esp_netif_init();

    // Tạo default STA netif (quan trọng để có IP)
    this->wifi_netif_ = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
      ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
      this->state_ = FallbackState::IDLE;
      return;
    }
    this->wifi_initialized_ = true;
  }

  // Cấu hình IP
  if (this->wifi_netif_ != nullptr) {
    if (this->use_manual_ip_) {
      esp_netif_dhcpc_stop(this->wifi_netif_);

      esp_netif_ip_info_t ip_info = {};
      ip_info.ip.addr = htonl(this->static_ip_);
      ip_info.gw.addr = htonl(this->gateway_);
      ip_info.netmask.addr = htonl(this->subnet_);
      esp_netif_set_ip_info(this->wifi_netif_, &ip_info);

      if (this->dns1_ != 0) {
        esp_netif_dns_info_t dns = {};
        dns.ip.type = ESP_IPADDR_TYPE_V4;
        dns.ip.u_addr.ip4.addr = htonl(this->dns1_);
        esp_netif_set_dns_info(this->wifi_netif_, ESP_NETIF_DNS_MAIN, &dns);
      }
      ESP_LOGI(TAG, "WiFi using static IP");
    } else {
      esp_netif_dhcpc_start(this->wifi_netif_);
      ESP_LOGI(TAG, "WiFi using DHCP");
    }
  }

  esp_wifi_set_mode(WIFI_MODE_STA);

  wifi_config_t wifi_config = {};
  strncpy((char *) wifi_config.sta.ssid, this->ssid_.c_str(), sizeof(wifi_config.sta.ssid) - 1);
  strncpy((char *) wifi_config.sta.password, this->password_.c_str(), sizeof(wifi_config.sta.password) - 1);
  wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();
  esp_wifi_connect();

  ESP_LOGI(TAG, "WiFi start requested (SSID: %s)", this->ssid_.c_str());
}

void EthWifiFallback::stop_wifi_() {
  esp_wifi_disconnect();
  esp_wifi_stop();
  // Giữ wifi_initialized_ = true để lần sau start nhanh hơn
  ESP_LOGI(TAG, "WiFi stopped");
}

}  // namespace eth_wifi_fallback
}  // namespace esphome