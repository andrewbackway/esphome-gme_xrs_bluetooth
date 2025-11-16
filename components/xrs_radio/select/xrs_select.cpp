#include "xrs_select.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace xrs_radio {

static const char *const TAG = "xrs_radio.select";

void XRSRadioSelect::setup() {
  // Populate initial options from the hub
  this->update_options_();
}

void XRSRadioSelect::dump_config() {
  ESP_LOGCONFIG(TAG, "XRS Radio Select");
  LOG_SELECT("  ", "XRS Select", this);
}

void XRSRadioSelect::update_options_() {
  if (this->parent_ == nullptr)
    return;

  std::vector<std::string> opts;
  if (this->type_ == XRSSelectType::XRS_SELECT_ZONE) {
    this->parent_->get_zone_options(opts);
  } else {
    this->parent_->get_channel_options(opts);
  }

  this->traits.set_options(opts);
}

void XRSRadioSelect::control(const std::string &value) {
  if (this->parent_ == nullptr)
    return;

  if (this->type_ == XRSSelectType::XRS_SELECT_ZONE) {
    // Expect values like "Zone 1"
    unsigned zone = 0;
    if (sscanf(value.c_str(), "Zone %u", &zone) == 1) {
      this->parent_->set_zone(static_cast<uint8_t>(zone));
      this->publish_state(value);
    }
  } else {
    // Expect values like "Z1 / Ch 12"
    unsigned zone = 0;
    unsigned ch = 0;
    if (sscanf(value.c_str(), "Z%u / Ch %u", &zone, &ch) == 2) {
      this->parent_->set_channel(static_cast<uint8_t>(zone), static_cast<uint8_t>(ch));
      this->publish_state(value);
    }
  }
}

}  // namespace xrs_radio
}  // namespace esphome
