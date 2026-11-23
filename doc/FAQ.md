# PIXELIX FAQ

Answers to common questions about configuring and using PIXELIX.

## Display

### Where do I change the LED matrix topology?

Set the matrix width and height and choose the topology that matches the
physical panel wiring in [`config/display.ini`](../config/display.ini). The
[NeoPixelBus layout guide](https://github.com/Makuna/NeoPixelBus/wiki/Layout-objects)
explains the available layouts.

### How do I change text properties?

Add text properties inside curly braces. Without braces, the keywords are
displayed as ordinary text.

| Keyword | Description |
| --- | --- |
| `{#RRGGBB}` | Set a solid text color (`RRGGBB` in hex). |
| `{lgv #RRGGBB,#RRGGBB,OFFSET,LENGTH}` | Set a vertical color gradient. |
| `{lgh #RRGGBB,#RRGGBB,OFFSET,LENGTH}` | Set a horizontal color gradient. |
| `{stc}` | Activate solid text color. |
| `{lgtc}` | Activate linear gradient text color. |
| `{hl}`, `{hc}`, `{hr}` | Align text left, center, or right. |
| `{vt}`, `{vc}`, `{vb}` | Align text top, center, or bottom. |
| `{0xCC}` | Insert a special character (`CC` in hex). |

When using these keywords through the
[REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.10.0), percent
encode unsafe ASCII characters. Keywords can be combined; do not add spaces
inside a keyword.

| Source text | URL-encoded text | Result |
| --- | --- | --- |
| `{hl}{#ff0000}Hi!` | `%7Bhl%7D%7B%23ff0000%7DHi!` | Left-aligned red text |
| `{hc}{#ff0000}Hi!` | `%7Bhc%7D%7B%23ff0000%7DHi!` | Centered red text |
| `{hr}{#ff0000}Hi!` | `%7Bhr%7D%7B%23ff0000%7DHi!` | Right-aligned red text |

### The display shows an error code such as "E4". What does it mean?

| Error code | Description |
| --- | --- |
| E1 | An unspecified fatal error occurred. |
| E2 | There is a problem with the two-wire (I2C) interface. |
| E3 | No user button is available. |
| E4 | The filesystem is invalid. Make sure the filesystem image was uploaded. |
| E5 | The display manager did not start. |
| E6 | The system message handler did not start. |
| E7 | A service failed to start. |

Press a button to open the PIXELIX Updater.

### How can I use animated icons?

Use animated GIF images. Sprite sheet support (before v8.0.0) was replaced by
GIF images. [Piskel](https://www.piskelapp.com/) is a free editor for animated
sprites and pixel art.

### How can I use alternative icons?

See the [alternative icons guide](./ICONS.md).

### Can I use a font with 8-pixel height?

Three font types are available: `default` (6 px), `normal` (6 px), and `large`
(8 px). The default is `normal`. Select the font type in each plugin instance's
display page. A plugin that cannot support the larger font in its layout falls
back to its default font.

### How do I configure the date and time format?

Date/time formats used by plugins such as DateTimePlugin and SunrisePlugin are
configured in the plugin's JSON file under `/configuration/<PLUGIN-UID>.json`.
The format specifiers follow
[`strftime()`](https://cplusplus.com/reference/ctime/strftime/).

Examples:

- `%I:%M %p`: 02:30 PM
- `%H:%M`: 14:30
- `%m/%d`: 11/12
- `%d.%m.`: 11.12.
- `%d - %b`: 11 - Nov

Text colors and alignment can be set using the [text properties](#how-do-i-change-text-properties).

### How do I rotate the display by 180 degrees?

Set `CONFIG_DISPLAY_ROTATE180` to `1` in [`config/display.ini`](../config/display.ini)
and rebuild:

```ini
[display:common]
build_flags =
    -D CONFIG_DISPLAY_ROTATE180=1
```

## Sensors

### How do I know whether a sensor is recognized?

1. Check the USB log output just after startup, before Wi-Fi connects.
2. Install SensorPlugin and open its web page. The sensor should appear in the
   sensor drop-down.

### Why does SensorPlugin sometimes show LDR values when no LDR is installed?

The LDR pin is configured as an analog input. Without an external pull-down
resistor, the pin can float and produce readings.

See the [supported sensors and wiring guide](./SENSORS.md) for sensor details.

## Configuration

### How do I configure my own list of plugins?

Available plugins depend on the board. A 4 MB flash board cannot fit every
plugin, and some plugins need more processing power than a single-core board
provides.

The feature profiles in [`config/`](../config/) define plugin sets:

- `configSmall.ini`: boards with 4 MB flash.
- `configNormal.ini`: boards with 8 MB flash or more.
- `configSmallNoI2s.ini`: boards with 4 MB flash and less MCU power.

Edit the profile used by your board, commenting plugins in or out as needed.
After changing the list, rebuild the firmware and filesystem.

[Back to the PIXELIX overview](../README.md)
