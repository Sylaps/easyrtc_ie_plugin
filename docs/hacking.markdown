hacking on the IE plugin
========================

# General Architecture

The plugin leverages ATL to interface with Win32 and COM and implements a scriptable ActiveX control that can be embedded within IE via an <object> tag. Javascript interacts with the ActiveX object via the mechanisms provided by ATL, as defined in the WebRTCAPI.idl file in the project.

This interface file (.idl == interface definition language) defines the ways that Javascript can call to the plugin. There is an extention interface used to implement calling back to Javascript following ATL convention as well.

-------------------    -------------      --------------------
| IE / Javascript |--->| WebRTCAPI |=====>| Peer Connections |
-------------------    -------------      --------------------
         |               |       |                  |
         |               |       |      --------------------------
         |               |       |      | Rendered base64 string |
         |               |       |      --------------------------
         |               |       |                  |
         |<--------------|       |<-----------------|
             Javascript             UI Thread Message
             Bindings

## Important parts:

- Interface Code  (idl, message map)
- device + context management in main\_wnd
- signalling negotiation, parsing of json in C++
- peer connection management, rendering
- cross-threaded concerns, posting events to javascript
- Javascript API, message formats
- Installer
- Sample

## TODO

### Refactorings and bugs
- Camera not started until a peer connection is opened
- possible memory leak in rendering pipeline (i420->jpeg->base64 string)
- hangup regressed


