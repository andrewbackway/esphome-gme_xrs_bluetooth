#pragma once

#include "esphome/components/number/number.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

// Volume number entity, uses XRSRadioComponent::set_volume().
class XRSRadioNumber : public number::Number {
 public:
  void set_parent(XRSRadioComponent *parent) { parent_ = parent; }
  void set_type(XRSNumberType type) { type_ = type; }

 protected:
  void control(float value) override;

  XRSRadioComponent *parent_{nullptr};
  XRSNumberType type_{XRS_NUMBER_VOLUME};
};

}  // namespace xrs_radio
}  // namespace esphome
