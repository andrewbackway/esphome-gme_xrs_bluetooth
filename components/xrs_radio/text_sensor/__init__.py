import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID, CONF_TYPE
from esphome.components import text_sensor as ts_base

from .. import XRSRadioComponent, XRSTextSensorType, CONF_XRS_ID, xrs_radio_ns

XRSRadioTextSensor = xrs_radio_ns.class_("XRSRadioTextSensor", ts_base.TextSensor)

XRS_RADIO_TEXT_TYPES = {
    "manufacturer": XRSTextSensorType.XRS_TEXT_MANUFACTURER,
    "model": XRSTextSensorType.XRS_TEXT_MODEL,
    "firmware": XRSTextSensorType.XRS_TEXT_FIRMWARE,
    "serial": XRSTextSensorType.XRS_TEXT_SERIAL,
    "last_message": XRSTextSensorType.XRS_TEXT_LAST_MESSAGE,
    "power_state": XRSTextSensorType.XRS_TEXT_POWER_STATE,
    "ptt_state": XRSTextSensorType.XRS_TEXT_PTT_STATE,
    "channel_label": XRSTextSensorType.XRS_TEXT_CHANNEL_LABEL,
}

CONFIG_SCHEMA = ts_base.text_sensor_schema(XRSRadioTextSensor).extend(
    {
        cv.GenerateID(): cv.declare_id(XRSRadioTextSensor),
        cv.GenerateID(CONF_XRS_ID): cv.use_id(XRSRadioComponent),
        cv.Required(CONF_TYPE): cv.one_of(*XRS_RADIO_TEXT_TYPES, lower=True),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await ts_base.register_text_sensor(var, config)

    parent = await cg.get_variable(config[CONF_XRS_ID])
    type_enum = XRS_RADIO_TEXT_TYPES[config[CONF_TYPE]]

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(type_enum))
    cg.add(parent.register_text_sensor(type_enum, var))
