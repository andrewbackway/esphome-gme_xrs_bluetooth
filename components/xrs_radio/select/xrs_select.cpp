#include "xrs_select.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

static const char *const TAG_SELECT = "xrs_radio.select";

void XRSRadioSelect::setup() {
  if (this->parent_ == nullptr)
    return;

  this->parent_->register_select(this->type_, this);
  this->refresh_options();

  if (this->type_ == XRS_SELECT_ZONE) {
    uint8_t zone = this->parent_->get_current_zone();
    std::string desired = string_sprintf("Zone %u", zone);
    auto it = std::find(this->options_.begin(), this->options_.end(), desired);
    if (it != this->options_.end())
      this->publish_state(*it);
    else
      this->publish_state(desired);
  } else if (this->type_ == XRS_SELECT_CHANNEL) {
    uint8_t z = this->parent_->get_current_zone();
    uint8_t ch = this->parent_->get_current_channel();
    std::string desired = string_sprintf("Z%u / Ch %u", z, ch);
    std::string best = desired;
    for (const auto &opt : this->options_) {
      if (opt.rfind(desired, 0) == 0) {
        best = opt;
        break;
      }
    }
    this->publish_state(best);
  }
}

void XRSRadioSelect::dump_config() {
  ESP_LOGCONFIG(TAG_SELECT, "XRS Radio Select");
}

void XRSRadioSelect::refresh_options() {
  this->options_.clear();
  if (this->parent_ == nullptr)
    return;

  std::vector<std::string> opts;
  if (this->type_ == XRS_SELECT_ZONE) {
    this->parent_->get_zone_options(opts);
  } else if (this->type_ == XRS_SELECT_CHANNEL) {
    this->parent_->get_channel_options(opts);
  }
  this->options_ = std::move(opts);
}

select::SelectTraits XRSRadioSelect::get_traits() {
  auto traits = select::SelectTraits{};
  traits.set_options(this->options_);
  return traits;
}

void XRSRadioSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG_SELECT, "No parent XRSRadioComponent bound");
    return;
  }

  if (this->type_ == XRS_SELECT_ZONE) {
    uint8_t zone = 0;
    if (sscanf(value.c_str(), "Zone %hhu", &zone) != 1) {
      if (sscanf(value.c_str(), "%hhu", &zone) != 1) {
        ESP_LOGW(TAG_SELECT, "Cannot parse zone from '%s'", value.c_str());
        return;
      }
    }
    this->parent_->set_target_zone(zone);
    this->publish_state(value);
  } else if (this->type_ == XRS_SELECT_CHANNEL) {
    uint8_t zone = 0;
    uint8_t ch = 0;
    if (sscanf(value.c_str(), "Z%hhu / Ch %hhu", &zone, &ch) != 2) {
      if (sscanf(value.c_str(), "%hhu,%hhu", &zone, &ch) != 2) {
        ESP_LOGW(TAG_SELECT, "Cannot parse zone/channel from '%s'", value.c_str());
        return;
      }
    }
    this->parent_->set_target_zone_channel(zone, ch);
    this->publish_state(value);
  }
}

}  // namespace xrs_radio
}  // namespace esphome
