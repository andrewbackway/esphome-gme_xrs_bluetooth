import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_TYPE
from esphome.components import select as select_base

from .. import XRSRadioComponent, XRSSelectType, CONF_XRS_ID, xrs_radio_ns

# C++ class: class XRSRadioSelect : public select::Select, public Component { ... }
XRSRadioSelect = xrs_radio_ns.class_("XRSRadioSelect", select_base.Select, cg.Component)

XRS_RADIO_SELECT_TYPES = {
    "zone": XRSSelectType.XRS_SELECT_ZONE,
    "channel": XRSSelectType.XRS_SELECT_CHANNEL,
}

# New-style schema (2025.10+)
CONFIG_SCHEMA = select_base.select_schema(XRSRadioSelect).extend(
    {
        cv.GenerateID(CONF_XRS_ID): cv.use_id(XRSRadioComponent),
        cv.Required(CONF_TYPE): cv.one_of(*XRS_RADIO_SELECT_TYPES, lower=True),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_XRS_ID])
    type_enum = XRS_RADIO_SELECT_TYPES[config[CONF_TYPE]]

    # Create the select entity from the schema
    var = await select_base.new_select(config)

    # Wire it to the hub
    cg.add(var.set_parent(parent))
    cg.add(var.set_type(type_enum))
    cg.add(parent.register_select(type_enum, var))
