# esp8266-whr930-mqtt
Based on the work that Mosibi did for the WHR930 and the reverse engineering the guys at See Solutions did, but now running on an ESP8266 based chip instead of a Raspberry Pi!

* http://www.see-solutions.de/sonstiges/Protokollbeschreibung_ComfoAir.pdf
* https://github.com/Mosibi/whr_930

# Requirements
* esp8266-based chip (I used a NodeMCU v3)
* RS232-TTL converter (based on max3232)
  * connect to VU, G, TX and RX (on a NodeMCU v3)
* some dupont cables
* a converter from DB9 to RJ45 with the following pinout:
  * RJ45 2 (orange) -> DB9 2
  * RJ45 3 (green/white) -> DB9 3
  * RJ45 8 (brown) -> DB9 5

# lovelace card!
See https://github.com/timjong93/lovelace-wtw for a nice visualization of the data this module creates.

# Home Assistant Configuration
![Image](https://raw.githubusercontent.com/LukasdeBoer/esp8266-whr930-mqtt/master/homeassistant.png)
```
input_number:
  set_wtw_ventilation_level:
    name: Set ventilation level
    initial: 1
    min: 1
    max: 3
    step: 1
    mode: slider

input_number:
  set_wtw_temperature:
    name: Set temperature
    initial: 20
    min: 10
    max: 25
    step: 1
    mode: slider

automation:
  - alias: Ventilation level slider moved in GUI
    trigger:
      platform: state
      entity_id: input_number.set_wtw_ventilation_level
    action:
      service: mqtt.publish
      data_template:
        topic: house/ventilation/whr930/setventilation
        retain: false
        payload: "{{ states('input_number.set_wtw_ventilation_level') | int }}"
  - alias: Temperature level slider moved in GUI
    trigger:
      platform: state
      entity_id: input_number.set_wtw_temperature
    action:
      service: mqtt.publish
      data_template:
        topic: house/ventilation/whr930/settemperature
        retain: false
        payload: "{{ states('input_number.set_wtw_temperature') | int }}"

customize:
  sensor.wtw_intake_fan_speed:
    icon: mdi:fan
  sensor.wtw_exhaust_fan_speed:
    icon: mdi:fan

sensor:
  - platform: mqtt
    state_topic: "house/ventilation/whr930/filter_state"
    name: "WTW Filter Status"
    qos: 0
  - platform: mqtt
    state_topic: "house/ventilation/whr930/comfort_temp"
    name: "WTW Comfort Temperature"
    qos: 0
    unit_of_measurement: '°C'
  - platform: mqtt
    state_topic: "house/ventilation/whr930/outside_air_temp"
    name: "WTW Outside Temperature"
    qos: 0
    unit_of_measurement: '°C'
  - platform: mqtt
    state_topic: "house/ventilation/whr930/supply_air_temp"
    name: "WTW Supply Temperature"
    qos: 0
    unit_of_measurement: '°C'
  - platform: mqtt
    state_topic: "house/ventilation/whr930/return_air_temp"
    name: "WTW Return Temperature"
    qos: 0
    unit_of_measurement: '°C'
  - platform: mqtt
    state_topic: "house/ventilation/whr930/exhaust_air_temp"
    name: "WTW Exhaust Temperature"
    qos: 0
    unit_of_measurement: '°C'

  - platform: mqtt
    state_topic: "house/ventilation/whr930/return_air_level"
    name: "WTW Return Air Level"
    qos: 0
    unit_of_measurement: '%'
  - platform: mqtt
    state_topic: "house/ventilation/whr930/supply_air_level"
    name: "WTW Supply Air Level"
    qos: 0
    unit_of_measurement: '%'
  - platform: mqtt
    state_topic: "house/ventilation/whr930/ventilation_level"
    name: "WTW Ventilation Level"
    qos: 0
  - platform: mqtt
    state_topic: "house/ventilation/whr930/intake_fan_active"
    name: "WTW Intake Fan Active"
    qos: 0

  - platform: mqtt
    state_topic: "house/ventilation/whr930/intake_fan_speed"
    name: "WTW Intake Fan Speed"
    qos: 0
    unit_of_measurement: '%'
  - platform: mqtt
    state_topic: "house/ventilation/whr930/exhaust_fan_speed"
    name: "WTW Exhaust Fan Speed"
    qos: 0
    unit_of_measurement: '%'


  - platform: mqtt
    state_topic: "house/ventilation/whr930/valve_bypass_percentage"
    name: "WTW Bypass %"
    qos: 0
    unit_of_measurement: '%'
  - platform: mqtt
    state_topic: "house/ventilation/whr930/valve_preheating"
    name: "WTW Valve Preheating"
    qos: 0
  - platform: mqtt
    state_topic: "house/ventilation/whr930/bypass_motor_current"
    name: "WTW Bypass motor current"
    qos: 0
  - platform: mqtt
    state_topic: "house/ventilation/whr930/preheating_motor_current"
    name: "WTW Preheating motor current"
    qos: 0

  - platform: mqtt
    state_topic: "house/ventilation/whr930/bypass_factor"
    name: "WTW Bypass Factor"
    qos: 0
  - platform: mqtt
    state_topic: "house/ventilation/whr930/bypass_step"
    name: "WTW Bypass Step"
    qos: 0
  - platform: mqtt
    state_topic: "house/ventilation/whr930/bypass_correction"
    name: "WTW Bypass Correction"
    qos: 0
  - platform: mqtt
    state_topic: "house/ventilation/whr930/summermode"
    name: "WTW Summer mode"
    qos: 0

```

## New Home Assistant Configuration
Please note, starting from Home Assistant version 2022.12.0 the above configuration will not work anymore.
The MQTT items have now to be `mqtt` configuration. See the [documentation](https://www.home-assistant.io/integrations/binary_sensor.mqtt/#new_format), for more information.

```
input_number:
  set_wtw_ventilation_level:
    name: Set ventilation level
    initial: 1
    min: 1
    max: 3
    step: 1
    mode: slider

input_number:
  set_wtw_temperature:
    name: Set temperature
    initial: 20
    min: 10
    max: 25
    step: 1
    mode: slider

automation:
  - alias: Ventilation level slider moved in GUI
    trigger:
      platform: state
      entity_id: input_number.set_wtw_ventilation_level
    action:
      service: mqtt.publish
      data_template:
        topic: house/ventilation/whr930/setventilation
        retain: false
        payload: "{{ states('input_number.set_wtw_ventilation_level') | int }}"
  - alias: Temperature level slider moved in GUI
    trigger:
      platform: state
      entity_id: input_number.set_wtw_temperature
    action:
      service: mqtt.publish
      data_template:
        topic: house/ventilation/whr930/settemperature
        retain: false
        payload: "{{ states('input_number.set_wtw_temperature') | int }}"

customize:
  sensor.wtw_intake_fan_speed:
    icon: mdi:fan
  sensor.wtw_exhaust_fan_speed:
    icon: mdi:fan


mqtt:
  sensor:
    - state_topic: "house/ventilation/whr930/filter_state"
      name: "WTW Filter Status"
      qos: 0
    
    - state_topic: "house/ventilation/whr930/comfort_temp"
      name: "WTW Comfort Temperature"
      qos: 0
      unit_of_measurement: '°C'


    - state_topic: "house/ventilation/whr930/comfort_temp"
      name: "WTW Comfort Temperature"
      qos: 0
      unit_of_measurement: '°C'

    - state_topic: "house/ventilation/whr930/outside_air_temp"
      name: "WTW Outside Temperature"
      qos: 0
      unit_of_measurement: '°C'

    - state_topic: "house/ventilation/whr930/supply_air_temp"
      name: "WTW Supply Temperature"
      qos: 0
      unit_of_measurement: '°C'

    - state_topic: "house/ventilation/whr930/return_air_temp"
      name: "WTW Return Temperature"
      qos: 0
      unit_of_measurement: '°C'
 
    - state_topic: "house/ventilation/whr930/exhaust_air_temp"
      name: "WTW Exhaust Temperature"
      qos: 0
      unit_of_measurement: '°C'


    - state_topic: "house/ventilation/whr930/return_air_level"
      name: "WTW Return Air Level"
      qos: 0
      unit_of_measurement: '%'

    - state_topic: "house/ventilation/whr930/supply_air_level"
      name: "WTW Supply Air Level"
      qos: 0
      unit_of_measurement: '%'

    - state_topic: "house/ventilation/whr930/ventilation_level"
      name: "WTW Ventilation Level"
      qos: 0

    - state_topic: "house/ventilation/whr930/intake_fan_active"
      name: "WTW Intake Fan Active"
      qos: 0


    - state_topic: "house/ventilation/whr930/intake_fan_speed"
      name: "WTW Intake Fan Speed"
      qos: 0
      unit_of_measurement: '%'

    - state_topic: "house/ventilation/whr930/exhaust_fan_speed"
      name: "WTW Exhaust Fan Speed"
      qos: 0
      unit_of_measurement: '%'



    - state_topic: "house/ventilation/whr930/valve_bypass_percentage"
      name: "WTW Bypass %"
      qos: 0
      unit_of_measurement: '%'

    - state_topic: "house/ventilation/whr930/valve_preheating"
      name: "WTW Valve Preheating"
      qos: 0

    - state_topic: "house/ventilation/whr930/bypass_motor_current"
      name: "WTW Bypass motor current"
      qos: 0

    - state_topic: "house/ventilation/whr930/preheating_motor_current"
      name: "WTW Preheating motor current"
      qos: 0


    - state_topic: "house/ventilation/whr930/bypass_factor"
      name: "WTW Bypass Factor"
      qos: 0

    - state_topic: "house/ventilation/whr930/bypass_step"
      name: "WTW Bypass Step"
      qos: 0

    - state_topic: "house/ventilation/whr930/bypass_correction"
      name: "WTW Bypass Correction"
      qos: 0

    - state_topic: "house/ventilation/whr930/summermode"
      name: "WTW Summer mode"
      qos: 0

```