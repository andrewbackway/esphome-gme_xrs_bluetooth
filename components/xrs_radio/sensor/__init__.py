import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID, CONF_TYPE
from esphome.components import sensor as sensor_base

from .. import XRSRadioComponent, XRSNumericSensorType, CONF_XRS_ID, xrs_radio_ns

XRSRadioSensor = xrs_radio_ns.class_("XRSRadioSensor", sensor_base.Sensor)

XRS_RADIO_SENSOR_TYPES = {
    "channel": XRSNumericSensorType.XRS_SENSOR_CHANNEL,
    "zone": XRSNumericSensorType.XRS_SENSOR_ZONE,
    "volume": XRSNumericSensorType.XRS_SENSOR_VOLUME,
    "ptt_timer": XRSNumericSensorType.XRS_SENSOR_PTT_TIMER,
}

CONFIG_SCHEMA = sensor_base.SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(XRSRadioSensor),
        cv.GenerateID(CONF_XRS_ID): cv.use_id(XRSRadioComponent),
        cv.Required(CONF_TYPE): cv.one_of(*XRS_RADIO_SENSOR_TYPES, lower=True),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor_base.register_sensor(var, config)

    parent = await cg.get_variable(config[CONF_XRS_ID])
    type_enum = XRS_RADIO_SENSOR_TYPES[config[CONF_TYPE]]

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(type_enum))
    cg.add(parent.register_numeric_sensor(type_enum, var))
