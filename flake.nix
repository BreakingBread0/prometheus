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

        preFixup = ''
          rm -f \
            $out/bin/lldb \
            $out/bin/lldb-mi \
            $out/bin/lldb-server \
            $out/bin/lldb-dap \
            $out/lib/liblldb.so* \
            $out/lib/liblldbIntelFeatures.so*
        '';

        nativeBuildInputs = [ pkgs.autoPatchelfHook ];

        buildInputs = with pkgs; [
          stdenv.cc.cc.lib
          glibc
          zlib
          libxml2
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
        packages = buildDeps;
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