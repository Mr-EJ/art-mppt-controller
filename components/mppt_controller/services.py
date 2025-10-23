import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.automation as automation
from esphome.const import CONF_ID
from . import mppt_ns, MPPTController

# Remove the template parameters from action class definitions
MPPTResetAction = mppt_ns.class_("MPPTResetAction", automation.Action)
MPPTSetChargingParamsAction = mppt_ns.class_("MPPTSetChargingParamsAction", automation.Action)

CONF_CLEAR = 'clear'
CONF_REBOOT = 'reboot'
CONF_CHARGE_CURRENT = 'charge_current'
CONF_BATTERY_TYPE = 'battery_type'
CONF_CONST_VOLTAGE = 'const_voltage'
CONF_LOAD_UNDERVOLTAGE = 'load_undervoltage'

@automation.register_action(
    "mppt_controller.reset_energy",
    MPPTResetAction,
    cv.Schema({
        cv.Required(CONF_ID): cv.use_id(MPPTController),
        cv.Optional(CONF_CLEAR, default=True): cv.boolean,
        cv.Optional(CONF_REBOOT, default=False): cv.boolean,
    })
)
async def mppt_reset_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    cg.add(var.set_clear(config[CONF_CLEAR]))
    cg.add(var.set_reboot(config[CONF_REBOOT]))
    return var

@automation.register_action(
    "mppt_controller.set_charging_params",
    MPPTSetChargingParamsAction,
    cv.Schema({
        cv.Required(CONF_ID): cv.use_id(MPPTController),
        cv.Required(CONF_CHARGE_CURRENT): cv.int_range(min=0, max=255),
        cv.Required(CONF_BATTERY_TYPE): cv.int_range(min=0, max=255),
        cv.Required(CONF_CONST_VOLTAGE): cv.int_range(min=0, max=65535),
        cv.Required(CONF_LOAD_UNDERVOLTAGE): cv.int_range(min=0, max=65535),
    })
)
async def mppt_set_params_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    cg.add(var.set_charge_current(config[CONF_CHARGE_CURRENT]))
    cg.add(var.set_battery_type(config[CONF_BATTERY_TYPE]))
    cg.add(var.set_const_voltage(config[CONF_CONST_VOLTAGE]))
    cg.add(var.set_load_undervoltage(config[CONF_LOAD_UNDERVOLTAGE]))
    return var

async def to_code(config):
    pass
