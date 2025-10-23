# mppt_controller/sensor.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, text_sensor
from esphome.const import CONF_ID

from . import mppt_ns, MPPTController

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MPPTController),
    cv.Optional("pv_voltage"): sensor.sensor_schema(unit_of_measurement="V", icon="mdi:solar-panel"),
    cv.Optional("pv_current"): sensor.sensor_schema(unit_of_measurement="A"),
    cv.Optional("pv_power"): sensor.sensor_schema(unit_of_measurement="W", icon="mdi:lightning-bolt"),
    cv.Optional("battery_voltage"): sensor.sensor_schema(unit_of_measurement="V"),
    cv.Optional("temperature"): sensor.sensor_schema(unit_of_measurement="°C"),
    cv.Optional("load_current"): sensor.sensor_schema(unit_of_measurement="A"),
    cv.Optional("daily_energy"): sensor.sensor_schema(unit_of_measurement="kWh"),
    cv.Optional("total_energy"): sensor.sensor_schema(unit_of_measurement="kWh"),
    # Add text sensors
    cv.Optional("error_code"): text_sensor.text_sensor_schema(icon="mdi:alert"),
    cv.Optional("working_mode"): text_sensor.text_sensor_schema(icon="mdi:state-machine"),
})

async def to_code(config):
    var = await cg.get_variable(config[CONF_ID])
    for key, setter in [
        ("pv_voltage", var.set_pv_voltage_sensor),
        ("pv_current", var.set_pv_current_sensor),
        ("pv_power", var.set_pv_power_sensor),
        ("battery_voltage", var.set_battery_voltage_sensor),
        ("temperature", var.set_temperature_sensor),
        ("load_current", var.set_load_current_sensor),
        ("daily_energy", var.set_daily_energy_sensor),
        ("total_energy", var.set_total_energy_sensor),
        ("error_code", var.set_error_code_sensor),
        ("working_mode", var.set_working_mode_sensor),
    ]:
        if key in config:
            if key in ["error_code", "working_mode"]:
                # For text sensors
                sens = await text_sensor.new_text_sensor(config[key])
            else:
                # For regular sensors
                sens = await sensor.new_sensor(config[key])
            cg.add(setter(sens))
