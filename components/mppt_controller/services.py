import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.automation as automation
from . import mppt_ns, MPPTController

MPPTResetAction = mppt_ns.class_("MPPTResetAction", automation.Action)
MPPTSetChargingParamsAction = mppt_ns.class_("MPPTSetChargingParamsAction", automation.Action)

RESET_ENERGY_SCHEMA = cv.Schema({
    cv.Optional("clear", default=True): cv.boolean,
    cv.Optional("reboot", default=False): cv.boolean,
})

SET_CHARGING_SCHEMA = cv.Schema({
    cv.Required("charge_current"): cv.int_,
    cv.Required("battery_type"): cv.int_,
    cv.Required("const_voltage"): cv.int_,
    cv.Required("load_undervoltage"): cv.int_,
})

@automation.register_action(
    "mppt_controller.reset_energy",
    MPPTResetAction,
    RESET_ENERGY_SCHEMA
)
async def mppt_reset_action(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id)
    cg.add(var.set_clear(config["clear"]))
    cg.add(var.set_reboot(config["reboot"]))
    return var

@automation.register_action(
    "mppt_controller.set_charging_params",
    MPPTSetChargingParamsAction,
    SET_CHARGING_SCHEMA
)
async def mppt_charge_action(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id)
    cg.add(var.set_charge_current(config["charge_current"]))
    cg.add(var.set_battery_type(config["battery_type"]))
    cg.add(var.set_const_voltage(config["const_voltage"]))
    cg.add(var.set_load_undervoltage(config["load_undervoltage"]))
    return var
    
async def to_code(config):
    # This function exists only so __init__.py can safely await it
    pass
