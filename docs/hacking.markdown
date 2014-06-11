hacking on the IE plugin
========================

# General Architecture

```
                                          /----------------\
                             /------------| Device Manager |
                             |            \----------------/
                             V                      |
/-----------------\    /-----------\      /------------------\
| IE / Javascript |--->| WebRTCAPI |=====>| Peer Connections |
\-----------------/    |           |      \------------------/
         |             |           |                | 
         |             |           |    /------------------------\
         |             |           |    | Rendered base64 string |
         |             \-----------/    \------------------------/
         |               |       |                  |
         \<--------------/       \<-----------------/
             Javascript            UI Thread Message
             Bindings
```

The plugin leverages ATL to interface with Win32 and COM and implements a scriptable ActiveX control that can be embedded within IE via an `<object>` tag. Javascript interacts with the ActiveX object via the mechanisms provided by ATL, as defined in the WebRTCAPI.idl file in the project.

This interface file (.idl == interface definition language) defines the ways that Javascript can call to the plugin. There is an extention interface used to implement calling back to Javascript following ATL convention as well.

## Important parts:

- Javascript-side shim/interface: [rtcplugin.js](../rtcplugin.js) 
- ATL Method Interface [WebRTCAPI.h](../include/WebRTCAPI.h)
- ATL Event Interface [\_IWebRTCAPIEvents\_CP.h](../include/_IWebRTCAPIEvents_CP.h)
- Device and call management [main_wnd.h](../include/main_wnd.h)

### Interface Code  (idl, message map)

[rtcplugin.js](../rtcplugin.js) provides an abstraction from and binding to the methods and events exposed on the WebRTCAPI ActiveX plugin. One instance of an RTCPlugin object is needed per page. Canvas elements for rendering to are passed to the plugin, and signalling is responsible for calling the appropriate interface methods to hook up a call, hang up, etc.

### ATL Message Map

The message map is implemented using ATL macros, and looks like:

- [WebRTCAPI.h](../include/WebRTCAPI.h)
```C++
BEGIN_MSG_MAP(CWebRTCAPI)
  MESSAGE_HANDLER(WM_PAINT, OnPaint)
  MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
  ...
  // HACK: using the message loop as a thread-safe message passing structure
  MESSAGE_HANDLER(WM_APP+1,  OnMessage)
END_MSG_MAP()
```

#### Cross-threaded concerns, posting events to javascript

The plugin makes use of windows primitives to pass data between threads. The ATL framework provides for us a windows (WndProc equivalent) message loop, and this is augmented with hooks via the message map detailed above. This message loop is used for both Win32 events (such as WM_PAINT, WM_CLOSE, etc) as well as application specific messages (WM_APP+1).

The need for multiple threads arises due to the fact that execution of Javascript should not be blocked, but messages need to be passed between the plugin and the Javascript/Browser with the two components running concurrently. 

Where a PeerConnectionWrapper needs to send data to javascript, it is first proxied across the thread boundary with a call to QueueUIThreadCallback, and on the other side of the thread wall, it is passed to the Javascript 'event' interface registered by the `rtcplugin.js`. (The strange double-colon notation is used for binding to the ActiveXObject's events.)


### TODO - refactor/rename the main\_wnd.h classes to be more along the lines of DeviceContext/DeviceManager
- [main\_wnd.h](../include/main_wnd.h)
- device + context management in main\_wnd

main\_wnd implements a number of interfaces to expose certain functionality to various parts of the process, while isolating unrelated functionality from others. 

Interfaces:
- MainWindow 
- JavascriptCallback - exposes a single method to push data to Javascript through WebRTCAPI
- MainWnd - implementation

## signalling negotiation

With all signalling handled in Javascript, a minimal interface is provided to pass the SDP records to the consuming C++ code. C++ 
- peer connection management, rendering


## Javascript API, message formats

The javascript interface to the plugin is provided by [rtcplugin.js](../rtcplugin.js). It exposes a number of calling and setup-related methods to allow the consumer to make use of the ActiveX plugin. See the source file for documentation, and also see [ie_sample.htm](../ie_sample.htm) for an example implementation making use of it.

## Installer

We chose to go with [Inno setup](http://www.jrsoftware.org/isinfo.php) for building our installer, and the [ksign](http://blog.ksoftware.net/tag/ksign/) tool for signing the binary.

* more information is availavble in the `installation` directory, at the root of the project.

## Sample 

A sample is included in [ie_sample.htm](../ie_sample.htm) 

## TODO

### Refactorings and bugs
- Camera not started until a peer connection is opened
  - Lift the capure source out of the peer connection wrapper and make it a borrowed resource
- possible memory leak in rendering pipeline (i420->jpeg->base64 string)


