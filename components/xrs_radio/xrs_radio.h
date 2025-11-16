#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cmath>

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/select/select.h"

extern "C" {
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
}

namespace esphome {
namespace xrs_radio {

// Numeric sensor types (channel, zone, volume, PTT timer)
enum XRSNumericSensorType {
  XRS_SENSOR_CHANNEL = 0,
  XRS_SENSOR_ZONE = 1,
  XRS_SENSOR_VOLUME = 2,
  XRS_SENSOR_PTT_TIMER = 3,
};

// Binary sensor types (connection, PTT, power, scan, duplex, memories, quiet mode)
enum XRSBinarySensorType {
  XRS_BIN_CONNECTED = 0,
  XRS_BIN_PTT_ACTIVE = 1,
  XRS_BIN_PTT_DATA = 2,
  XRS_BIN_POWER_LOW = 3,
  XRS_BIN_SCANNING = 4,
  XRS_BIN_DUPLEX_ENABLED = 5,
  XRS_BIN_SILENT_MEMORY = 6,
  XRS_BIN_QUIET_MEMORY = 7,
  XRS_BIN_QUIET_MODE = 8,
};

// Text sensor types (device info, last message, PTT/power state, channel label)
enum XRSTextSensorType {
  XRS_TEXT_MANUFACTURER = 0,
  XRS_TEXT_MODEL = 1,
  XRS_TEXT_FIRMWARE = 2,
  XRS_TEXT_SERIAL = 3,
  XRS_TEXT_LAST_MESSAGE = 4,
  XRS_TEXT_POWER_STATE = 5,
  XRS_TEXT_PTT_STATE = 6,
  XRS_TEXT_CHANNEL_LABEL = 7,
};

// Number entities (writeable numeric controls)
enum XRSNumberType {
  XRS_NUMBER_VOLUME = 0,
};

// Switch entities (writeable boolean controls)
enum XRSSwitchType {
  XRS_SWITCH_LOCATION_MODE = 0,
  XRS_SWITCH_SCAN = 1,
  XRS_SWITCH_DUPLEX = 2,
  XRS_SWITCH_QUIET_MODE = 3,
  XRS_SWITCH_QUIET_MEMORY = 4,
  XRS_SWITCH_SILENT_MEMORY = 5,
};

// Select entities for zone/channel control.
enum XRSSelectType {
  XRS_SELECT_ZONE = 0,
  XRS_SELECT_CHANNEL = 1,
};

class XRSRadioComponent;
class XRSRadioSensor;
class XRSRadioBinarySensor;
class XRSRadioTextSensor;
class XRSRadioNumber;
class XRSRadioSwitch;
class XRSRadioSelect;

// Core component managing Bluetooth SPP, AT commands and state.
class XRSRadioComponent : public Component {
 public:
  XRSRadioComponent();

  // Store MAC address string ("AA:BB:CC:DD:EE:FF") of the XRS radio.
  void set_mac_address(const std::string &mac);

  // Store external latitude/longitude sensors used for AT+WGTLOC payload.
  void set_location_sensors(sensor::Sensor *lat, sensor::Sensor *lon);

  // Configure interval between automatic AT+WGTLOC commands (milliseconds).
  void set_location_interval(uint32_t interval_ms);

  // Register a numeric sensor (channel, zone, volume, PTT timer).
  void register_numeric_sensor(XRSNumericSensorType type, XRSRadioSensor *s);

  // Register a binary sensor (connection, PTT flags, power low, scan etc.).
  void register_binary_sensor(XRSBinarySensorType type, XRSRadioBinarySensor *s);

  // Register a text sensor (device info, last message, power/PTT state).
  void register_text_sensor(XRSTextSensorType type, XRSRadioTextSensor *s);

  // Register a number entity (volume control).
  void register_number(XRSNumberType type, XRSRadioNumber *n);

  // Register a switch entity (location/scan/duplex/quiet/silent controls).
  void register_switch(XRSSwitchType type, XRSRadioSwitch *sw);

  // Register a select entity (zone/channel).
  void register_select(XRSSelectType type, XRSRadioSelect *sel);

  // Set volume on the radio (0–31) using AT+WGAV=<volume>.
  void set_volume(float volume);

  // Enable/disable location mode (controls AT+WGTLOC schedule).
  void set_location_mode(bool enabled);

  // Enable/disable scan mode (AT+WGSCAN=<0/1>).
  void set_scan_enabled(bool enabled);

  // Enable/disable duplex mode (AT+WGDUP=<0/1>).
  void set_duplex_enabled(bool enabled);

  // Enable/disable quiet mode (AT+WGSSQ=<0/1>).
  void set_quiet_mode(bool enabled);

  // Enable/disable quiet memory for current channel (AT+WGSQM=<0/1>).
  void set_quiet_memory(bool enabled);

  // Enable/disable silent memory for current channel (AT+WGCSM=<0/1>).
  void set_silent_memory(bool enabled);

  // Request a zone change from the radio (AT+WGZS=<zone>).
  void set_target_zone(uint8_t zone);

  // Request a zone+channel change from the radio (AT+WGCHS=<zone>,<channel>).
  void set_target_zone_channel(uint8_t zone, uint8_t channel);

  // Get the current zone/channel for select initial state.
  uint8_t get_current_zone() const { return static_cast<uint8_t>(current_zone_); }
  uint8_t get_current_channel() const { return static_cast<uint8_t>(current_channel_); }

  // Fill out available zone options, e.g. ["Zone 1", "Zone 2", ...].
  void get_zone_options(std::vector<std::string> &out) const;

  // Fill out channel options, e.g. ["Z1 / Ch 40: CH40", ...].
  void get_channel_options(std::vector<std::string> &out) const;

  /// Set the current zone on the radio and cache state
  void set_zone(uint8_t zone);

  /// Set the current zone + channel on the radio and cache state
  void set_channel(uint8_t zone, uint8_t channel);

  // Request a full channel/squelch table from the radio (AT_WGCHSQ).
  void request_channel_table();

  // Standard ESPHome lifecycle: initialize BT/SPP and start connection attempts.
  void setup() override;

  // Standard ESPHome lifecycle: handle reconnect backoff and location tick.
  void loop() override;

  // Standard ESPHome lifecycle: dump configuration and current state to the log.
  void dump_config() override;

 protected:
  // Single channel entry from the radio’s channel/squelch table.
  struct ChannelInfo {
    uint8_t zone;
    uint8_t channel;
    float rx_freq;
    float tx_freq;
    std::string label;
  };

  esp_bd_addr_t target_mac_{};

  // Initialize ESP32 Bluetooth Classic controller and SPP stack.
  void init_bluetooth_();

  // Start SPP connection to the configured MAC address.
  void start_connection_();

  // Close current SPP connection if any.
  void close_connection_();

  // Handle a complete AT/notification line received from the radio.
  void handle_line_(const std::string &line);

  // Send an AT command line terminated with CRLF over SPP.
  void send_command_(const std::string &cmd);

  // Send initial identification and setup commands after SPP connect.
  void send_handshake_commands_();

  // Build and send AT+WGTLOC=<time>,<lat>,<lon> using configured location sensors.
  void send_location_update_();

  // Publish all current state values to registered sensors/entities.
  void publish_all_state_();

  // Publish connection state to any registered "connected" binary sensors.
  void publish_connection_state_();

  // Parse and handle +WGPTT notification from the radio.
  void handle_ptt_notification_(int state, int timer);

  // Parse and handle +WGPOW notification from the radio.
  void handle_power_notification_(int state);

  // Parse and handle +WGSCAN notification from the radio.
  void handle_scan_notification_(int enabled);

  // Parse and handle +WGDUP notification from the radio.
  void handle_duplex_notification_(int enabled);

  // Parse and handle +WGCSM notification from the radio.
  void handle_silent_memory_notification_(int enabled);

  // Parse and handle +WGSQM notification from the radio.
  void handle_quiet_memory_notification_(int enabled);

  // Parse and handle +WGSSQ notification from the radio.
  void handle_quiet_mode_notification_(int enabled);

  // Parse a +WGCHSQ: ... line and update internal channel table.
  void handle_channel_table_line_(const std::string &line);

  // Publish a label for the current zone/channel to XRS_TEXT_CHANNEL_LABEL sensors.
  void publish_channel_label_();

  // Find label for given zone/channel in channel_table_ (empty if unknown).
  std::string get_channel_label_(uint8_t zone, uint8_t channel) const;

  // ESP-IDF SPP callback static entry, forwarding events to instance_.
  static void spp_callback_static(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

  // ESP-IDF SPP callback implementation on the active instance.
  void on_spp_event_(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

  // Convert "AA:BB:CC:DD:EE:FF" into esp_bd_addr_t (6 bytes).
  bool parse_mac_address_(esp_bd_addr_t out);

  static XRSRadioComponent *instance_;

  std::string mac_address_;
  bool bt_initialized_{false};
  bool spp_ready_{false};
  bool connected_{false};
  bool connecting_{false};
  uint32_t spp_handle_{0};

  std::string rx_buffer_;

  // Reconnect/backoff state.
  uint32_t reconnect_delay_ms_{2000};
  uint32_t last_reconnect_attempt_{0};
  static constexpr uint32_t RECONNECT_DELAY_MAX_MS = 60000;

  // Identification info from AT+GMI?, +GMM?, +GMR?, +GSN?.
  std::string manufacturer_;
  std::string model_;
  std::string firmware_;
  std::string serial_;

  // Basic radio state.
  int current_channel_{0};
  int current_zone_{0};
  int current_volume_{0};

  // Extended state from notifications.
  bool ptt_active_{false};
  bool ptt_data_{false};
  int ptt_timer_{0};
  int power_state_{0};
  bool power_low_{false};
  bool scanning_{false};
  bool duplex_enabled_{false};
  bool silent_memory_{false};
  bool quiet_memory_{false};
  bool quiet_mode_{false};

  // Registered sensors/entities.
  std::vector<std::pair<XRSNumericSensorType, XRSRadioSensor *>> numeric_sensors_;
  std::vector<std::pair<XRSBinarySensorType, XRSRadioBinarySensor *>> binary_sensors_;
  std::vector<std::pair<XRSTextSensorType, XRSRadioTextSensor *>> text_sensors_;
  std::vector<std::pair<XRSNumberType, XRSRadioNumber *>> numbers_;
  std::vector<std::pair<XRSSwitchType, XRSRadioSwitch *>> switches_;
  std::vector<std::pair<XRSSelectType, XRSRadioSelect *>> selects_;

  // Location upload configuration/state.
  sensor::Sensor *latitude_sensor_{nullptr};
  sensor::Sensor *longitude_sensor_{nullptr};
  bool location_mode_{false};
  uint32_t location_interval_ms_{60000};
  uint32_t last_location_sent_{0};

  // Channel table from radio.
  std::vector<ChannelInfo> channel_table_;
};

}  // namespace xrs_radio
}  // namespace esphome
