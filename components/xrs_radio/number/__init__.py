import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID, CONF_TYPE
from esphome.components import number as number_base

from .. import XRSRadioComponent, XRSNumberType, CONF_XRS_ID, xrs_radio_ns

XRSRadioNumber = xrs_radio_ns.class_("XRSRadioNumber", number_base.Number)

XRS_RADIO_NUMBER_TYPES = {
    "volume": XRSNumberType.XRS_NUMBER_VOLUME,
}

CONFIG_SCHEMA = number_base.NUMBER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(XRSRadioNumber),
        cv.GenerateID(CONF_XRS_ID): cv.use_id(XRSRadioComponent),
        cv.Required(CONF_TYPE): cv.one_of(*XRS_RADIO_NUMBER_TYPES, lower=True),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await number_base.register_number(var, config)

    parent = await cg.get_variable(config[CONF_XRS_ID])
    type_enum = XRS_RADIO_NUMBER_TYPES[config[CONF_TYPE]]

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(type_enum))
    cg.add(parent.register_number(type_enum, var))
