#include "xrs_switch.h"
#include "../xrs_radio.h"

namespace esphome {
namespace xrs_radio {

void XRSRadioSwitch::write_state(bool state) {
  if (this->parent_ == nullptr)
    return;

  switch (this->type_) {
    case XRS_SWITCH_LOCATION_MODE:
      this->parent_->set_location_mode(state);
      break;
    case XRS_SWITCH_SCAN:
      this->parent_->set_scan_enabled(state);
      break;
    case XRS_SWITCH_DUPLEX:
      this->parent_->set_duplex_enabled(state);
      break;
    case XRS_SWITCH_QUIET_MODE:
      this->parent_->set_quiet_mode(state);
      break;
    case XRS_SWITCH_QUIET_MEMORY:
      this->parent_->set_quiet_memory(state);
      break;
    case XRS_SWITCH_SILENT_MEMORY:
      this->parent_->set_silent_memory(state);
      break;
  }
}

}  // namespace xrs_radio
}  // namespace esphome
