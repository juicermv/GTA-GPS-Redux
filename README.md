# WARNING
This branch is purely experimental and **has not been tested**.
Compile at your own accord. Made possible by @jandlapuk.

# GTA SA GPS Reudx

This is a fork of the fixed GPS version by JuniorDjjr. It includes automatic navigation for mission markers.
If you'd like to contribute, go ahead.

    Warning: This mod is still in the testing phase, if you find any 
    bugs, please report them as issues on this github repo.

---

## Installation

1. Via modloader
    - Create a new folder inside your modloader folder. You can name it whatever you want.
    - Drop/extract all the files from the release archive you downloaded into that folder.

2. Via asi loader/scripts folder
    - Drop/extract all the files from the release archive you downloaded into your scripts folder.

## Default config:
`SA.GPS.CONF.ini`
```ini
; This applies to all GPS routes
[Navigation Config]

; Self explanatory. Can be any positive value.
Navigation line width = 2.5

; Self explanatory once again. 0/1
Enable navigation on bicycles = 0

; GTA V does not provide GPS routes for moving
; targets like cars and peds. If you'd like to
; enable navigation for these objects set this to 1. 0/1
Enable navigation for moving targets = 0

; This value sets how close you have to be before the
; navigation line stops showing. Also automatically
; removes waypoint. 
Navigation line removal proximity = 25.0


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

; Self explanatory. 0-255
Waypoint line opacity = 255

; These settings allow you to set custom colors
; for the various navigation paths in missions.
; Each color is represented by RGBA values separated
; by , .
[Custom Colors]

; Self explanatory. 0/1
Enabled = 0

; Used for enemies.
Red = 255, 0 , 0, 255

; Used for pickups and other things.
Green = 0, 255, 0, 255

; Used for friendlies.
Blue = 0, 0, 255, 255

; Not really used as far as I've seen.
White = 255, 255, 255, 255

; Used for destinations.
Yellow = 255, 255, 0

; These two aren't really used either.
Purple = 255, 0, 255, 255
Cyan = 0, 255, 255, 255
```
---
### This mod uses the following libraries:
* [inipp](https://github.com/mcmtroffaes/inipp)
* [plugin-sdk](https://github.com/DK22Pac/plugin-sdk)