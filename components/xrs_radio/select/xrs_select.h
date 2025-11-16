#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/log.h"
#include "esphome/components/select/select.h"

#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

class XRSRadioSelect : public select::Select, public Component {
 public:
  void set_parent(XRSRadioComponent *parent) { this->parent_ = parent; }
  void set_type(XRSSelectType type) { this->type_ = type; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void refresh_from_parent();
 protected:
  // Called when HA/user changes the selected option
  void control(const std::string &value) override;

  // Internal: refresh traits.options from the hub’s channel/zone table
  void update_options_();

  XRSRadioComponent *parent_{nullptr};
  XRSSelectType type_{XRSSelectType::XRS_SELECT_ZONE};
};

}  // namespace xrs_radio
}  // namespace esphome
