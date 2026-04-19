{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/master";
#    nixpkgs_old.url = "github:nixos/nixpkgs/d916df777523d75f7c5acca79946652f032f633e";
  };

  outputs = { self, nixpkgs, ... }:
  let
    system = "x86_64-linux";

    pkgs = nixpkgs.legacyPackages.x86_64-linux.pkgs;
    cross-env = self.packages.${system}.cross-env;

    environment = {
      CC = "x86_64-w64-mingw32-clang";
      CXX = "x86_64-w64-mingw32-clang++";
      RC = "x86_64-w64-mingw32-windres";

      LIBRARY_PATH = "${cross-env}/lib";
      C_INCLUDE_PATH = "${cross-env}/include";
      CMAKE_PREFIX_PATH = "${cross-env}/x86_64-w64-mingw32";

      VCPKG_ROOT = "${pkgs.vcpkg}/share/vcpkg";
    };

    buildDeps = with pkgs; [
      cmake
      ninja
      pkg-config
      vcpkg
      powershell
      cross-env

      # This is for installing libbacktrace.
      autoconf
      automake
      libtool
    ];
  in {
    packages.${system} = {
      cross-env = pkgs.stdenv.mkDerivation rec {
        pname   = "llvm-mingw";
        version = "20260407";

        src = pkgs.fetchurl {
          url    = "https://github.com/mstorsjo/llvm-mingw/releases/download/${version}/llvm-mingw-${version}-ucrt-ubuntu-22.04-x86_64.tar.xz";
#          sha256 = pkgs.lib.fakeSha256;  # run once to get real hash
          sha256 = "sha256-w5rrSCO7yJzipAgglkoRRhSlJMLLe+Hj2v0W94D6ObE=";
        };

        nativeBuildInputs = [ pkgs.autoPatchelfHook ];

        buildInputs = with pkgs; [
          stdenv.cc.cc.lib
          glibc
          zlib
          libxml2_13
          xz
          zstd
          ncurses
        ];

        dontBuild     = true;
        dontConfigure = true;

        installPhase = ''
          mkdir -p $out
          cp -r . $out/
        '';
      };
#       default = pkgs.stdenv.mkDerivation ({
#         name = "prometheus";
#         src  = ./.;
# 
#         nativeBuildInputs = buildDeps;
# 
#         # Point CMake at the Nix-built windows deps
#         cmakeFlags = [
#           "--preset=Release-MinGW"
#         ];
# 
#       } // environment);
    };
    devShells.${system} = {
      default = pkgs.mkShellNoCC ({
        packages = buildDeps ++ [
          pkgs.umu-launcher
#          pkgs.wine64
        ];
        shellHook = ''
        echo "================================================"
        echo "The build environment is ready. You can either use cmake directly or use your preferred IDE (like clion). In order to compile on Linux, use the MinGw (Release-MinGW, Debug-MinGW) presets."
        echo "To run the launcher, use 'umu-run'."
        echo "To automatically build the launcher and core library automatically, execute 'nix develop .#build'"
        echo "================================================"
        '';
      } // environment );
      build = pkgs.mkShellNoCC ({
        packages = buildDeps;
        shellHook = ''
          cmake --preset Release-MinGW
          # output path is set by binaryDir in presets
          cmake --build output/Release --target Prometheus-Core
          exit
        '';
      } // environment);
      build-clean = pkgs.mkShellNoCC ({
        packages = buildDeps;
        shellHook = ''
          rm -rf build
          cmake --preset Release-MinGW
          cmake --build output/Release --target Prometheus-Core
          exit
        '';
      } // environment);
    };
  };
}