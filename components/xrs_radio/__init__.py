import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import (
    CONF_ID,
    CONF_MAC_ADDRESS,
)

from esphome.components import sensor as sensor_comp

DEPENDENCIES = ["esp32"]

xrs_radio_ns = cg.esphome_ns.namespace("xrs_radio")

XRSNumericSensorType = xrs_radio_ns.enum("XRSNumericSensorType")
XRSBinarySensorType = xrs_radio_ns.enum("XRSBinarySensorType")
XRSTextSensorType = xrs_radio_ns.enum("XRSTextSensorType")
XRSNumberType = xrs_radio_ns.enum("XRSNumberType")
XRSSwitchType = xrs_radio_ns.enum("XRSSwitchType")
XRSSelectType = xrs_radio_ns.enum("XRSSelectType")

XRSRadioComponent = xrs_radio_ns.class_("XRSRadioComponent", cg.Component)

CONF_XRS_ID = "xrs_id"
CONF_LATITUDE_SENSOR = "latitude_sensor"
CONF_LONGITUDE_SENSOR = "longitude_sensor"
CONF_LOCATION_INTERVAL = "location_interval"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(XRSRadioComponent),
        cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
        cv.Optional(CONF_LATITUDE_SENSOR): cv.use_id(sensor_comp.Sensor),
        cv.Optional(CONF_LONGITUDE_SENSOR): cv.use_id(sensor_comp.Sensor),
        cv.Optional(CONF_LOCATION_INTERVAL, default="60s"): cv.positive_time_period_milliseconds,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_mac_address(config[CONF_MAC_ADDRESS]))

    lat_id = config.get(CONF_LATITUDE_SENSOR)
    lon_id = config.get(CONF_LONGITUDE_SENSOR)
    if lat_id is not None and lon_id is not None:
        lat = await cg.get_variable(lat_id)
        lon = await cg.get_variable(lon_id)
        cg.add(var.set_location_sensors(lat, lon))

    cg.add(var.set_location_interval(config[CONF_LOCATION_INTERVAL]))
