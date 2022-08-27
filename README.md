# GTA SA GPS Reudx

This is a fork of the fixed GPS version by JuniorDjjr. It includes automatic navigation for mission markers.
If you'd like to contribute, go ahead.


    Warning: This mod is still in the testing phase, if you find any 
    bugs, please report them as issues on this github repo.

---

## Default config:
`SA.GPS.CONF.ini`
```ini
; This applies to all GPS routes
[Navigation Config]

; Self explanatory. Can be any value technically.
Navigation line width = 4.0

; Also self explanatory. 0-255
Navigation line opacity = 255

; Self explanatory once again. 0/1
Enable navigation on bicycles = 0


; These settings are specific to the GPS
; route following your waypoint (the manual
; target set by the player). 
[Waypoint Config]

; R color value. 0-255
Waypoint line red = 180

; G color value. 0-255
Waypoint line green = 24
 
; B color value. 0-255
Waypoint line blue = 24

; How close you need to get before the waypoint automatically disappears.
; Can be any positive value.
Waypoint removal distance = 10.0
```
---
### This mod uses the following libraries:
* [inipp](https://github.com/mcmtroffaes/inipp)
* [plugin-sdk](https://github.com/DK22Pac/plugin-sdk)