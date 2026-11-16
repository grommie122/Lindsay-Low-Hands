# Lindsay Low-Hands

Lindsay Low-Hands is a Windows x64 high-gain metal distortion VST3. Gain drives three clipping stages. Tight cuts bass before clipping. Bass, Mid, and Treble each adjust the post-distortion EQ by up to 12 dB. Level sets the output.

## Build

Install CMake and Visual Studio with Desktop development with C++. From
PowerShell in this folder, run:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

CMake fetches JUCE. To build with an existing JUCE checkout, add
`-DJUCE_DIR=C:/path/to/JUCE` to the first command.

## Install in REAPER

Copy the entire `Lindsay Low-Hands.vst3` folder from
`build/LindsayDistortion_artefacts/Release/VST3` into a directory in REAPER's VST
search path. On this machine the per-user path is
`%LOCALAPPDATA%\Programs\Common\VST3`.
In REAPER, open Preferences > Plug-ins > VST and click Re-scan. On a track,
open FX and search for Lindsay Low-Hands.





