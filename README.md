[![Build multi-platform](https://github.com/smiarx/aelapse/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/smiarx/aether/actions/workflows/cmake-multi-platform.yml)

![A screenshot of the Ælapse plugin](images/screenshot.png)

ÆLAPSE
=====

ÆLAPSE is a delay and reverb plugin. The delay is based on tape-delay
mechanisms, while the reverb is inspired by (but not limited to) spring
reverbs.

It is designed for a variety of applications, from subtle spring-reverb guitar
tones to dreamy synth pads, as well as dub-style snare sounds.

[See quick demo](https://www.youtube.com/watch?v=QFNVgy2odFs)

## Downloads

The plugin is available in VST, AU and LV2 format on the [release page](https://github.com/smiarx/aelapse/releases)

## Building

To build ÆLAPSE, you need CMake:

```bash
# Clone the repository and update submodules
$ git clone https://github.com/smiarx/aelapse.git
$ cd aelapse
$ git submodule update --init --recursive

# build
$ cmake -Bbuild -DCMAKE_BUILD_TYPE=Release .
$ cmake --build build --config Release --parallel 4
```

VST will be located in `build/Aelapse_artifacts/Release`.

On x86, you can enable the `DSP_X86_DISPATCH` option. This will compile the
audio section of the plugin for different architectures and select the
available one at runtime.

```bash
# build
$ cmake -Bbuild -DDSP_X86_DISPATCH=1 -DCMAKE_BUILD_TYPE=Release .
$ cmake --build build --config Release --parallel 4
```

You can also build with `-march=native` to build specifically for you machine.

```bash
$ cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native" .
$ cmake --build build --config Release --parallel 4
```

## Fonts

- [Roboto](https://fonts.google.com/specimen/Roboto)
- [Nunito](https://fonts.google.com/specimen/Nunito)
- [Lexend](https://fonts.google.com/specimen/Lexend)
