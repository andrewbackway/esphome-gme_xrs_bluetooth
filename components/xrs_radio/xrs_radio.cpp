#include "xrs_radio.h"
#include "at_parser.h"

#include "sensor/xrs_sensor.h"
#include "binary_sensor/xrs_binary_sensor.h"
#include "text_sensor/xrs_text_sensor.h"
#include "number/xrs_number.h"
#include "switch/xrs_switch.h"
#include "select/xrs_select.h"

namespace esphome {
namespace xrs_radio {

static const char *const TAG = "xrs_radio";

XRSRadioComponent *XRSRadioComponent::instance_ = nullptr;

XRSRadioComponent::XRSRadioComponent() {}

void XRSRadioComponent::set_mac_address(const std::string &mac) { this->mac_address_ = mac; }

void XRSRadioComponent::set_location_sensors(sensor::Sensor *lat, sensor::Sensor *lon) {
  this->latitude_sensor_ = lat;
  this->longitude_sensor_ = lon;
}

void XRSRadioComponent::set_location_interval(uint32_t interval_ms) {
  this->location_interval_ms_ = interval_ms;
}

void XRSRadioComponent::register_numeric_sensor(XRSNumericSensorType type, XRSRadioSensor *s) {
  this->numeric_sensors_.push_back({type, s});
  switch (type) {
    case XRS_SENSOR_CHANNEL:
      s->publish_state(this->current_channel_);
      break;
    case XRS_SENSOR_ZONE:
      s->publish_state(this->current_zone_);
      break;
    case XRS_SENSOR_VOLUME:
      s->publish_state(this->current_volume_);
      break;
    case XRS_SENSOR_PTT_TIMER:
      s->publish_state(this->ptt_timer_);
      break;
  }
}

void XRSRadioComponent::register_binary_sensor(XRSBinarySensorType type, XRSRadioBinarySensor *s) {
  this->binary_sensors_.push_back({type, s});
  switch (type) {
    case XRS_BIN_CONNECTED:
      s->publish_state(this->connected_);
      break;
    case XRS_BIN_PTT_ACTIVE:
      s->publish_state(this->ptt_active_);
      break;
    case XRS_BIN_PTT_DATA:
      s->publish_state(this->ptt_data_);
      break;
    case XRS_BIN_POWER_LOW:
      s->publish_state(this->power_low_);
      break;
    case XRS_BIN_SCANNING:
      s->publish_state(this->scanning_);
      break;
    case XRS_BIN_DUPLEX_ENABLED:
      s->publish_state(this->duplex_enabled_);
      break;
    case XRS_BIN_SILENT_MEMORY:
      s->publish_state(this->silent_memory_);
      break;
    case XRS_BIN_QUIET_MEMORY:
      s->publish_state(this->quiet_memory_);
      break;
    case XRS_BIN_QUIET_MODE:
      s->publish_state(this->quiet_mode_);
      break;
  }
}

void XRSRadioComponent::register_text_sensor(XRSTextSensorType type, XRSRadioTextSensor *s) {
  this->text_sensors_.push_back({type, s});
  switch (type) {
    case XRS_TEXT_MANUFACTURER:
      s->publish_state(this->manufacturer_);
      break;
    case XRS_TEXT_MODEL:
      s->publish_state(this->model_);
      break;
    case XRS_TEXT_FIRMWARE:
      s->publish_state(this->firmware_);
      break;
    case XRS_TEXT_SERIAL:
      s->publish_state(this->serial_);
      break;
    case XRS_TEXT_LAST_MESSAGE:
      break;
    case XRS_TEXT_POWER_STATE: {
      std::string txt;
      switch (this->power_state_) {
        case 0:
          txt = "Booting";
          break;
        case 1:
          txt = "Running";
          break;
        case 2:
          txt = "Reset initiated";
          break;
        case 3:
          txt = "Power down initiated";
          break;
        case 4:
          txt = "Power down";
          break;
        case 5:
          txt = "Low battery";
          break;
        default:
          txt = "";
          break;
      }
      if (!txt.empty())
        s->publish_state(txt);
      break;
    }
    case XRS_TEXT_PTT_STATE: {
      std::string txt;
      if (!this->ptt_active_) {
        txt = "Idle";
      } else if (this->ptt_data_) {
        txt = "Transmitting voice+data";
      } else {
        txt = "Transmitting voice";
      }
      s->publish_state(txt);
      break;
    }
    case XRS_TEXT_CHANNEL_LABEL:
      this->publish_channel_label_();
      break;
  }
}

void XRSRadioComponent::register_number(XRSNumberType type, XRSRadioNumber *n) {
  this->numbers_.push_back({type, n});
  if (type == XRS_NUMBER_VOLUME)
    n->publish_state(this->current_volume_);
}

void XRSRadioComponent::register_switch(XRSSwitchType type, XRSRadioSwitch *sw) {
  this->switches_.push_back({type, sw});
  switch (type) {
    case XRS_SWITCH_LOCATION_MODE:
      sw->publish_state(this->location_mode_);
      break;
    case XRS_SWITCH_SCAN:
      sw->publish_state(this->scanning_);
      break;
    case XRS_SWITCH_DUPLEX:
      sw->publish_state(this->duplex_enabled_);
      break;
    case XRS_SWITCH_QUIET_MODE:
      sw->publish_state(this->quiet_mode_);
      break;
    case XRS_SWITCH_QUIET_MEMORY:
      sw->publish_state(this->quiet_memory_);
      break;
    case XRS_SWITCH_SILENT_MEMORY:
      sw->publish_state(this->silent_memory_);
      break;
  }
}

void XRSRadioComponent::register_select(XRSSelectType type, XRSRadioSelect *sel) {
  this->selects_.push_back({type, sel});
}

void XRSRadioComponent::set_volume(float volume) {
  int vol = static_cast<int>(volume + 0.5f);
  if (vol < 0)
    vol = 0;
  if (vol > 31)
    vol = 31;

  this->current_volume_ = vol;

  char buf[32];
  snprintf(buf, sizeof(buf), "AT+WGAV=%d", vol);
  this->send_command_(buf);

  for (auto &p : this->numeric_sensors_) {
    if (p.first == XRS_SENSOR_VOLUME)
      p.second->publish_state(this->current_volume_);
  }
  for (auto &p : this->numbers_) {
    if (p.first == XRS_NUMBER_VOLUME)
      p.second->publish_state(this->current_volume_);
  }
}

void XRSRadioComponent::set_location_mode(bool enabled) {
  this->location_mode_ = enabled;
  ESP_LOGI(TAG, "Location mode %s", enabled ? "enabled" : "disabled");
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_LOCATION_MODE)
      p.second->publish_state(this->location_mode_);
  }
}

void XRSRadioComponent::set_scan_enabled(bool enabled) {
  this->scanning_ = enabled;
  char buf[32];
  snprintf(buf, sizeof(buf), "AT+WGSCAN=%d", enabled ? 1 : 0);
  this->send_command_(buf);

  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_SCANNING)
      p.second->publish_state(this->scanning_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_SCAN)
      p.second->publish_state(this->scanning_);
  }
}

void XRSRadioComponent::set_duplex_enabled(bool enabled) {
  this->duplex_enabled_ = enabled;
  char buf[32];
  snprintf(buf, sizeof(buf), "AT+WGDUP=%d", enabled ? 1 : 0);
  this->send_command_(buf);

  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_DUPLEX_ENABLED)
      p.second->publish_state(this->duplex_enabled_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_DUPLEX)
      p.second->publish_state(this->duplex_enabled_);
  }
}

void XRSRadioComponent::set_quiet_mode(bool enabled) {
  this->quiet_mode_ = enabled;
  char buf[32];
  snprintf(buf, sizeof(buf), "AT+WGSSQ=%d", enabled ? 1 : 0);
  this->send_command_(buf);

  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_QUIET_MODE)
      p.second->publish_state(this->quiet_mode_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_QUIET_MODE)
      p.second->publish_state(this->quiet_mode_);
  }
}

void XRSRadioComponent::set_quiet_memory(bool enabled) {
  this->quiet_memory_ = enabled;
  char buf[32];
  snprintf(buf, sizeof(buf), "AT+WGSQM=%d", enabled ? 1 : 0);
  this->send_command_(buf);

  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_QUIET_MEMORY)
      p.second->publish_state(this->quiet_memory_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_QUIET_MEMORY)
      p.second->publish_state(this->quiet_memory_);
  }
}

void XRSRadioComponent::set_silent_memory(bool enabled) {
  this->silent_memory_ = enabled;
  char buf[32];
  snprintf(buf, sizeof(buf), "AT+WGCSM=%d", enabled ? 1 : 0);
  this->send_command_(buf);

  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_SILENT_MEMORY)
      p.second->publish_state(this->silent_memory_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_SILENT_MEMORY)
      p.second->publish_state(this->silent_memory_);
  }
}

void XRSRadioComponent::set_target_zone(uint8_t zone) {
  if (zone < 1 || zone > 8) {
    ESP_LOGW(TAG, "Ignoring invalid zone %u (must be 1..8)", zone);
    return;
  }
  if (!this->connected_) {
    ESP_LOGW(TAG, "Ignoring zone change while not connected");
    return;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "AT+WGZS=%u", zone);
  this->send_command_(buf);
}

void XRSRadioComponent::set_target_zone_channel(uint8_t zone, uint8_t channel) {
  if (zone < 1 || zone > 8 || channel < 1 || channel > 255) {
    ESP_LOGW(TAG, "Ignoring invalid zone/channel %u/%u", zone, channel);
    return;
  }
  if (!this->connected_) {
    ESP_LOGW(TAG, "Ignoring zone/channel change while not connected");
    return;
  }

  char buf[40];
  snprintf(buf, sizeof(buf), "AT+WGCHS=%u,%u", zone, channel);
  this->send_command_(buf);
}

void XRSRadioComponent::setup() {
  ESP_LOGI(TAG, "Setting up XRSRadioComponent");
  instance_ = this;
  this->init_bluetooth_();
}

void XRSRadioComponent::loop() {
  const uint32_t now = millis();

  if (this->bt_initialized_ && this->spp_ready_ && !this->connected_ && !this->connecting_ &&
      !this->mac_address_.empty()) {
    if (this->last_reconnect_attempt_ == 0 ||
        (now - this->last_reconnect_attempt_) > this->reconnect_delay_ms_) {
      this->last_reconnect_attempt_ = now;
      this->start_connection_();

      this->reconnect_delay_ms_ *= 2;
      if (this->reconnect_delay_ms_ > RECONNECT_DELAY_MAX_MS)
        this->reconnect_delay_ms_ = RECONNECT_DELAY_MAX_MS;
    }
  }

  if (this->connected_ && this->location_mode_ && this->latitude_sensor_ != nullptr &&
      this->longitude_sensor_ != nullptr) {
    if (this->latitude_sensor_->has_state() && this->longitude_sensor_->has_state()) {
      const float lat = this->latitude_sensor_->state;
      const float lon = this->longitude_sensor_->state;
      if (!std::isnan(lat) && !std::isnan(lon)) {
        if ((now - this->last_location_sent_) >= this->location_interval_ms_) {
          this->send_location_update_();
          this->last_location_sent_ = now;
        }
      }
    }
  }
}

void XRSRadioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "XRS Radio:");
  ESP_LOGCONFIG(TAG, "  MAC Address: %s", this->mac_address_.c_str());
  ESP_LOGCONFIG(TAG, "  BT initialized: %s", YESNO(this->bt_initialized_));
  ESP_LOGCONFIG(TAG, "  SPP ready: %s", YESNO(this->spp_ready_));
  ESP_LOGCONFIG(TAG, "  Connected: %s", YESNO(this->connected_));
  ESP_LOGCONFIG(TAG, "  Location mode: %s", YESNO(this->location_mode_));
  ESP_LOGCONFIG(TAG, "  Location interval: %u ms", this->location_interval_ms_);
}

void XRSRadioComponent::init_bluetooth_() {
  if (this->bt_initialized_)
    return;

  esp_err_t ret;

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_bt_controller_init failed: %d", ret);
    return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_bt_controller_enable failed: %d", ret);
    return;
  }

  ret = esp_bluedroid_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_bluedroid_init failed: %d", ret);
    return;
  }

  ret = esp_bluedroid_enable();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_bluedroid_enable failed: %d", ret);
    return;
  }

  ret = esp_spp_register_callback(&XRSRadioComponent::spp_callback_static);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_spp_register_callback failed: %d", ret);
    return;
  }

  ret = esp_spp_init(ESP_SPP_MODE_CB);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_spp_init failed: %d", ret);
    return;
  }

  this->bt_initialized_ = true;
  ESP_LOGI(TAG, "Bluetooth Classic and SPP initialized");
}

bool XRSRadioComponent::parse_mac_address_(esp_bd_addr_t out) {
  if (this->mac_address_.size() != 17) {
    ESP_LOGE(TAG, "MAC address must be 17 characters (AA:BB:CC:DD:EE:FF)");
    return false;
  }
  int values[6];
  if (sscanf(this->mac_address_.c_str(), "%x:%x:%x:%x:%x:%x",
             &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]) != 6) {
    ESP_LOGE(TAG, "Failed to parse MAC address '%s'", this->mac_address_.c_str());
    return false;
  }
  for (int i = 0; i < 6; i++) {
    out[i] = static_cast<uint8_t>(values[i]);
  }
  return true;
}

void XRSRadioComponent::start_connection_() {
  if (!this->spp_ready_) {
    ESP_LOGW(TAG, "SPP not ready, cannot connect yet");
    return;
  }
  if (this->mac_address_.empty()) {
    ESP_LOGE(TAG, "MAC address is not configured, cannot connect");
    return;
  }

  esp_bd_addr_t addr;
  if (!this->parse_mac_address_(addr))
    return;

  ESP_LOGI(TAG, "Connecting to XRS radio at %s", this->mac_address_.c_str());
  esp_err_t ret = esp_spp_connect(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_MASTER, 0, addr);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_spp_connect failed: %d", ret);
    return;
  }
  this->connecting_ = true;
}

void XRSRadioComponent::close_connection_() {
  if (this->connected_ && this->spp_handle_ != 0) {
    esp_spp_disconnect(this->spp_handle_);
  }
}

void XRSRadioComponent::send_command_(const std::string &cmd) {
  if (!this->connected_ || this->spp_handle_ == 0) {
    ESP_LOGW(TAG, "Cannot send command, not connected: '%s'", cmd.c_str());
    return;
  }
  std::string line = cmd;
  line += "\r\n";
  ESP_LOGD(TAG, "TX: %s", cmd.c_str());
  esp_err_t ret =
      esp_spp_write(this->spp_handle_, line.size(),
                    reinterpret_cast<const uint8_t *>(line.c_str()));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_spp_write failed: %d", ret);
  }
}

void XRSRadioComponent::send_handshake_commands_() {
  this->send_command_("ATE1");
  this->send_command_("ATV1");
  this->send_command_("AT+GMI?");
  this->send_command_("AT+GMM?");
  this->send_command_("AT+GMR?");
  this->send_command_("AT+GSN?");
  this->send_command_("AT+GOI?");
  this->request_channel_table();
}

void XRSRadioComponent::send_location_update_() {
  if (this->latitude_sensor_ == nullptr || this->longitude_sensor_ == nullptr)
    return;
  if (!this->latitude_sensor_->has_state() || !this->longitude_sensor_->has_state())
    return;

  const float lat = this->latitude_sensor_->state;
  const float lon = this->longitude_sensor_->state;
  if (std::isnan(lat) || std::isnan(lon))
    return;

  char buf[128];
  snprintf(buf, sizeof(buf), "AT+WGTLOC=000000,%.6f,%.6f", lat, lon);
  this->send_command_(buf);
}

void XRSRadioComponent::publish_connection_state_() {
  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_CONNECTED)
      p.second->publish_state(this->connected_);
  }
}

void XRSRadioComponent::publish_all_state_() {
  for (auto &p : this->numeric_sensors_) {
    switch (p.first) {
      case XRS_SENSOR_CHANNEL:
        p.second->publish_state(this->current_channel_);
        break;
      case XRS_SENSOR_ZONE:
        p.second->publish_state(this->current_zone_);
        break;
      case XRS_SENSOR_VOLUME:
        p.second->publish_state(this->current_volume_);
        break;
      case XRS_SENSOR_PTT_TIMER:
        p.second->publish_state(this->ptt_timer_);
        break;
    }
  }

  for (auto &p : this->numbers_) {
    if (p.first == XRS_NUMBER_VOLUME)
      p.second->publish_state(this->current_volume_);
  }

  for (auto &p : this->binary_sensors_) {
    switch (p.first) {
      case XRS_BIN_CONNECTED:
        p.second->publish_state(this->connected_);
        break;
      case XRS_BIN_PTT_ACTIVE:
        p.second->publish_state(this->ptt_active_);
        break;
      case XRS_BIN_PTT_DATA:
        p.second->publish_state(this->ptt_data_);
        break;
      case XRS_BIN_POWER_LOW:
        p.second->publish_state(this->power_low_);
        break;
      case XRS_BIN_SCANNING:
        p.second->publish_state(this->scanning_);
        break;
      case XRS_BIN_DUPLEX_ENABLED:
        p.second->publish_state(this->duplex_enabled_);
        break;
      case XRS_BIN_SILENT_MEMORY:
        p.second->publish_state(this->silent_memory_);
        break;
      case XRS_BIN_QUIET_MEMORY:
        p.second->publish_state(this->quiet_memory_);
        break;
      case XRS_BIN_QUIET_MODE:
        p.second->publish_state(this->quiet_mode_);
        break;
    }
  }

  for (auto &p : this->text_sensors_) {
    switch (p.first) {
      case XRS_TEXT_MANUFACTURER:
        p.second->publish_state(this->manufacturer_);
        break;
      case XRS_TEXT_MODEL:
        p.second->publish_state(this->model_);
        break;
      case XRS_TEXT_FIRMWARE:
        p.second->publish_state(this->firmware_);
        break;
      case XRS_TEXT_SERIAL:
        p.second->publish_state(this->serial_);
        break;
      case XRS_TEXT_LAST_MESSAGE:
        break;
      case XRS_TEXT_POWER_STATE: {
        std::string txt;
        switch (this->power_state_) {
          case 0:
            txt = "Booting";
            break;
          case 1:
            txt = "Running";
            break;
          case 2:
            txt = "Reset initiated";
            break;
          case 3:
            txt = "Power down initiated";
            break;
          case 4:
            txt = "Power down";
            break;
          case 5:
            txt = "Low battery";
            break;
        }
        if (!txt.empty())
          p.second->publish_state(txt);
        break;
      }
      case XRS_TEXT_PTT_STATE: {
        std::string txt;
        if (!this->ptt_active_) {
          txt = "Idle";
        } else if (this->ptt_data_) {
          txt = "Transmitting voice+data";
        } else {
          txt = "Transmitting voice";
        }
        p.second->publish_state(txt);
        break;
      }
      case XRS_TEXT_CHANNEL_LABEL:
        this->publish_channel_label_();
        break;
    }
  }
}

void XRSRadioComponent::handle_ptt_notification_(int state, int timer) {
  this->ptt_active_ = (state == 1 || state == 2);
  this->ptt_data_ = (state == 2);
  this->ptt_timer_ = (state == 2 && timer > 0) ? timer : 0;

  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_PTT_ACTIVE)
      p.second->publish_state(this->ptt_active_);
    else if (p.first == XRS_BIN_PTT_DATA)
      p.second->publish_state(this->ptt_data_);
  }
  for (auto &p : this->numeric_sensors_) {
    if (p.first == XRS_SENSOR_PTT_TIMER)
      p.second->publish_state(this->ptt_timer_);
  }
  for (auto &p : this->text_sensors_) {
    if (p.first == XRS_TEXT_PTT_STATE) {
      std::string txt;
      if (!this->ptt_active_) {
        txt = "Idle";
      } else if (this->ptt_data_) {
        txt = "Transmitting voice+data";
      } else {
        txt = "Transmitting voice";
      }
      p.second->publish_state(txt);
    }
  }
}

void XRSRadioComponent::handle_power_notification_(int state) {
  this->power_state_ = state;
  this->power_low_ = (state == 5);

  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_POWER_LOW)
      p.second->publish_state(this->power_low_);
  }
  for (auto &p : this->text_sensors_) {
    if (p.first == XRS_TEXT_POWER_STATE) {
      std::string txt;
      switch (state) {
        case 0:
          txt = "Booting";
          break;
        case 1:
          txt = "Running";
          break;
        case 2:
          txt = "Reset initiated";
          break;
        case 3:
          txt = "Power down initiated";
          break;
        case 4:
          txt = "Power down";
          break;
        case 5:
          txt = "Low battery";
          break;
      }
      if (!txt.empty())
        p.second->publish_state(txt);
    }
  }
}

void XRSRadioComponent::handle_scan_notification_(int enabled) {
  this->scanning_ = (enabled != 0);
  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_SCANNING)
      p.second->publish_state(this->scanning_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_SCAN)
      p.second->publish_state(this->scanning_);
  }
}

void XRSRadioComponent::handle_duplex_notification_(int enabled) {
  this->duplex_enabled_ = (enabled != 0);
  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_DUPLEX_ENABLED)
      p.second->publish_state(this->duplex_enabled_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_DUPLEX)
      p.second->publish_state(this->duplex_enabled_);
  }
}

void XRSRadioComponent::handle_silent_memory_notification_(int enabled) {
  this->silent_memory_ = (enabled != 0);
  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_SILENT_MEMORY)
      p.second->publish_state(this->silent_memory_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_SILENT_MEMORY)
      p.second->publish_state(this->silent_memory_);
  }
}

void XRSRadioComponent::handle_quiet_memory_notification_(int enabled) {
  this->quiet_memory_ = (enabled != 0);
  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_QUIET_MEMORY)
      p.second->publish_state(this->quiet_memory_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_QUIET_MEMORY)
      p.second->publish_state(this->quiet_memory_);
  }
}

void XRSRadioComponent::handle_quiet_mode_notification_(int enabled) {
  this->quiet_mode_ = (enabled != 0);
  for (auto &p : this->binary_sensors_) {
    if (p.first == XRS_BIN_QUIET_MODE)
      p.second->publish_state(this->quiet_mode_);
  }
  for (auto &p : this->switches_) {
    if (p.first == XRS_SWITCH_QUIET_MODE)
      p.second->publish_state(this->quiet_mode_);
  }
}

void XRSRadioComponent::request_channel_table() {
  if (!this->connected_) {
    ESP_LOGW(TAG, "Cannot request channel table, not connected");
    return;
  }
  ESP_LOGI(TAG, "Requesting channel/squelch table via AT_WGCHSQ");
  this->send_command_("AT_WGCHSQ");
}

std::string XRSRadioComponent::get_channel_label_(uint8_t zone, uint8_t channel) const {
  for (const auto &entry : this->channel_table_) {
    if (entry.zone == zone && entry.channel == channel)
      return entry.label;
  }
  return "";
}

void XRSRadioComponent::publish_channel_label_() {
  std::string label =
      this->get_channel_label_(static_cast<uint8_t>(this->current_zone_),
                               static_cast<uint8_t>(this->current_channel_));
  if (label.empty()) {
    label = string_sprintf("Z%u / Ch %u", this->current_zone_, this->current_channel_);
  }
  for (auto &p : this->text_sensors_) {
    if (p.first == XRS_TEXT_CHANNEL_LABEL)
      p.second->publish_state(label);
  }
}

void XRSRadioComponent::handle_channel_table_line_(const std::string &line) {
  std::string payload = ATParser::extract_payload(line, "+WGCHSQ:");
  if (payload.empty())
    return;

  auto parts = ATParser::split_args(payload);
  if (parts.size() < 2)
    return;

  int zone = atoi(parts[0].c_str());
  int ch = atoi(parts[1].c_str());

  float rx = 0.0f;
  float tx = 0.0f;
  if (parts.size() >= 4) {
    rx = static_cast<float>(atof(parts[2].c_str()));
    tx = static_cast<float>(atof(parts[3].c_str()));
  }

  std::string label;
  if (!parts.empty()) {
    label = parts.back();
    if (!label.empty() && label.front() == '\"')
      label.erase(0, 1);
    if (!label.empty() && label.back() == '\"')
      label.pop_back();
    while (!label.empty() && (label.front() == ' ' || label.front() == '\t'))
      label.erase(label.begin());
    while (!label.empty() && (label.back() == ' ' || label.back() == '\t'))
      label.pop_back();
  }

  ChannelInfo info;
  info.zone = static_cast<uint8_t>(zone);
  info.channel = static_cast<uint8_t>(ch);
  info.rx_freq = rx;
  info.tx_freq = tx;
  info.label = label;

  bool updated = false;
  for (auto &entry : this->channel_table_) {
    if (entry.zone == info.zone && entry.channel == info.channel) {
      entry = info;
      updated = true;
      break;
    }
  }
  if (!updated)
    this->channel_table_.push_back(info);

  if (info.zone == this->current_zone_ && info.channel == this->current_channel_) {
    this->publish_channel_label_();
  }

  for (auto &p : this->selects_) {
    p.second->refresh_options();
  }
}

void XRSRadioComponent::get_zone_options(std::vector<std::string> &out) const {
  out.clear();
  std::vector<uint8_t> zones;
  for (const auto &c : this->channel_table_) {
    if (std::find(zones.begin(), zones.end(), c.zone) == zones.end())
      zones.push_back(c.zone);
  }
  if (zones.empty()) {
    for (uint8_t z = 1; z <= 8; z++)
      zones.push_back(z);
  }
  std::sort(zones.begin(), zones.end());
  for (auto z : zones) {
    out.push_back(string_sprintf("Zone %u", static_cast<unsigned>(z)));
  }
}

void XRSRadioComponent::get_channel_options(std::vector<std::string> &out) const {
  out.clear();
  if (this->channel_table_.empty()) {
    for (uint8_t z = 1; z <= 8; z++) {
      for (uint8_t ch = 1; ch <= 80; ch++) {
        out.push_back(string_sprintf("Z%u / Ch %u", z, ch));
      }
    }
    return;
  }

  std::vector<ChannelInfo> sorted = this->channel_table_;
  std::sort(sorted.begin(), sorted.end(), [](const ChannelInfo &a, const ChannelInfo &b) {
    if (a.zone != b.zone)
      return a.zone < b.zone;
    return a.channel < b.channel;
  });

  for (const auto &ci : sorted) {
    if (!ci.label.empty()) {
      out.push_back(
          string_sprintf("Z%u / Ch %u: %s", ci.zone, ci.channel, ci.label.c_str()));
    } else {
      out.push_back(string_sprintf("Z%u / Ch %u", ci.zone, ci.channel));
    }
  }
}

void XRSRadioComponent::handle_line_(const std::string &line) {
  ESP_LOGD(TAG, "RX: %s", line.c_str());
  if (line == "OK" || line == "ERROR")
    return;

  auto starts_with = [&](const char *prefix) -> bool {
    size_t len = strlen(prefix);
    return line.size() >= len && line.compare(0, len, prefix) == 0;
  };

  if (starts_with("+GMI:")) {
    this->manufacturer_ = str_trim_copy(line.substr(5));
    for (auto &p : this->text_sensors_) {
      if (p.first == XRS_TEXT_MANUFACTURER)
        p.second->publish_state(this->manufacturer_);
    }
    return;
  }

  if (starts_with("+GMM:")) {
    this->model_ = str_trim_copy(line.substr(5));
    for (auto &p : this->text_sensors_) {
      if (p.first == XRS_TEXT_MODEL)
        p.second->publish_state(this->model_);
    }
    return;
  }

  if (starts_with("+GMR:")) {
    this->firmware_ = str_trim_copy(line.substr(5));
    for (auto &p : this->text_sensors_) {
      if (p.first == XRS_TEXT_FIRMWARE)
        p.second->publish_state(this->firmware_);
    }
    return;
  }

  if (starts_with("+GSN:")) {
    this->serial_ = str_trim_copy(line.substr(5));
    for (auto &p : this->text_sensors_) {
      if (p.first == XRS_TEXT_SERIAL)
        p.second->publish_state(this->serial_);
    }
    return;
  }

  if (starts_with("+WGAV:")) {
    int v = atoi(line.c_str() + 7);
    if (v < 0)
      v = 0;
    if (v > 31)
      v = 31;
    this->current_volume_ = v;
    for (auto &p : this->numeric_sensors_) {
      if (p.first == XRS_SENSOR_VOLUME)
        p.second->publish_state(this->current_volume_);
    }
    for (auto &p : this->numbers_) {
      if (p.first == XRS_NUMBER_VOLUME)
        p.second->publish_state(this->current_volume_);
    }
    return;
  }

  if (starts_with("+WGCHS:")) {
    int zone = 0;
    int ch = 0;
    if (sscanf(line.c_str(), "+WGCHS: %d,%d", &zone, &ch) == 2) {
      this->current_zone_ = zone;
      this->current_channel_ = ch;
      for (auto &p : this->numeric_sensors_) {
        if (p.first == XRS_SENSOR_ZONE)
          p.second->publish_state(this->current_zone_);
        else if (p.first == XRS_SENSOR_CHANNEL)
          p.second->publish_state(this->current_channel_);
      }
      this->publish_channel_label_();
    }
    return;
  }

  if (starts_with("+WHZS:")) {
    int zone = 0;
    if (sscanf(line.c_str(), "+WHZS: %d", &zone) == 1) {
      this->current_zone_ = zone;
      for (auto &p : this->numeric_sensors_) {
        if (p.first == XRS_SENSOR_ZONE)
          p.second->publish_state(this->current_zone_);
      }
      this->publish_channel_label_();
    }
    return;
  }

  if (starts_with("+WGPTT:")) {
    int state = 0;
    int timer = 0;
    int n = sscanf(line.c_str(), "+WGPTT: %d,%d", &state, &timer);
    if (n == 1)
      timer = 0;
    this->handle_ptt_notification_(state, timer);
    return;
  }

  if (starts_with("+WGPOW:")) {
    int state = 0;
    if (sscanf(line.c_str(), "+WGPOW: %d", &state) == 1) {
      this->handle_power_notification_(state);
    }
    return;
  }

  if (starts_with("+WGSCAN:")) {
    int enabled = 0;
    if (sscanf(line.c_str(), "+WGSCAN: %d", &enabled) == 1) {
      this->handle_scan_notification_(enabled);
    }
    return;
  }

  if (starts_with("+WGDUP:")) {
    int enabled = 0;
    if (sscanf(line.c_str(), "+WGDUP: %d", &enabled) == 1) {
      this->handle_duplex_notification_(enabled);
    }
    return;
  }

  if (starts_with("+WGCSM:")) {
    int enabled = 0;
    if (sscanf(line.c_str(), "+WGCSM: %d", &enabled) == 1) {
      this->handle_silent_memory_notification_(enabled);
    }
    return;
  }

  if (starts_with("+WGSQM:")) {
    int enabled = 0;
    if (sscanf(line.c_str(), "+WGSQM: %d", &enabled) == 1) {
      this->handle_quiet_memory_notification_(enabled);
    }
    return;
  }

  if (starts_with("+WGSSQ:")) {
    int enabled = 0;
    if (sscanf(line.c_str(), "+WGSSQ: %d", &enabled) == 1) {
      this->handle_quiet_mode_notification_(enabled);
    }
    return;
  }

  if (starts_with("+WGCHSQ:")) {
    this->handle_channel_table_line_(line);
    return;
  }

  if (!line.empty() && line[0] == '+') {
    for (auto &p : this->text_sensors_) {
      if (p.first == XRS_TEXT_LAST_MESSAGE)
        p.second->publish_state(line);
    }
  }
}

void XRSRadioComponent::spp_callback_static(esp_spp_cb_event_t event,
                                            esp_spp_cb_param_t *param) {
  if (XRSRadioComponent::instance_ != nullptr)
    XRSRadioComponent::instance_->on_spp_event_(event, param);
}

void XRSRadioComponent::on_spp_event_(esp_spp_cb_event_t event,
                                      esp_spp_cb_param_t *param) {
  switch (event) {
    case ESP_SPP_INIT_EVT: {
      ESP_LOGI(TAG, "ESP_SPP_INIT_EVT");
      this->spp_ready_ = true;
      this->reconnect_delay_ms_ = 2000;
      this->last_reconnect_attempt_ = 0;
      break;
    }

    case ESP_SPP_OPEN_EVT: {
      ESP_LOGI(TAG, "ESP_SPP_OPEN_EVT: connection opened");
      this->connected_ = true;
      this->connecting_ = false;
      this->spp_handle_ = param->open.handle;
      this->reconnect_delay_ms_ = 2000;
      this->last_reconnect_attempt_ = 0;
      this->publish_connection_state_();
      this->send_handshake_commands_();
      break;
    }

    case ESP_SPP_CLOSE_EVT: {
      ESP_LOGI(TAG, "ESP_SPP_CLOSE_EVT: connection closed");
      this->connected_ = false;
      this->connecting_ = false;
      this->spp_handle_ = 0;
      this->publish_connection_state_();
      break;
    }

    case ESP_SPP_DATA_IND_EVT: {
      const uint8_t *data = param->data_ind.data;
      const uint16_t len = param->data_ind.len;
      for (uint16_t i = 0; i < len; i++) {
        char c = static_cast<char>(data[i]);
        if (c == '\r')
          continue;
        if (c == '\n') {
          if (!this->rx_buffer_.empty()) {
            std::string line = this->rx_buffer_;
            this->rx_buffer_.clear();
            this->handle_line_(line);
          }
        } else {
          this->rx_buffer_.push_back(c);
        }
      }
      break;
    }

    default:
      ESP_LOGD(TAG, "Unhandled SPP event: %d", event);
      break;
  }
}

}  // namespace xrs_radio
}  // namespace esphome
