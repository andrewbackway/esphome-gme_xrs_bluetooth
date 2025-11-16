import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    CONF_MIN_VALUE,
    CONF_MAX_VALUE,
    CONF_STEP,
)
from esphome.components import number as number_base

from .. import XRSRadioComponent, XRSNumberType, CONF_XRS_ID, xrs_radio_ns

# C++ class: class XRSRadioNumber : public number::Number { ... }
XRSRadioNumber = xrs_radio_ns.class_("XRSRadioNumber", number_base.Number)

# Map YAML "type: ..." to the C++ enum
XRS_RADIO_NUMBER_TYPES = {
    "volume": XRSNumberType.XRS_NUMBER_VOLUME,
}

# New-style schema for ESPHome 2025.x
CONFIG_SCHEMA = number_base.number_schema(XRSRadioNumber).extend(
    {
        # Link to the hub
        cv.GenerateID(CONF_XRS_ID): cv.use_id(XRSRadioComponent),

        # Which logical control this number drives
        cv.Required(CONF_TYPE): cv.one_of(*XRS_RADIO_NUMBER_TYPES, lower=True),

        # Per-entity bounds and step, with radio-friendly defaults
        cv.Optional(CONF_MIN_VALUE, default=0.0): cv.float_,
        cv.Optional(CONF_MAX_VALUE, default=31.0): cv.float_,
        cv.Optional(CONF_STEP, default=1.0): cv.positive_float,
    }
)


async def to_code(config):
    # Get parent hub
    parent = await cg.get_variable(config[CONF_XRS_ID])

    # Create the number entity with min/max/step
    var = await number_base.new_number(
        config,
        min_value=config[CONF_MIN_VALUE],
        max_value=config[CONF_MAX_VALUE],
        step=config[CONF_STEP],
    )

    # Tie into C++ side
    type_enum = XRS_RADIO_NUMBER_TYPES[config[CONF_TYPE]]

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(type_enum))
    cg.add(parent.register_number(type_enum, var))
