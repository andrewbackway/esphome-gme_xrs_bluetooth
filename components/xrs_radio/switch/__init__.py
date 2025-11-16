import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID, CONF_TYPE
from esphome.components import switch as switch_base

from .. import XRSRadioComponent, XRSSwitchType, CONF_XRS_ID, xrs_radio_ns

XRSRadioSwitch = xrs_radio_ns.class_("XRSRadioSwitch", switch_base.Switch)

XRS_RADIO_SWITCH_TYPES = {
    "location_mode": XRSSwitchType.XRS_SWITCH_LOCATION_MODE,
    "scan": XRSSwitchType.XRS_SWITCH_SCAN,
    "duplex": XRSSwitchType.XRS_SWITCH_DUPLEX,
    "quiet_mode": XRSSwitchType.XRS_SWITCH_QUIET_MODE,
    "quiet_memory": XRSSwitchType.XRS_SWITCH_QUIET_MEMORY,
    "silent_memory": XRSSwitchType.XRS_SWITCH_SILENT_MEMORY,
}

CONFIG_SCHEMA = switch_base.switch_schema(XRSRadioSwitch).extend(
    {
        cv.GenerateID(): cv.declare_id(XRSRadioSwitch),
        cv.GenerateID(CONF_XRS_ID): cv.use_id(XRSRadioComponent),
        cv.Required(CONF_TYPE): cv.one_of(*XRS_RADIO_SWITCH_TYPES, lower=True),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await switch_base.register_switch(var, config)

    parent = await cg.get_variable(config[CONF_XRS_ID])
    type_enum = XRS_RADIO_SWITCH_TYPES[config[CONF_TYPE]]

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(type_enum))
    cg.add(parent.register_switch(type_enum, var))
