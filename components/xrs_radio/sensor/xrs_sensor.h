#pragma once

#include "esphome/components/sensor/sensor.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

// Thin wrapper sensor used by the xrs_radio platform so that each configured
// sensor is a distinct C++ type.
class XRSRadioSensor : public sensor::Sensor {
 public:
  void set_parent(XRSRadioComponent *parent) { parent_ = parent; }
  void set_type(XRSNumericSensorType type) { type_ = type; }

 protected:
  XRSRadioComponent *parent_{nullptr};
  XRSNumericSensorType type_{XRS_SENSOR_CHANNEL};
};

}  // namespace xrs_radio
}  // namespace esphome
