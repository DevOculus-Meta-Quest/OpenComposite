# About
This project as a fork of a fantastic work by Campbell Suter, please see main project site here: https://gitlab.com/znixian/OpenOVR

This fork contains stripped down version of the Open Composite effort, with two goals in mind: 

* Focus on rFactor 2 simulator
* Stable, quality and predictable releases

# Installation

Download the 64 bit DLL:
[Latest 64-bit v0.6.0.0](https://ci.appveyor.com/api/projects/TheIronWolfModding/openovr/artifacts/OpenComposite_tiwm_0.6.0.0.zip)

[Older DLL versions](https://ci.appveyor.com/project/TheIronWolfModding/openovr/history)

# History
11/17/18 - v0.6.0.0
Initial fork.  Added features:
* `enableCubemap=true/false`: Default: `true`. Implements SkyBox (rF2 loading screen).
* `postInitCmd=<your command>` Default: `false`.  - runs supplied command after Oculus Runtime is initialized.  Example:
postInitCmd=cmd /q /c timeout 5 & "C:\Program Files\notepad.exe"
* `enableLayers=true/false`.  Default `false`.  Enables layers subsystem.  Currently used by Raceroom.
* `dx10Mode=true/false`.  Default `false`.  Allows forcing OC to accept DX10 textures (Raceroom).  Note: will be removed when Raceroom moves to DX11.