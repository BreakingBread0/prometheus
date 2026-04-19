#!/usr/bin/env -S nix --experimental-features 'nix-command flakes' shell nixpkgs#umu-launcher nixpkgs#meow nixpkgs#bash --command bash
# The above shebang is for NixOS users and may not work on Ubuntu or other distros.
# If it doesnt, try changing it to /bin/bash or /usr/bin/bash

meow
WINEPREFIX=~/.prometheus.wineprefix umu-run ./Prometheus.exe
