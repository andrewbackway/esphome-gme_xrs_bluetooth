XRS Radio ESPHome External Component
====================================

Folder layout
-------------

components/xrs_radio/
  xrs_radio.h
  xrs_radio.cpp
  at_parser.h
  at_parser.cpp

  sensor/
    xrs_sensor.h
    xrs_sensor.cpp
    __init__.py

  binary_sensor/
    xrs_binary_sensor.h
    xrs_binary_sensor.cpp
    __init__.py

  switch/
    xrs_switch.h
    xrs_switch.cpp
    __init__.py

  number/
    xrs_number.h
    xrs_number.cpp
    __init__.py

  select/
    xrs_select.h
    xrs_select.cpp
    __init__.py

  text_sensor/
    xrs_text_sensor.h
    xrs_text_sensor.cpp
    __init__.py

  __init__.py

Usage
-----

external_components:
  - source:
      type: local
      path: ./components
    components:
      - xrs_radio

xrs_radio:
  id: xrs1
  mac_address: "34:81:F4:12:34:56"
  latitude_sensor: ext_lat
  longitude_sensor: ext_lon
  location_interval: 60s

sensor:
  - platform: xrs_radio
    xrs_id: xrs1
    type: channel
    name: "XRS Channel"

  - platform: xrs_radio
    xrs_id: xrs1
    type: zone
    name: "XRS Zone"

  - platform: xrs_radio
    xrs_id: xrs1
    type: volume
    name: "XRS Volume"

  - platform: xrs_radio
    xrs_id: xrs1
    type: ptt_timer
    name: "XRS PTT Timer"

binary_sensor:
  - platform: xrs_radio
    xrs_id: xrs1
    type: connected
    name: "XRS Connected"

  - platform: xrs_radio
    xrs_id: xrs1
    type: ptt_active
    name: "XRS PTT Active"

  - platform: xrs_radio
    xrs_id: xrs1
    type: ptt_data
    name: "XRS PTT Data"

  - platform: xrs_radio
    xrs_id: xrs1
    type: power_low
    name: "XRS Power Low"

  - platform: xrs_radio
    xrs_id: xrs1
    type: scanning
    name: "XRS Scanning"

  - platform: xrs_radio
    xrs_id: xrs1
    type: duplex_enabled
    name: "XRS Duplex Enabled"

  - platform: xrs_radio
    xrs_id: xrs1
    type: silent_memory
    name: "XRS Silent Memory"

  - platform: xrs_radio
    xrs_id: xrs1
    type: quiet_memory
    name: "XRS Quiet Memory"

  - platform: xrs_radio
    xrs_id: xrs1
    type: quiet_mode
    name: "XRS Quiet Mode"

text_sensor:
  - platform: xrs_radio
    xrs_id: xrs1
    type: manufacturer
    name: "XRS Manufacturer"

  - platform: xrs_radio
    xrs_id: xrs1
    type: model
    name: "XRS Model"

  - platform: xrs_radio
    xrs_id: xrs1
    type: firmware
    name: "XRS Firmware"

  - platform: xrs_radio
    xrs_id: xrs1
    type: serial
    name: "XRS Serial"

  - platform: xrs_radio
    xrs_id: xrs1
    type: last_message
    name: "XRS Last Message"

  - platform: xrs_radio
    xrs_id: xrs1
    type: power_state
    name: "XRS Power State"

  - platform: xrs_radio
    xrs_id: xrs1
    type: ptt_state
    name: "XRS PTT State"

  - platform: xrs_radio
    xrs_id: xrs1
    type: channel_label
    name: "XRS Channel Label"

number:
  - platform: xrs_radio
    xrs_id: xrs1
    type: volume
    name: "XRS Volume Control"
    min_value: 0
    max_value: 31
    step: 1

switch:
  - platform: xrs_radio
    xrs_id: xrs1
    type: location_mode
    name: "XRS Location Mode"

  - platform: xrs_radio
    xrs_id: xrs1
    type: scan
    name: "XRS Scan"

  - platform: xrs_radio
    xrs_id: xrs1
    type: duplex
    name: "XRS Duplex"

  - platform: xrs_radio
    xrs_id: xrs1
    type: quiet_mode
    name: "XRS Quiet Mode"

  - platform: xrs_radio
    xrs_id: xrs1
    type: quiet_memory
    name: "XRS Quiet Memory"

  - platform: xrs_radio
    xrs_id: xrs1
    type: silent_memory
    name: "XRS Silent Memory"

select:
  - platform: xrs_radio
    xrs_id: xrs1
    type: zone
    name: "XRS Zone Select"

  - platform: xrs_radio
    xrs_id: xrs1
    type: channel
    name: "XRS Channel Select"
