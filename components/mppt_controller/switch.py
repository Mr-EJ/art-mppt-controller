import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID

from . import mppt_ns, MPPTController

CONF_LOAD_SWITCH = "load_switch"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MPPTController),
    cv.Optional(CONF_LOAD_SWITCH): switch.switch_schema(
        icon="mdi:power",
        entity_category="config"
    ),
})

async def to_code(config):
    var = await cg.get_variable(config[CONF_ID])
    if CONF_LOAD_SWITCH in config:
        sw_conf = config[CONF_LOAD_SWITCH]
        sw = await switch.new_switch(sw_conf)
        cg.add(var.set_load_switch(sw))
