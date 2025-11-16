#include "xrs_number.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

void XRSRadioNumber::control(float value) {
  if (this->parent_ == nullptr)
    return;

  switch (this->type_) {
    case XRS_NUMBER_VOLUME:
      this->parent_->set_volume(value);
      break;
  }
}

}  // namespace xrs_radio
}  // namespace esphome
