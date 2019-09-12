# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# BOM

| Amount | Description | Package | Part | URL |
| --- | --- | --- | --- | --- |
| 2 | Capacitor 10 pF | C1206 | C3, C4 | [Reichelt](https://www.reichelt.de/smd-vielschicht-keramikkondensator-10p-5-npo-g1206-10p-p31885.html?&trstct=pol_2) |
| 3 | Capacitor 100 nF | C1206 | C1, C5, C6 | [Reichelt](https://www.reichelt.de/vielschicht-kerko-100nf-50v-125-c-kem-x7r1206b100n-p207152.html?) |
| 1 | Capacitor 1000 uF / 16 V | E5-8.5 | C2 | [Reichelt](https://www.reichelt.de/elko-radial-1-0-mf-16-v-105-c-low-esr-rad-fr-1-000-16-p121253.html?) |
| 4 | Resistor 100 Ohm | R1206 | R6, R7, R8, R9 | [Reichelt](https://www.reichelt.de/smd-widerstand-1206-100-ohm-250-mw-5-smd-1-4w-100-p18242.html?r=1) |
| 1 | Resistor 1 kOhm | R1206 | R4 | [Reichelt](https://www.reichelt.de/smd-widerstand-1206-1-0-kohm-250-mw-5-smd-1-4w-1-0k-p18221.html?r=1)  |
| 2 | Resistor 10 kOhm | R1206 | R2, R3 | [Reichelt](https://www.reichelt.de/smd-widerstand-1206-10-kohm-250-mw-5-smd-1-4w-10k-p18244.html?r=1) |
| 1 | Resistor 100 kOhm | R1206 | R1 | [Reichelt](https://www.reichelt.de/smd-widerstand-1206-100-kohm-250-mw-1-koa-rk73h2bttd14-p238058.html?) |
| 1 | Resistor 8.2 MOhm | R1206 | R5 | [Reichelt](https://www.reichelt.de/smd-widerstand-1206-8-2-mohm-250-mw-5-smd-1-4w-8-2m-p18377.html?)  |
| 1 | Harting connector | ML10 | CON1 | [Reichelt](https://www.reichelt.de/wannenstecker-10-polig-gerade-wsl-10g-p22816.html?)  |
| 1 | ESD suppressor CDSOD323-T05C | SOD-323 | D1 | [Conrad](https://www.conrad.de/de/p/bourns-tvs-diode-cdsod323-t05c-sod-323-6-v-350-w-1055444.html?searchType=SearchRedirect) |
| 1 | Micro fuse block | OMNIBLOCK | F1 | [Reichelt](https://www.reichelt.de/smd-sicherung-mit-halter-superflink-4-0a-smd-hasf-4-0a-p23933.html?)  |
| 1 | ESP32-DEVKIT V1 | - | ESP32-DEVKITV1 | [Ebay](https://www.ebay.de/itm/Espressif-ESP32-WLAN-Dev-Kit-Board-Development-Bluetooth-Wifi-v1-WROOM32-NodeMCU/253059783728?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649) |
| 1 | N-channel mosfet BSS138 | SOT23 | Q1 | [Reichelt](https://www.reichelt.de/mosfet-n-ch-50v-0-22a-0-36w-sot-23-bss-138-smd-p41437.html?)  |
| 1 | 32.768 kHz crystal | TC26V | Q2 | [Reichelt](https://www.reichelt.de/standardquarz-grundton-32-768-khz-iqd-lfxtal014219-p245417.html?)  |
| 1 | P-channel mosfet TSM2323CX | SOT96P280X 130-3N | U1 | [Reichelt](https://www.reichelt.de/mosfet-p-ch-20v-4-7a-0-039r-sot23-tsm2323cx-p254945.html?)  |
| 5 | Wago clamp 2 | W237-102 | X1, X2, X3, X4, X5, X6 | [Reichelt](https://www.reichelt.de/thr-leiterplattenklemme-rm-4-mm-2-pol-sw-wago-2060-1422-p163238.html?)  |
| 1 | Wago clamp 1 | W237-103 | X2 | [Reichelt](https://www.reichelt.de/thr-leiterplattenklemme-rm-4-mm-1-pol-sw-wago-2060-1421-p163237.html?&trstct=lsbght_sldr::163234) |
| 1 | WS2812B 5050 8x32 RGB Flexible LED Panel Matrix | - | - | [Ebay](https://www.ebay.de/itm/WS2812B-5050-8x32-RGB-Flexible-LED-Panel-Matrix-Programmable-Display-Screen-DE/223166536588?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649) |

Note, X2 is a composition of Wago clamp 1 and 2.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
