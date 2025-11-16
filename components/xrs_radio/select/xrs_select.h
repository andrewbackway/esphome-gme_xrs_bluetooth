#pragma once

#include <vector>
#include <string>

#include "esphome/core/component.h"
#include "esphome/components/select/select.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

// Select entity for zone/channel backed by XRSRadioComponent.
class XRSRadioSelect : public select::Select, public Component {
 public:
  void set_parent(XRSRadioComponent *parent) { parent_ = parent; }
  void set_type(XRSSelectType type) { type_ = type; }

  void setup() override;
  void dump_config() override;

  // Refresh internal option list from parent's channel table.
  void refresh_options();

  // Called when HA/UI selects a new option.
  void control(const std::string &value) override;

  // Expose traits with our dynamic options.
  select::SelectTraits get_traits() override;

 protected:
  XRSRadioComponent *parent_{nullptr};
  XRSSelectType type_{XRS_SELECT_ZONE};
  std::vector<std::string> options_;
};

}  // namespace xrs_radio
}  // namespace esphome
