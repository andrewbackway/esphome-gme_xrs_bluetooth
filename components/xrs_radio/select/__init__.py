import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID, CONF_TYPE
from esphome.components import select as select_base

from .. import XRSRadioComponent, XRSSelectType, CONF_XRS_ID, xrs_radio_ns

XRSRadioSelect = xrs_radio_ns.class_("XRSRadioSelect", select_base.Select, cg.Component)

XRS_RADIO_SELECT_TYPES = {
    "zone": XRSSelectType.XRS_SELECT_ZONE,
    "channel": XRSSelectType.XRS_SELECT_CHANNEL,
}

CONFIG_SCHEMA = select_base.select_schema(XRSRadioSelect).extend(
    {
        cv.GenerateID(): cv.declare_id(XRSRadioSelect),
        cv.GenerateID(CONF_XRS_ID): cv.use_id(XRSRadioComponent),
        cv.Required(CONF_TYPE): cv.one_of(*XRS_RADIO_SELECT_TYPES, lower=True),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await select_base.register_select(var, config)

    parent = await cg.get_variable(config[CONF_XRS_ID])
    type_enum = XRS_RADIO_SELECT_TYPES[config[CONF_TYPE]]

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(type_enum))
    cg.add(parent.register_select(type_enum, var))
