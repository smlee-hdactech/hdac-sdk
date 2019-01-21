[![Build Status](https://travis-ci.org/smlee-hdactech/hdac-sdk.svg?branch=master)](https://travis-ci.org/smlee-hdactech/hdac-sdk)

# hdac-sdk

## ubuntu 에서 빌드 방법

### 빌드 전에 설치해야 할 패키지
```
sudo apt install -y autoconf libtool cmake g++ libssl-dev pkg-config
sudo apt install -y libboost-system-dev libboost-thread-dev
```

### 직접 빌드해서 설치해야 할 개발용 라이브러리

#### secp256k1
```
git clone https://github.com/bitcoin-core/secp256k1.git
cd secp256k1
./autogen.sh
./configure --enable-module-recovery
make && sudo make install
cd -
```

#### json_spirit
```
git clone https://github.com/smlee-hdactech/json_spirit.git
cd json_spirit
cmake -S . -B_build
cmake --build _build
sudo make install -C_build
cd -
```

### hdac-sdk 빌드
```
cmake -S . -B_build
cmake --build _build
```

## windows에서 빌드 방법

### vcpkg 및 관련 package 설치
```
git clone https://github.com/smlee-hdactech/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

vcpkg install boost-system boost-thread boost-assign boost-variant boost-asio boost-timer
vcpkg install secp256k1
vcpkg install json-spirit
```

### hdac-sdk 빌드
```
cmake -S . -B_build -DCMAKE_TOOLCHAIN_FILE=[vcpkg 설치 위치]\scripts\buildsystems\vcpkg.cmake
cmake --build _build
```

### visual studio 2017에서 빌드하는 경우
CMakeSettings.json 파일을 아래와 같이 수정해주면 사용할 수 있습니다.
```
{
  "configurations": [
    {
      "name": "x86-Debug",
      "generator": "Ninja",
      "configurationType": "Debug",
      "inheritEnvironments": [
        "msvc_x86"
      ],
      "buildRoot": "${env.USERPROFILE}\\CMakeBuilds\\${workspaceHash}\\build\\${name}",
      "installRoot": "${env.USERPROFILE}\\CMakeBuilds\\${workspaceHash}\\install\\${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "-v",
      "ctestCommandArgs": "",
      "variables": [
        {
          "name": "CMAKE_TOOLCHAIN_FILE",
          "value": "[vcpkg 설치 위치]\\scripts\\buildsystems\\vcpkg.cmake"
        }
      ]
    }
  ]
}
```
