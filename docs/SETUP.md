# Configuring a Development Environment {#setup}

## General Notes

It is strongly recommend that you follow the instructions on this page to set up your development environment, including the ESP-IDF. It is also possible to follow [Espressif's instructions to install ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v5.2.5/esp32s2/get-started/index.html#installation) through a standalone installer or an IDE. This can be done if you're sure you know what you're doing or the process written here doesn't work anymore.

It is recommended to use native tools (i.e. Windows programs on Windows), not Windows Subsystem for Linux (WSL) or a virtual machine.

Espressif's installation guide notes limitations for the ESP-IDF's path:
> The installation path of ESP-IDF and ESP-IDF Tools must not be longer than 90 characters.
>
> The installation path of Python or ESP-IDF must not contain white spaces or parentheses.
>
> The installation path of Python or ESP-IDF should not contain special characters (non-ASCII) unless the operating system is configured with "Unicode UTF-8" support.

If the path to your home directory has spaces in it, then installation paths should be changed to something without a space, like `c:\esp\`. Also note that `ccache` uses a temporary directory in your home directory, and spaces in that path cause issues. `ccache` is enabled by default when running `export.ps1`, but it can be disabled by removing the following from `esp-idf/tools/tools.json`:
```
"export_vars": {
  "IDF_CCACHE_ENABLE": "1"
},
```

## Configuring a Windows Environment

The continuous integration for this project runs on a Windows instance. This means one can read [build-firmware-and-emulator.yml](https://github.com/AEFeinstein/Super-2024-Swadge-FW/blob/main/.github/workflows/build-firmware-and-emulator.yml) to see how the Windows build environment is set up from scratch for both the firmware and emulator, though it does not install extra development tools. It is recommend to follow the following guide.

1. [Install `git`](https://git-scm.com/download/win). This is for version control.
2. [Install `python`](https://www.python.org/downloads/). This is for a few utilities. Make sure to check "Add Python to environment variables" when installing.
    * Once python is installed, _before_ setting up the IDF, install `esptool`. We've seen issues when running the IDF's `esptool` independently, but the version in the The Python Package Index seems to work fine. If you've already set up an environment and need to install `esptool`, make sure to do so in a terminal where you **have not** run `export.ps1`, which sets up IDF environment variables.
    ```bash
    python -m pip install esptool
    ```
3. [Install `msys2`](https://www.msys2.org/). This is the environment in which the emulator will be built.
4. Start an `msys2` shell and run the following command to install all required packages for building the emulator:
    ```bash
    pacman --noconfirm -S base-devel gcc gdb zip mingw-w64-x86_64-graphviz mingw-w64-x86_64-cppcheck doxygen
    ```
5. [Install `LLVM-17.0.6-win64.exe`](https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.6). This is for the `clang-format-17` tool. During the install, when it asks to add LLVM to the system PATH, add it to the path for all users.
6. Add the following paths to the Windows path variable. [Here are some instructions on how to do that](https://www.architectryan.com/2018/03/17/add-to-the-path-on-windows-10/).
    * `C:\msys64\mingw64\bin`
    * `C:\msys64\usr\bin`

    You must add the `msys2` paths **after** the `python` paths and **before** `C:\Windows\System32`. This is because the build uses Windows `python`, not msys2's, and it uses msys2 `find.exe`, not System32's. When it's all set up, it should look something like this:
    
    <img src="./win_path.png">

    If the above doesn't work try adding `C:\msys64\mingw64\bin` and `C:\msys64\usr\bin` to the system path instead of the user path. The System path is highlighted in red, and the user path in purple. 

    <img src="./Correct_path.jpg">

6. Clone the ESP-IDF v5.2.5 and install the tools. Note that it will clone into `$HOME/esp/esp-idf`.
Note: Some installs of Python will have py.exe instead of python.exe - If this is the case, you can edit install.ps1 to replace all instances of python.exe to py.exe OR rename your locally installed py.exe file to python.exe
    ```powershell
    & Set-ExecutionPolicy -Scope CurrentUser Unrestricted
    & git clone -b v5.2.5 --recurse-submodules https://github.com/espressif/esp-idf.git $HOME/esp/esp-idf
    & $HOME\esp\esp-idf\install.ps1
    ```
    > **Warning**
    >
    > Sometimes `install.ps1` can be a bit finicky and not install everything it's supposed to. If it doesn't create a `$HOME/.espressif/python_env` folder, try running a few more times. As a last resort you can try editing `install.ps1` and swap the `"Setting up Python environment"` and `"Installing ESP-IDF tools"` sections to set up the Python environment first.

## Configuring a Linux Environment

1. Run the following commands, depending on your package manager, to install all necessary packages:
    * `apt`:
        ```bash
        sudo apt install build-essential xorg-dev libx11-dev libxinerama-dev libxext-dev mesa-common-dev libglu1-mesa-dev libasound2-dev libpulse-dev git libasan8 cppcheck python3 python3-pip python3-venv cmake libusb-1.0-0-dev lcov gdb graphviz
        ```
    * `dnf`:
        ```bash
        sudo dnf group install "C Development Tools and Libraries" "Development Tools"
        sudo dnf install libX11-devel libXinerama-devel libXext-devel mesa-libGLU-devel alsa-lib-devel pulseaudio-libs-devel libudev-devel cmake libasan cppcheck python3 python3-pip python3-virtualenv cmake libusb1-devel lcov gdb graphviz git doxygen clang clang-tools-extra libasan-static libubsan-static
        sudo dnf install libX11-devel libXinerama-devel libXext-devel mesa-libGLU-devel alsa-lib-devel pulseaudio-libs-devel libudev-devel cmake libasan8 cppcheck python3 python3-pip python3-venv cmake libusb-1.0-0-dev lcov gdb graphviz git
        ```
2. Install `doxygen` separately from their website (https://www.doxygen.nl/download.html). Note that the version used in this project is currently 1.10.0 and the version in many package managers is less than that. You will need to extract the binary somewhere and add it to your `PATH` variable. For example, GitHub Actions installs `doxygen` like this:
    ```bash
    wget -q -P ~ https://www.doxygen.nl/files/doxygen-1.13.2.linux.bin.tar.gz
    tar -xf ~/doxygen-1.13.2.linux.bin.tar.gz -C ~

    # This will temporarily add doxygen to the PATH.
    # To do this permanently, add this line to the bottom of your ~/.bashrc file
    export PATH="$PATH:$HOME/doxygen-1.13.2/bin"
    ```
    It is recommended that you uninstall any prior doxygen versions as well:
    ```bash
    sudo apt remove doxygen
    ```
3. Install `clang-17` and `clang-format-17` separately. Note that the version used in this project is currently 17 and the version in many package managers is less than that.
    ```bash
    wget https://apt.llvm.org/llvm.sh
    chmod u+x llvm.sh
    sudo ./llvm.sh 17
    sudo apt install clang-format-17
    ```
4. Clone the ESP-IDF v5.2.5 and install the tools. Note that it will clone into `~/esp/esp-idf`.
    ```bash
    git clone -b v5.2.5 --recurse-submodules https://github.com/espressif/esp-idf.git ~/esp/esp-idf
    ~/esp/esp-idf/install.sh
    ```

## Configuring a MacOS Environment

> **Warning**
>
> This section is still under development, and as a result, may have unexpected errors in its process.

1. Install [Homebrew](https://brew.sh/)
2. Install [XQuartz](https://www.xquartz.org/)
3. Install the [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) extension in VS Code
4. Run the following command to install all necessary dependencies:
    ```bash
    brew install libxinerama pulseaudio clang-format cppcheck wget doxygen cmake graphviz
    ```
5. Clone the ESP-IDF v5.2.5 and install the tools. Note that it will clone into `~/esp/esp-idf`.
    ```bash
    git clone -b v5.2.5 --recurse-submodules https://github.com/espressif/esp-idf.git ~/esp/esp-idf
    ~/esp/esp-idf/install.sh
    ```
6. Before running the simulator on your machine, you need to start pulseaudio like so:
    ```bash
    brew services start pulseaudio
    ```
    You can stop it by running `brew services stop pulseaudio` when you are done.

When launching from VS Code, make sure the `(lldb) Launch` configuration is selected.
    
## Building and Flashing Firmware

1. Clone this repository.
    ```powershell
    cd ~/esp/
    git clone --recurse-submodules https://github.com/AEFeinstein/Super-2024-Swadge-FW.git
    cd Super-2024-Swadge-FW
    ```
2. Make sure the ESP-IDF symbols are exported. This example is for Windows, so the actual command may be different for your OS. Note that `export.ps1` does not make any permanent changes and it must be run each time you open a new terminal for a build.
    ```powershell
    ~/esp/esp-idf/export.ps1
    ```
3. Manual Flashing
    1. Switch the Swadge to USB power so that it is off, hold down the PGM button (up on the D-Pad), and plug it into your computer. Note the serial port that enumerates.
    2. Build and flash with a single command. Note in this example the ESP is connected to `COM8`, and the serial port will likely be different on your system.
    ```powershell
    idf.py -p COM8 -b 2000000 build flash
    ```
      - For Linux and MacOS, use /dev/tty\[devicename] and /dev/cu.\[devicename]
4. Automatic Flashing
    1. Once the Swadge is plugged in and powered on, run this single command which will build the firmware, reboot the Swadge into bootloader mode, flash the firmware, reboot the Swadge again, and open up a serial terminal for debug output.
    ```bash
    make usbflash
    ```
    > **Warning**
    >
    > The Windows version of this script uses `esptool.exe` rather than `esptool.py` because `esptool.py`sometimes doesn't work when invoked independently. If this doesn't work, make sure you've installed `esptool` per the instructions in "Configuring a Windows Environment".

## Building and Running the Emulator

1. Clone this repository. You probably already have it from the prior step but it never hurt to mention.
2. Build the emulator.
    ```powershell
    make clean all
    ```
3. Run the emulator. 
   ```powershell
   ./swadge_emulator
   ```

## Configuring VSCode

[Visual Studio Code IDE](https://code.visualstudio.com/) is recommended for all OSes. The following plugins are **REQUIRED**:
* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) - Basic support
  - For MacOS, [LLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) is used instead
* [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) - Basic support
* [Makefile Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.makefile-tools) - Basic support
* [Espressif IDF](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) - Integration with ESP-IDF. When setting this up for the first time, point it at ESP-IDF which was previously installed. Do not let it install a second copy. Remember that ESP-IDF should exist in `~/esp/esp-idf` and the tools should exist in `~/.espressif/`.

While not 100% required, these extensions are highly recommended:
* [C/C++ Advanced Lint](https://marketplace.visualstudio.com/items?itemName=jbenden.c-cpp-flylint) - Integration with `cppcheck`
* [Clang-Format](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format) - Integration with `clang-format`
* [Code Spell Checker](https://marketplace.visualstudio.com/items?itemName=streetsidesoftware.code-spell-checker) - Helps minimize typos
* [Doxygen Documentation Generator](https://marketplace.visualstudio.com/items?itemName=cschlosser.doxdocgen) - Integration with `doxygen`
* [Github Actions](https://marketplace.visualstudio.com/items?itemName=github.vscode-github-actions) - GUI for git interactions
* [Todo Tree](https://marketplace.visualstudio.com/items?itemName=Gruntfuggly.todo-tree) - Handy to track "to-do items"

All of the extensions listed above are in the "recommended" tab of VS Code when this repository is active.

The `.vscode` folder already has tasks for making and cleaning the emulator. It also has launch settings for launching the emulator with `gdb` attached. To build the firmware from VSCode, use the Espressif extension buttons on the bottom toolbar. The build icon looks like a cylinder. Hover over the other icons to see what they do.

If VSCode isn't finding ESP-IDF symbols, try running the `export.ps1` script from a terminal, then launching code from that same session. For convenience, you can use a small script which exports the ESP-IDF symbols and launches VSCode.

`vsc_esp.sh`:
```bash
~/esp/esp-idf/export.ps1
code ~/esp/Super-2024-Swadge-FW
```

## Updating ESP-IDF

On occasion the ESP-IDF version used to build this project will increment. The easiest way to update ESP-IDF is to delete the existing one, by default installed at `~/esp/esp-idf/`, and the tools, by default installed at `~/.espressif/`, and follow the guide above to clone the new ESP-IDF and run the install script.

Alternatively, you can update the IDF in-place with the following commands. This example updates the IDF to 5.2.5, and you can change that version as is necessary. These are Linux commands, so they may need to be tweaked slightly for Windows.

```bash
# Change directory to where the IDF is installed
cd ~/esp/esp-idf/

# Update the IDF with git
git fetch --prune
git checkout tags/v5.2.5
git submodule update --init --recursive

# Install updated tools and python environment
./install.sh
. ./export.sh

# Clean up
./tools/idf_tools.py uninstall --remove-archives
```
