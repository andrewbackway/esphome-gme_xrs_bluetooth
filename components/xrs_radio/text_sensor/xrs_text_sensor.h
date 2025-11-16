#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

// Thin wrapper text sensor for xrs_radio platform.
class XRSRadioTextSensor : public text_sensor::TextSensor {
 public:
  void set_parent(XRSRadioComponent *parent) { parent_ = parent; }
  void set_type(XRSTextSensorType type) { type_ = type; }

 protected:
  XRSRadioComponent *parent_{nullptr};
  XRSTextSensorType type_{XRS_TEXT_MANUFACTURER};
};

}  // namespace xrs_radio
}  // namespace esphome
