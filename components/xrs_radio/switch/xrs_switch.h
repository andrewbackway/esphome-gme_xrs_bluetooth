#pragma once

#include "esphome/components/switch/switch.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

// Simple switch entity, used for location/scan/duplex/quiet/silent controls.
class XRSRadioSwitch : public switch_::Switch {
 public:
  void set_parent(XRSRadioComponent *parent) { parent_ = parent; }
  void set_type(XRSSwitchType type) { type_ = type; }

 protected:
  void write_state(bool state) override;

  XRSRadioComponent *parent_{nullptr};
  XRSSwitchType type_{XRS_SWITCH_LOCATION_MODE};
};

}  // namespace xrs_radio
}  // namespace esphome
