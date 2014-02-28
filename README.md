easyrtc_ie_plugin
=================

# # Deps

Install depot_tools:

[Depot tools Windows installation instructions](http://www.chromium.org/developers/how-tos/install-depot-tools)

Follow the [instructions for installing the Chromium build deps for Visual Studio 2013](http://www.chromium.org/developers/how-tos/build-instructions-windows#TOC-Setting-up-the-environment-for-Visual-Studio-2013)

Add the following environment variables:

- ```GYP_DEFINES``` -> ```component=shared_library build_with_chromium=0```
- ```GYP_GENERATORS``` ->  ```msvs```
- ```GYP_MSVS_VERSION``` ->  ```2013```
- ```WDK_DIR``` -> ```c:\WinDDK\7600.16385.1``` (or somewhere else if you chose a custom dir)

Clone the repository using ```git```.

# # # Important depot_tools Commands

Get the source code -> ```gclient sync --nohooks```
Create project files with gyp (overwriting any existing ones!) -> ```gclient runhooks --force```

# # Building

Place this repo in the same directory as ```trunk/``` as cloned from ```gclient sync```.

```
<dir>/trunk/
<dir>/easyrtc_ie_plugin/
```
