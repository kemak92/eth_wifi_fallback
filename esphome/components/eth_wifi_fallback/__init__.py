#
# This ESPHome component wraps around the repo by @kemak92:
# https://github.com/kemak92/eth_wifi_fallback
#
# Ethernet primary + WiFi Client fallback (same or different static IP).
# by @kemak92 - heungelectric, 2026
#
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_SSID,
    CONF_PASSWORD,
    CONF_STATIC_IP,
    CONF_GATEWAY,
    CONF_SUBNET,
    CONF_DNS1,
)
from esphome.components import ethernet

DEPENDENCIES = ["ethernet"]
AUTO_LOAD = []

eth_wifi_fallback_ns = cg.esphome_ns.namespace("eth_wifi_fallback")
EthWifiFallback = eth_wifi_fallback_ns.class_("EthWifiFallback", cg.Component)

CONF_CHECK_INTERVAL = "check_interval"
CONF_MANUAL_IP = "manual_ip"

MANUAL_IP_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_STATIC_IP): cv.ipv4address,
        cv.Required(CONF_GATEWAY): cv.ipv4address,
        cv.Required(CONF_SUBNET): cv.ipv4address,
        cv.Optional(CONF_DNS1): cv.ipv4address,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EthWifiFallback),
        cv.Required(CONF_SSID): cv.string,
        cv.Required(CONF_PASSWORD): cv.string,
        cv.Optional(CONF_CHECK_INTERVAL, default="15s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_MANUAL_IP): MANUAL_IP_SCHEMA,
    }
).extend(cv.COMPONENT_SCHEMA)


def ip_to_uint32(ip):
    """Convert IPv4Address to uint32_t (network byte order)."""
    parts = str(ip).split(".")
    return (int(parts[0]) << 24) | (int(parts[1]) << 16) | (int(parts[2]) << 8) | int(parts[3])


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_ssid(config[CONF_SSID]))
    cg.add(var.set_password(config[CONF_PASSWORD]))
    cg.add(var.set_check_interval(config[CONF_CHECK_INTERVAL]))

    if CONF_MANUAL_IP in config:
        manual = config[CONF_MANUAL_IP]
        dns = 0
        if CONF_DNS1 in manual:
            dns = ip_to_uint32(manual[CONF_DNS1])
        cg.add(
            var.set_manual_ip(
                ip_to_uint32(manual[CONF_STATIC_IP]),
                ip_to_uint32(manual[CONF_GATEWAY]),
                ip_to_uint32(manual[CONF_SUBNET]),
                dns,
            )
        )