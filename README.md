```markdown
# eth_wifi_fallback

ESPHome external component: **Ethernet primary + WiFi Client fallback**.

Repository: https://github.com/kemak92/eth_wifi_fallback

by **@kemak92** — heungelectric, 2026

Tested on **HEUNGELECTRIC** ESP32 + LAN8720 board.

---

## Features

- Ethernet as primary connection
- Automatic WiFi Client (STA) when Ethernet is lost
- Optional static IP for WiFi (can be the same as Ethernet)
- Stops WiFi when Ethernet recovers

---

## Usage
[![Donate với PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.me/kemak92)
```yaml
external_components:
  - source: github://kemak92/eth_wifi_fallback@main
    components: [eth_wifi_fallback]
    refresh: 0s

ethernet:
  type: LAN8720
  mdc_pin: GPIO23
  mdio_pin: GPIO18
  clk:
    pin: GPIO17
    mode: CLK_OUT
  phy_addr: 0
  manual_ip:
    static_ip: 192.168.1.150
    gateway: 192.168.1.1
    subnet: 255.255.255.0
    dns1: 192.168.1.1

eth_wifi_fallback:
  ssid: "YourSSID"
  password: "YourPassword"
  check_interval: 15s
  manual_ip:
    static_ip: 192.168.1.150
    gateway: 192.168.1.1
    subnet: 255.255.255.0
    dns1: 192.168.1.1
```

---

## Options

| Option | Required | Default | Description |
|--------|----------|---------|-------------|
| `ssid` | yes | — | WiFi SSID |
| `password` | yes | — | WiFi password |
| `check_interval` | no | `15s` | How often to check Ethernet status |
| `manual_ip.static_ip` | no | DHCP | Fixed IP when on WiFi |
| `manual_ip.gateway` | no | — | Gateway |
| `manual_ip.subnet` | no | — | Subnet mask |
| `manual_ip.dns1` | no | — | DNS server |

---

## Example logs (HEUNGELECTRIC board)

Ethernet up → lost → WiFi fallback (same IP) → Ethernet recovered:

```text
[I][eth_wifi_fallback:011]: EthWifiFallback ready (check every 15000 ms)
[I][app:117]: setup() finished successfully!
[I][ethernet:088]: Starting connection
[I][ethernet:099]: Connected

[W][ethernet:120]: Connection lost; reconnecting
[W][eth_wifi_fallback:029]: Ethernet lost → starting WiFi Client
[I][eth_wifi_fallback:118]: WiFi using static IP
[I][eth_wifi_fallback:136]: WiFi start requested (SSID: WIFI)
[I][eth_wifi_fallback:038]: WiFi connected successfully
[I][eth_wifi_fallback:076]: WiFi IP: 192.168.1.150
[I][eth_wifi_fallback:077]: Gateway: 192.168.1.1
[I][eth_wifi_fallback:078]: Netmask: 255.255.255.0

[I][ethernet:099]: Connected
[I][eth_wifi_fallback:050]: Ethernet recovered → stopping WiFi
[I][eth_wifi_fallback:143]: WiFi stopped
```

---

## Notes

- Do **not** declare the official `wifi:` component together with `ethernet:`.
- This component starts WiFi at low level only when Ethernet is down.
- Using the same IP for Ethernet and WiFi is supported; brief ping loss can occur during switch (ARP update).
- Warning `took a long time for an operation` on WiFi start is normal (blocking init ~100 ms).
- Tested with ESPHome 2026.7.x / ESP32 + LAN8720 (HEUNGELECTRIC board).

---

## License

Use freely. Attribution appreciated.
