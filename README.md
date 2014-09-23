This code will only work with an antiquidated version of the Google WebRTC code base and was only at a demo-able state, not production quality.
We have stopped work on this project because Temasys has a free IE plugin and Doubango has an
open source one (see https://github.com/sarandogou/webrtc-everywhere). If you want to develop your own plugin,
we suggest starting from the Doubango code base.


easyrtc_ie_plugin
=================

## Deps 

Install depot_tools:

[Depot tools Windows installation instructions](http://www.chromium.org/developers/how-tos/install-depot-tools)

Follow the [instructions for installing the Chromium build deps for Visual Studio 2013](http://www.chromium.org/developers/how-tos/build-instructions-windows#TOC-Setting-up-the-environment-for-Visual-Studio-2013)

### Environment Variables

You can create a shortcut to the environment variable dialog with the path: ```%windir%\System32\rundll32.exe sysdm.cpl,EditEnvironmentVariables```

Add the following environment variables:

- ```GYP_DEFINES``` -> ```component=shared_library build_with_chromium=0```
- ```GYP_GENERATORS``` ->  ```msvs```
- ```GYP_MSVS_VERSION``` ->  ```2013```
- ```WDK_DIR``` -> ```c:\WinDDK\7600.16385.1``` (or somewhere else if you chose a custom dir)

### Getting The Code

- Clone the repository using ```gclient config http://webrtc.googlecode.com/svn/trunk```.
- Get the source code -> ```gclient sync --nohooks```.
- Create project files with gyp (overwriting any existing ones!) -> ```gclient runhooks --force```.

### Directory Setup

This project is not integrated with ```gyp``` currently, so a manual setup is required.

Place this repo in the same directory as ```trunk/``` as cloned from ```gclient sync```.

```
<dir>/trunk/
<dir>/easyrtc_ie_plugin/
```

Open the ```<dir>/trunk/all.sln``` file in Visual Studio (VS2013).
Right click root project node and "Add Existing Project..."
Choose ```<dir>/easyrtc_ie_plugin/WebRTC_ATL.vcxproj```

## Building (Visual Studio 2013)

- Start VS2013 as Administrator (required to register ActiveX control)
- In VS2013, right click the WebRTC_ATL project node
-- Select "Rebuild Project". 

Once built, you can avoid rebuilding WebRTC by just choosing "Build Project".

# Useful Links

- [webrtc.org](http://www.webrtc.org/)
- [WebRTC Development - Getting Started](http://www.webrtc.org/reference/getting-started)
