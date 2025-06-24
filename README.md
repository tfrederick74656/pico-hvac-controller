# pico-hvac-controller

A rudimentary HVAC controller for the Raspberry Pi Pico 2W

## Description

This began as a project to supplement the lack of compressor runtime options on an Ecobee thermostat when in FCU mode (required for multi-speed fan control). It's grown to include additional handling for a freeze stat, and aims to eventually become a full-blown thermostat replacement.

## Getting Started

### Dependencies

* Software
    * Visual Studio Code
    * Raspberry Pi VS Code Extension
* Hardware
    * [Raspberry Pi Pico 2W (with Headers)](https://www.amazon.com/dp/B0DP54FWX1)
    * [Raspberry Pi Debug Probe](https://www.raspberrypi.com/products/debug-probe/)
    * [Freenove Breakout Board](https://store.freenove.com/products/fnk0081)
    * [2x 3.3V DC Relays](https://www.amazon.com/dp/B0D8PSX9WL)
    * [24V AC Relay Board](https://www.amazon.com/dp/B0CKYPH724)
    * [Set of Dupont Wires](https://www.amazon.com/dp/B01EV70C78)
    * [Breadboard](https://www.amazon.com/dp/B07PCJP9DY)
    * Soldering Gun
    * Thermostat Wire
    * Box of Wago Connectors

Note: The above hardware represents the author's specifc "as-built" configuration. Not all components are necessary or compatible with all HVAC configurations.

### Building

* If you're not familiar with Pico development, follow the [getting started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) first
* Clone this repo and open the directory in VS Code
    * The Raspberry Pi extension should automatically manage setting up the build directory
    * If you change the workspace folder name, you may need to update it in `CMakeLists.txt`
* Connect your Pico to the computer while holding the `BOOTSEL` buttom
* From the Raspberry Pi Pico Project menu, select `Run Project (USB)` to build and flash the image

### Using

Proper details on the hardware configuration are coming soon. In the meantime, the gist is there are two relay-driven inputs (thermostat cooling call and freeze stat) and two relay-driving outputs (compressor and fan). Hook them up to the appropriate GPIO pins and enjoy.

## Version History

* 1.3.1
    * Simplify freeze stat handling logic
* 1.2.1
    * Add support for delayed freeze stat handling
* 1.0.0
    * Initial Release

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Disclaimer

Working on HVAC equipment risks property damage, severe personal injury, or even death. HVAC equipment may contain exposed live electrical wiring, sharp and/or heavy moving components without safety interlocks, burning or freezing temperatures, and toxic chemical substances. If in doubt, please contact an experienced HVAC professional. The author of this tool takes no responsibility for your use of this software or related HVAC hardware.