# UV-KX Open Radio Firmware

## ..... Work in PROGRESS - not for everyday use

> [!WARNING]  
> To test this version you must bear in mind that it can corrupt the data in the EEPROM, so you must make a backup before using this version.
>
> **If you want to have a more stable FW and even with more features, I advise you to look at other options.**

> [!WARNING]  
> Use this firmware at your own risk (entirely). There is absolutely no guarantee that it will work in any way shape or form on your radio(s), it may even brick your radio(s), in which case, you'd need to buy another radio.
>

## Features
- Customizable operating modes and configurations.
- Support for multiple radio models.
- Real-time signal monitoring.
- [ToDo] Additional detailed feature list.

## Installation
- **Requirements:**
  - A working C/C++ toolchain (GCC recommended).
  - Make utility installed.
  - A compatible radio device.
- **Steps:**
  1. Clone the repository:
  
         git clone https://github.com/joaquimorg/UV-KX.git
  
  2. Navigate to the project directory:
  
         cd UV-KX
         
  3. Ensure you have installed the necessary dependencies (libraries, headers, etc.).  
     [ToDo: List any extra dependencies]
     
  4. [Optional] Set up your environment variables if required.
  
  5. Backup your existing EEPROM data before proceeding.

## Building

- To compile the firmware, simply run:

         make
         
- To program the radio via a specific COM port, run:

         make prog COMPORT=com3

## Radio

<img src="images/uv-k5-screenshot_home.png" alt="Welcome" width="400" />
<img src="images/uv-k5-screenshot_vfo.png" alt="VFO" width="400" />
<img src="images/uv-k5-screenshot_vfo_rx.png" alt="VFO RX" width="400" />
<img src="images/uv-k5-screenshot_menu.png" alt="Menu" width="400" />
<img src="images/uv-k5-screenshot_set_vfoa_1.png" alt="VFO Menu" width="400" />
<img src="images/uv-k5-screenshot_set_vfoa_2.png" alt="VFO Menu" width="400" />

## Open implementation of the Quansheng UV-K5/K6/5R firmware

Based on the work of DualTachyon's open firmware found [here](https://github.com/DualTachyon/uv-k5-firmware) ... a cool achievement !

## Credits

Many thanks to various people:
- [egzumer](https://github.com/egzumer/uv-k5-firmware-custom)
- [DualTachyon](https://github.com/DualTachyon)
- [Mikhail](https://github.com/fagci)
- [@Matoz](https://github.com/spm81)
- [OneOfEleven](https://github.com/OneOfEleven)
- [ijv](https://github.com/INDIAJULIETVICTOR)
- and others...

## Other sources of information

- [ludwich66 - Quansheng UV-K5 Wiki](https://github.com/ludwich66/Quansheng_UV-K5_Wiki/wiki)<br>
- [amnemonic - tools and sources of information](https://github.com/amnemonic/Quansheng_UV-K5_Firmware)

## License

    Copyright 2024 joaquim.org
    https://github.com/joaquimorg

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

