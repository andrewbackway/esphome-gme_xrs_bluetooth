#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

// Thin wrapper binary sensor for xrs_radio platform.
class XRSRadioBinarySensor : public binary_sensor::BinarySensor {
 public:
  void set_parent(XRSRadioComponent *parent) { parent_ = parent; }
  void set_type(XRSBinarySensorType type) { type_ = type; }

 protected:
  XRSRadioComponent *parent_{nullptr};
  XRSBinarySensorType type_{XRS_BIN_CONNECTED};
};

}  // namespace xrs_radio
}  // namespace esphome
