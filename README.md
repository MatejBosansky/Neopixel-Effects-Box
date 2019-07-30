# Neopixel Effects Box
Control predefined effects of your attached neopixel(ws2812b) strip. To get brief overview how it looks like and what do you need, check:
- [Wiring scheme](https://github.com/MatejBosansky/Neopixel-Effects-Box/blob/master/docs/Scheme_bb.pdf).
- Video:
[![Youtube Video](http://img.youtube.com/vi/JeZSuh0Q6hs/0.jpg)](http://www.youtube.com/watch?v=JeZSuh0Q6hs "Youtube Video")

## Features
- Power supply can be provided via any standard DC adapter like laptop charger with input rating 4-32V. 
- Configurable length of neopixel strip.
- Predefined 4 effects including sound spectrum visualizer based on your audio input.
- Control current effect and its properties with rotary encoder and LCD display.
- Possible to add your another effects - check notes


## Hardware
I prepared shopping list from 2 aliexpress sellers to not overload your local post service.
Additionally to soldering iron, AC to DC adapter and neopixel strip you will need:

Basic components - This components you need.
- [Wemos D1 Mini ESP8266 board](http://www.aliexpress.com/item/32674463823.html).
- [DC-DC Step down converter with proper current rating](https://www.aliexpress.com/item/32821840536.html). Output DC must be adjustable to 5V.
- [ST7735S TFT LCD Display module](http://www.aliexpress.com/item/2055099048.html).
- [Rotary encoder](http://www.aliexpress.com/item/32224563961.html).
- [Mini digital voltmeter](http://www.aliexpress.com/item/32813014220.html).
- [Logic level converter](http://www.aliexpress.com/item/32216841860.html).
- [Audio female 3.5 connector](http://s.click.aliexpress.com/e/cAcySBSY).
- 1000uF +6.3V capacitor.
- 2x 1k Ohm resistor.

This components can be different if you design your own wiring and box.
- [Cable terminal](http://www.aliexpress.com/item/32815936999.html).
- [DC female connector](http://www.aliexpress.com/item/32829667875.html). Choose correct one based on your adapters male connector.
- [Switch](http://s.click.aliexpress.com/e/cFFxSPNA). Must be capable to handle max input current for strip.
- [Prototyping board](http://www.aliexpress.com/item/32224138773.html). 
- Fuse with proper rating.
- Wires.
- 3D printed BOX - check model in repository. This model is designed for components suggested in description.

## Notes
- Take in mind, that one ws2812b pixel can draw 60mA when is fully loaded (brightest white color). Multiply this value by count of your pixels and you have max possible power rating for your components like DC-DC converter, AC-DC adapter, wires, fuse, button.... But in reality real peak consuption for default effects is max 1/4 of this value.
- Recommending in code to define around 70% of max brightness - no too much noticable difference in brightness but significantly less power consuption.
- Code has been initialy developed in original Arduino IDE divided into multiple *ino files. In order to make quick migration to PlatformIO I needed to merge all files content into one main.cpp file.
- If you want to add your own effect, search `//**` code marks to go through guide inside the code.
- Check photos in Docs folder to see parts layout on prototype board.
- Must have project for every makers party!
