// dllmain.h : Declaration of module class.

class CWebRTC_ATLModule : public ATL::CAtlDllModuleT< CWebRTC_ATLModule >
{
public :
	DECLARE_LIBID(LIBID_WebRTC_ATLLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WEBRTC_ATL, "{55C0776C-1EFB-4589-BD70-209395CC3F32}")
};

extern class CWebRTC_ATLModule _AtlModule;
