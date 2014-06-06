hacking on the IE plugin
========================

# General Architecture

```
                                          ------------------
                             |------------| Device Manager |
                             |            ------------------
                             V                      |
-------------------    -------------      --------------------
| IE / Javascript |--->| WebRTCAPI |=====>| Peer Connections |
-------------------    |           |      --------------------
         |             |           |                | 
         |             |           |    --------------------------
         |             |           |    | Rendered base64 string |
         |             -------------    --------------------------
         |               |       |                  |
         |<--------------|       |<-----------------|
             Javascript            UI Thread Message
             Bindings
```

The plugin leverages ATL to interface with Win32 and COM and implements a scriptable ActiveX control that can be embedded within IE via an `<object>` tag. Javascript interacts with the ActiveX object via the mechanisms provided by ATL, as defined in the WebRTCAPI.idl file in the project.

This interface file (.idl == interface definition language) defines the ways that Javascript can call to the plugin. There is an extention interface used to implement calling back to Javascript following ATL convention as well.

## Important parts:

### Interface Code  (idl, message map)

- [WebRTCAPI.h](../include/WebRTCAPI.h)

The message map is implemented using ATL macros, and looks like:

```C++
BEGIN_MSG_MAP(CWebRTCAPI)
  MESSAGE_HANDLER(WM_PAINT, OnPaint)
  MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
  ...
  // HACK: using the message loop as a thread-safe message passing structure
  MESSAGE_HANDLER(WM_APP+1,  OnMessage)
END_MSG_MAP()
```


# TODO - refactor/rename the main\_wnd.h classes to be more along the lines of DeviceContext/DeviceManager
- [main\_wnd.h](../include/main_wnd.h)
- device + context management in main\_wnd

main\_wnd implements a number of interfaces to expose certain functionality to various parts of the process, while isolating unrelated functionality from others. 

Interfaces:
- MainWindow - 
- JavascriptCallback - exposes a single method to push data to Javascript through WebRTCAPI
- MainWnd - implementation

- signalling negotiation, parsing of json in C++
- peer connection management, rendering
- cross-threaded concerns, posting events to javascript

- Javascript API, message formats
- Installer
- Sample

## TODO

### Refactorings and bugs
- Camera not started until a peer connection is opened
-- Lift the capure source out of the peer connection wrapper and make it a borrowed resource

- possible memory leak in rendering pipeline (i420->jpeg->base64 string)
- hangup regressed


