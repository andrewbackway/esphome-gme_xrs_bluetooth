import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID, CONF_TYPE
from esphome.components import binary_sensor as bs_base

from .. import XRSRadioComponent, XRSBinarySensorType, CONF_XRS_ID, xrs_radio_ns

XRSRadioBinarySensor = xrs_radio_ns.class_("XRSRadioBinarySensor", bs_base.BinarySensor)

XRS_RADIO_BINARY_TYPES = {
    "connected": XRSBinarySensorType.XRS_BIN_CONNECTED,
    "ptt_active": XRSBinarySensorType.XRS_BIN_PTT_ACTIVE,
    "ptt_data": XRSBinarySensorType.XRS_BIN_PTT_DATA,
    "power_low": XRSBinarySensorType.XRS_BIN_POWER_LOW,
    "scanning": XRSBinarySensorType.XRS_BIN_SCANNING,
    "duplex_enabled": XRSBinarySensorType.XRS_BIN_DUPLEX_ENABLED,
    "silent_memory": XRSBinarySensorType.XRS_BIN_SILENT_MEMORY,
    "quiet_memory": XRSBinarySensorType.XRS_BIN_QUIET_MEMORY,
    "quiet_mode": XRSBinarySensorType.XRS_BIN_QUIET_MODE,
}

CONFIG_SCHEMA = bs_base.binary_sensor_schema(XRSRadioBinarySensor).extend(
    {
        cv.GenerateID(): cv.declare_id(XRSRadioBinarySensor),
        cv.GenerateID(CONF_XRS_ID): cv.use_id(XRSRadioComponent),
        cv.Required(CONF_TYPE): cv.one_of(*XRS_RADIO_BINARY_TYPES, lower=True),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await bs_base.register_binary_sensor(var, config)

    parent = await cg.get_variable(config[CONF_XRS_ID])
    type_enum = XRS_RADIO_BINARY_TYPES[config[CONF_TYPE]]

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(type_enum))
    cg.add(parent.register_binary_sensor(type_enum, var))
