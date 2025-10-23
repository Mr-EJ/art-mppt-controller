import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL

CODEOWNERS = ["@edwardallen"]

# Define namespace and component class
mppt_ns = cg.esphome_ns.namespace("mppt_controller")
MPPTController = mppt_ns.class_("MPPTController", cg.PollingComponent, uart.UARTDevice)

# Import submodules
from . import sensor as mppt_sensor
from . import switch as mppt_switch
from . import services as mppt_services

# Combine schemas from submodules into main schema
CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(MPPTController),
        cv.Optional(CONF_UPDATE_INTERVAL, default="5s"): cv.update_interval,
    })
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("5s"))
    .extend(mppt_sensor.CONFIG_SCHEMA)   # sensors
    .extend(mppt_switch.CONFIG_SCHEMA)   # switches
)

# Generate code
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    # Delegate to submodules
    await mppt_sensor.to_code(config)
    await mppt_switch.to_code(config)
    await mppt_services.to_code(config)
