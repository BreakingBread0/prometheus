mkdir -p build
cd build


cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
  -DCMAKE_TOOLCHAIN_FILE=../external/vcpkg/scripts/buildsystems/vcpkg.cmake \


#cmake .. \
#  -DCMAKE_BUILD_TYPE=Release \
#  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
#  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
#  -DCMAKE_TOOLCHAIN_FILE=../external/vcpkg/scripts/buildsystems/vcpkg.cmake \
#  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/home/uwu/Desktop/prometheus/external/vcpkg/scripts/toolchains/mingw.cmake \
#  -DVCPKG_TARGET_TRIPLET=x64-mingw-static \

cmake --build .