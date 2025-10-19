# external_components/mppt_controller/sensor.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID

from . import mppt_ns, MPPTController

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MPPTController),
    cv.Optional("pv_voltage"): sensor.sensor_schema(unit_of_measurement="V", icon="mdi:solar-panel"),
    cv.Optional("pv_current"): sensor.sensor_schema(unit_of_measurement="A"),
    cv.Optional("battery_voltage"): sensor.sensor_schema(unit_of_measurement="V"),
    cv.Optional("temperature"): sensor.sensor_schema(unit_of_measurement="°C"),
    cv.Optional("load_current"): sensor.sensor_schema(unit_of_measurement="A"),
    cv.Optional("daily_energy"): sensor.sensor_schema(unit_of_measurement="kWh"),
    cv.Optional("total_energy"): sensor.sensor_schema(unit_of_measurement="kWh"),
})

async def to_code(config):
    var = await cg.get_variable(config[CONF_ID])
    for key, setter in [
        ("pv_voltage", var.set_pv_voltage_sensor),
        ("pv_current", var.set_pv_current_sensor),
        ("battery_voltage", var.set_battery_voltage_sensor),
        ("temperature", var.set_temperature_sensor),
        ("load_current", var.set_load_current_sensor),
        ("daily_energy", var.set_daily_energy_sensor),
        ("total_energy", var.set_total_energy_sensor),
    ]:
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(setter(sens))

