# Prometheus <!-- omit in toc -->

Hey and welcome, thanks for stopping by!
- [0. Preface](#0-preface)
- [1. Usage](#1-usage)
  - [Running a release build](#running-a-release-build)
    - [Running on Linux](#running-on-linux)
    - [Optional stuff](#optional-stuff)
  - [Compilation](#compilation)
    - [Windows](#windows)
    - [Linux](#linux)
- [2. Documentation about the game](#2-documentation-about-the-game)
- [3. Contributions Welcome!](#3-contributions-welcome)
- [4. Open Source libraries used](#4-open-source-libraries-used)
- [5. License and Contact](#5-license-and-contact)

# 0. Preface

This project aims to revive the (long abandoned) Overwatch 0.8 beta from 2015. It is not feature complete, and much work is to be done. If you are not a developer, you can currently load a map, spawn a hero and look around, but not much else apart from that.

# 1. Usage

This project is more an exploration of all the structures and the general game layout. It is not ready, and you cannot play with other people. A lot of hero abilities don't work, including shooting (some) weapons.

If you havent already, download the 0.8 beta files and extract the files somewhere.
* ⚠️ Make sure that you don't download any malicious executable and verify that GameClientApp.exe is signed by Blizzard.
* You can safely remove the BlizzardError directory.
* You do not need the "Overwatch Launcher.exe". This just downloads and opens Battle.net.

## Running a release build

* Download a release.zip from [HERE](https://cdn.owdev.wiki/ci/prometheus/).
* Extract everything and run Prometheus.exe. Once started, select the game executable called "GameClientApp.exe" and press Launch.
  * You can optionally change some settings and startup arguments.
* Congratulations, you're done :) Have a cookie 🍪

### Running on Linux

On linux, instead of running Prometheus.exe, wrapper scripts _prometheus.sh_ and _prometheus-nixos.sh_ are provided.
* If you're not on NixOS, install [umu-launcher](https://github.com/Open-Wine-Components/umu-launcher) and run _prometheus.sh_
  * This is the easiest method to run the launcher and Overwatch. It should work out of the box. If you do not want to install umu-launcher, you can also use your favourite wine/proton game launcher like [Lutris](https://lutris.net/).
* If you're on NixOS, you can directly run _prometheus-nixos.sh_

### Optional stuff

* Download the [MonaspaceXenon](https://monaspace.githubnext.com/) font and put the -regular.otf and -bold.otf in the directory of the game files.
* Download the [Font Awesome v6](https://fontawesome.com/v6/download) free desktop font files and put the .otf files into the game directory.
* Once first started, the library will create hashlibrary.json. You can add crc32 strings / elements to hash which will be displayed in various places where applicable. You can just add all the strings from the [overtools github repository](https://github.com/overtools/OWLib/tree/develop/TankLibHelper/DataPreHashChange). To do so add another root JSON element (an array) called "add" and put all your strings there. See the [json schema](hashlibrary.schema.json).

## Compilation

### Windows

* Install Visual Studio and the C++ compiler (msvc)
* Install the vcpkg & CMake extensions for Visual Studio
* Clone the repository recursively
* Initialize vcpkg by running vcpkg.bat
* Build the MSVC preset (Debug-MSVC, Release-MSVC)

### Linux

* Install the [nix package manager](https://nixos.org/)
* Enable nix flakes* (or run every "nix" command with the arguments `--extra-experimental-features 'nix-command flakes'`)
* Clone the repository recursively
* cd into the directory
* Execute the following commands:
  * To build the launcher + core dll: `nix develop .#build`
    * Currently not using `nix build`, since we are using vcpkg as a package manager and it downloads dependencies during the build step (but that should be deterministic enough since we have pinned a baseline)
  * To enter the build environment: `nix develop`
    * Once you are in the build environment, you can start your favourite IDE. All environment variables (like compiler, linker, libraries, headers, etc) are already set for you.

\* please dont make the same mistake as me and follow literally every tutorial's advice ever when installing NixOS: Enable flakes immediately :D

# 2. Documentation about the game

This has been moved to the new wiki, [owdev.wiki](https://owdev.wiki)!

# 3. Contributions Welcome!
I envision a future in which we are able to play any Overwatch version that was released. With help from the community, this isn't just a dream, but a real possibility. Please help by forking, contributing, opening bug reports and sharing <3. Remember, great science is always the result of collaboration!

# 4. Open Source libraries used

(TODO, I probably forgot something)
* [keystone](https://github.com/keystone-engine/keystone)
* [capstone](github.com/capstone-engine/capstone)
* [imgui](https://github.com/ocornut/imgui/)
* [imnodes](https://github.com/BreakingBread0/imnodes/) ([my branch](https://github.com/BreakingBread0/imnodes/))
* pe (Could not find the original repository, please open an issue if you know it!)
* [lazy_importer](https://github.com/JustasMasiulis/lazy_importer)
* [nlohmann_json](https://github.com/nlohmann/json)
* [freetype](https://github.com/freetype/freetype)
* [spdlog](https://github.com/gabime/spdlog)
* [fmt](https://github.com/fmtlib/fmt)
* [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended)
* [libbacktrace](https://github.com/ianlancetaylor/libbacktrace)
* [vcpkg](https://github.com/microsoft/vcpkg)
* [Monaspace Xenon](https://monaspace.githubnext.com/) (optional)
* [Font Awesome](https://fontawesome.com) (optional)
<!-- * [ixwebsocket](https://github.com/machinezone/IXWebSocket) -->

# 5. License and Contact

AGPL License. Contact me for any questions at contact@owdev.wiki or open a discussion thread <3
NOTE: The license was changed from MIT. AGPL ensures that this is a project made by the community, for the community.

![prometheus logo](images/prometheus.png)
