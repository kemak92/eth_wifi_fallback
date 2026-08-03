/*************************************************************************/
// This ESPHome component wraps around the repo by @kemak92:
// https://github.com/kemak92/eth_wifi_fallback
//
// Ethernet primary + WiFi Client fallback (same or different static IP).
// by @kemak92 - heungelectric, 2026
/*************************************************************************/

#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/ethernet/ethernet_component.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

namespace esphome {
namespace eth_wifi_fallback {

enum class FallbackState {
  IDLE,            // Đang dùng Ethernet
  STARTING_WIFI,   // Đang khởi tạo / kết nối WiFi
  WIFI_ACTIVE,     // Đang dùng WiFi fallback
  STOPPING_WIFI,   // Đang tắt WiFi
};

class EthWifiFallback : public Component {
 public:
  void set_ssid(const std::string &ssid) { this->ssid_ = ssid; }
  void set_password(const std::string &password) { this->password_ = password; }
  void set_check_interval(uint32_t interval_ms) { this->check_interval_ = interval_ms; }

  void set_manual_ip(uint32_t static_ip, uint32_t gateway, uint32_t subnet, uint32_t dns1 = 0) {
    this->use_manual_ip_ = true;
    this->static_ip_ = static_ip;
    this->gateway_ = gateway;
    this->subnet_ = subnet;
    this->dns1_ = dns1;
  }

  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

 protected:
  std::string ssid_;
  std::string password_;
  uint32_t check_interval_{15000};
  uint32_t last_check_{0};
  uint32_t wifi_start_time_{0};

  FallbackState state_{FallbackState::IDLE};
  bool wifi_initialized_{false};
  esp_netif_t *wifi_netif_{nullptr};

  bool use_manual_ip_{false};
  uint32_t static_ip_{0};
  uint32_t gateway_{0};
  uint32_t subnet_{0};
  uint32_t dns1_{0};

  void start_wifi_();
  void stop_wifi_();
  bool is_wifi_connected_();
  void log_wifi_ip_();
};

}  // namespace eth_wifi_fallback
}  // namespace esphome