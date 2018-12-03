# About
This project is a fork of a fantastic work by Campbell Suter, please see main project site here: https://gitlab.com/znixian/OpenOVR

This fork contains stripped down version of the Open Composite effort, with two goals in mind: 

* Focus on rFactor 2 simulator
* Stable, quality and predictable releases

# Installation
Download the DLL:
[Latest 64-bit v0.6.0.0_AVX2+PGO](https://ci.appveyor.com/api/projects/TheIronWolfModding/openovr/artifacts/OpenComposite_tiwm_0.6.0.0.zip) - Custom compiled DLL for Hasewell or later Intel processors + profile guided optimizations.    Not that it matters, just wanted to learn :D

If above version crashes, here's regular DLL version.
[Latest 64-bit v0.6.0.0](https://ci.appveyor.com/api/projects/TheIronWolfModding/openovr/artifacts/OpenComposite_tiwm_0.6.0.0.zip)

[Older DLL versions](https://ci.appveyor.com/project/TheIronWolfModding/openovr/history)

* Copy `openvr_api.dll` into `[rFactor 2 root]\Bin64` folder.  Usually `[rFactor 2 root]` is in Steam installation folder `Steam\SteamApps\common\rFactor 2\Bin64\`.
* If this is first time you install this .dll, optionally, copy included `opencomposite.ini` to the `[rFactor 2 root]\Bin64` folder.  This contains rF2 default settings.
* Adjust `supersampleRatio` value to your liking (this is Oculus PPD).

# Supported configuration flags
`supersampleRatio` - real number, default 1.0. The supersample ratio in use - this is similar to what you would enter into SteamVR,
a value of 145% in SteamVR is a value of 1.45 here. A value of 80% in SteamVR is 0.8 here, and so on. Higher numbers improve
graphics, at a major performance cost.
`enableLayers` - flag with `true` or `false` value, default `false`.  If `true`, converts SteamVR overlay creation calls to Oculus layers.  Otherwise, ignores those calls.
`enableCubemap` - flag with `true` or `false` value, default `true`.  If `true`, converts SteamVR skybox creation calls to Oculus cubemap.  Otherwise, ignores those calls.
`threePartSubmit` - flag with `true` or `false` value, default `true`.  If `true`, follows Oculus' frame submission pacing guidelines by waiting on `ovr_WaitToBeginFrame`.  Otherwise, no wait on submission is performed.
`useViewportStencil` - flag with `true` or `false` value, default `true`.  If `true`, enables experimental feature to hide invisible texture areas, which might improve performance.
`postInitCmd` - string value, default is empty.  Specifies command to run after Oculus runtime is initialized.  For example: `cmd /q /c timeout 5 & "C:\Program Files\Oculus\Support\oculus-dash\bin\OculusDash.exe"`.  Can be used start various tools, and kill undesired processes :).
`dx10Mode` - flag with `true` or `false` value, default `false`.  If `true`, enables DX10 support (used by Raceroom).  This is unsuported feature and will be deprecated in the future.


# History
12/03/2018 - v1.0.0.0
* Deprecate `enableAudio` feature.  Please use Windows 10 "App Volume and Device Preferences" instead.
* Removed code irrelevant to rF2.
* Internal cleanups and refactorings.
* Implemented AVX2+PGO  build.
* Logging improvements.

11/17/18 - v0.6.0.0
Initial fork.  Added features:
* `enableCubemap=true/false`: Default: `true`. Implements SkyBox (rF2 loading screen).
* `postInitCmd=<your command>` Default: `false`.  - runs supplied command after Oculus Runtime is initialized.  Example:
postInitCmd=cmd /q /c timeout 5 & "C:\Program Files\notepad.exe"
* `enableLayers=true/false`.  Default `false`.  Enables layers subsystem.  Currently used by Raceroom.
* `dx10Mode=true/false`.  Default `false`.  Allows forcing OC to accept DX10 textures (Raceroom).  Note: will be removed when Raceroom moves to DX11.