# stm32f4-usbdevice
Implementing a USB Device in C++ on the STM32F4 Processor.

This project demonstrates how to operate the STM32F4 USB Hardware (Control and Bulk Endpoints) by implementing some UART-related applications.

There are two USB Configurations available:
- _Virtual COM Port_: Set the pre-processor macro `USB_INTERFACE_VCP`.
  - This will make the device behave as a standard _USB Character Device Class (CDC)_ device. This means the operating system's default driver will attach and a new serial port (e.g. _COMx_ on Windows, _/dev/cu.usbmodem..._ on macOS) will appear.
- _Vendor Defined Interface_: Set the pre-processor macro `USB_INTERFACE_VENDOR`.
  - This makes the device identify as a Vendor-defined interface, i.e. no standard driver will attach. This will allow you to easily write your own driver or application. See [hellousb](https://github.com/PhischDotOrg/hellousb) for an example.

There are two USB Applications implemented:
- _Loopback_: Set the pre-processor macro `USB_APPLICATION_LOOPBACK`.
  - In this configuration, all data received on a Bulk OUT endpoint will be sent back to the host on a Bulk IN endpoint.
- _UART_: Set the pre-processor macro `USB_APPLICATION_UART`.
  - In this configuration, all data received on a Bulk OUT endpoint will be sent out to a hardware UART. Please note that the reverse path is not implemented b/c this path is not implemented in the UART hardware driver.

The top-level [CMakeLists.txt](https://github.com/PhischDotOrg/stm32f4-usbdevice/blob/master/CMakeLists.txt) file shows how to set the mentioned pre-processor macros.

# How to check out
This project makes use of git's [submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules) feature. This repository is therefore more of a front-end for other code parts. In order to obtain all the needed code, please check out with the `--recursive` flag.

Example:

```
$ git clone git@github.com:PhischDotOrg/stm32f4-usbdevice.git --recursive
```

# How to build

## Pre-requisites
In order to build the code, you need these packages installed on your computer:
- [CMake](https://cmake.org/download/)
- [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
  - Other C++11 capable compilers might work, too, but I haven't tested them.
- [OpenOCD](http://openocd.org/getting-openocd/)
- _Optional_: [Doxygen](https://www.doxygen.nl/download.html)
  - If Doxygen is found, the `make doxygen` target will allow you to create the Doxygen documentation.

## Building in Visual Studio Code
The easiest way to build this project is to load the [Visual Studio Code](https://code.visualstudio.com) Workspace:

[https://github.com/PhischDotOrg/stm32f4-usbdevice/blob/master/stm32f4-usbdevice.code-workspace](https://github.com/PhischDotOrg/stm32f4-usbdevice/blob/master/stm32f4-usbdevice.code-workspace)

The Workspace should set up the CMake Kits to offer _Generic STM32F4_ and _Generic STM32F4 (Windows)_. Please chose the one that is suitable for you.

## Building via Command Line
The code can be built via Command Line, e.g. like this:

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=../common/Generic_STM32F4.ctools -DSTM32F4_BOARD=STM32F4_Discovery ..
$ make
```

_Please note:_ CMake apparently uses [Ninja](https://ninja-build.org) as the default Build Tool on some platforms (e.g. recent Ubuntu). In that case, the `make` command should be `ninja` instead.

# How to flash
The CMake build files include a `flash` target which uses OpenOCD to flash the binary to the controller. This is probably the most convenient way to flash:

```
$ make flash
```

# How to test
If you need to test whether your setup works, I suggest to build with `USB_INTERFACE_VCP` and `USB_APPLICATION_LOOPBACK`.

After flashing, you should see a new virtual COM port on your system. All data that you send to the port should be echo'd back:

```
$ cu -l /dev/cu.usbmodem1432101
Connected.
Hallo, Welt.
```

On Windows, you can use [PuTTY](https://www.putty.org) to access the serial port.

# How to use (in your own application)

If you need a different application than simply looping back received packets, you can
- Derive and extend the class `UsbInterface` to implement your own Control Requests.
- Derive and extend the class `UsbBulkOutApplication` to implement your own handler for Bulk OUT requests.

# Build Variants
The Workspace will also allow you to select a few variants:
- _Build Type_: This sets up the [CMAKE_BUILD_TYPE](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html) variable which is evaluated in the [common/CMakeLists.txt](https://github.com/PhischDotOrg/stm32f4-common/blob/master/CMakeLists.txt) file.
  - _Debug_: Compile with `-O0 -g`.
  - _Release_: Compile with `-O3`.
  - _RelWithDebInfo_: Compile with `-O2 -Og -g`.
  - _MinSizeRel_: Compile with `-Os -g`.
- _Board Type_: This sets up the `STM32F4_BOARD` variable which is also evaluated in the [common/CMakeLists.txt](https://github.com/PhischDotOrg/stm32f4-common/blob/master/CMakeLists.txt) file.
  - _STM32F4 Discovery_: Sets `STM32F4_BOARD=STM32F4_Discovery` to build for the [STM32F4 Discovery Board](https://www.st.com/en/evaluation-tools/stm32f4discovery.html).
  - _STM32F4 Nucleo F411RE_: Sets `STM32F4_BOARD=STM32F4_Nucleo_F411RE` to build for the [STM32F4 Nucleo F411RE Board](https://www.st.com/en/evaluation-tools/nucleo-f411re.html).
  
# Build Targets
- _all_: Default pseudo-target building all executables, most notably `firmware.elf`
- _flash_: Build the binary and flash it to the controller using OpenOCD.
- _doxygen_: Build the Doxygen documentation for the C++ Code.
- _bin_: Build a `.bin`, `.s19`and `.hex` file from the `.elf` file. Also generate an ASCII dump from the `.bin` file.
  - This may be useful if you need to flash using other tools. The ASCII dump may come in handy if you need to compare binary changes between two builds.

# Limitations
I have not tested this much on the STM32F4 Nucleo Board, so this might not work at all.
