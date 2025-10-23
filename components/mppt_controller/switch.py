import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID

from . import mppt_ns, MPPTController

CONF_LOAD_SWITCH = "load_switch"
CONF_MPPT_CONTROLLER = "mppt_controller"

# Create a separate switch class that controls the MPPT controller
MPPTLoadSwitch = mppt_ns.class_("MPPTLoadSwitch", switch.Switch)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MPPTController),
    cv.Optional(CONF_LOAD_SWITCH): switch.switch_schema(
        MPPTLoadSwitch,
        icon="mdi:power",
        entity_category="config"
    ).extend({
        cv.Required(CONF_MPPT_CONTROLLER): cv.use_id(MPPTController),
    }),
})

async def to_code(config):
    var = await cg.get_variable(config[CONF_ID])
    if CONF_LOAD_SWITCH in config:
        sw_conf = config[CONF_LOAD_SWITCH]
        sw = cg.new_Pvariable(sw_conf[CONF_ID])
        await switch.register_switch(sw, sw_conf)
        
        # Link the switch to the MPPT controller
        mppt_controller = await cg.get_variable(sw_conf[CONF_MPPT_CONTROLLER])
        cg.add(sw.set_controller(mppt_controller))
