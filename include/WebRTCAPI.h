#pragma once

#include "resource.h"       // main symbols
#include <atlctl.h>
#include "..\WebRTC_ATL_i.h"
#include "_IWebRTCAPIEvents_CP.h"
#include "talk/base/ssladapter.h"

// this group is a hack to get around min/max issues with importing GdiPlus with NOMINMAX defined.
#include <algorithm>
using std::min;
using std::max;
#pragma warning(push)
#pragma warning(disable : 4244)
#include <GdiPlus.h>
#pragma warning(pop)

#include <atlenc.h>
#include <atlstr.h>
#include <atlimage.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;

static std::string* mBase64Encode(void * bytes, int byteLength){
	std::string* result = nullptr;
	if (0 != bytes) {

		CStringA base64;
		int base64Length = Base64EncodeGetRequiredLength(byteLength);
		if (Base64Encode(
				static_cast<const BYTE*>(bytes),
				byteLength,
				base64.GetBufferSetLength(base64Length),
				&base64Length, 
				ATL_BASE64_FLAG_NOCRLF)){

			base64.ReleaseBufferSetLength(base64Length);
			result = new std::string(base64);
		}
	}
	return result;
}

// forget understanding it. It's magic.
static std::string* encodeImage(const uint8* image, const BITMAPINFO bmi){

	HBITMAP bitmap = ::CreateBitmap(
		bmi.bmiHeader.biWidth, 
		abs(bmi.bmiHeader.biHeight), 
		bmi.bmiHeader.biPlanes, 
		bmi.bmiHeader.biBitCount, 
		(void*)image
	);

	std::string* result = nullptr;
	if (bitmap) {
		CImage c;
		c.Attach(bitmap);
		ULONGLONG length;
		IStream *pStream = NULL;
		if (CreateStreamOnHGlobal(NULL, /*delete on release*/TRUE, &pStream) == S_OK) {
			if (c.Save(pStream, Gdiplus::ImageFormatJPEG) == S_OK) {
				ULARGE_INTEGER ulnSize;
				LARGE_INTEGER lnOffset;
				lnOffset.QuadPart = 0;
				if (pStream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize) == S_OK)	{
					if (pStream->Seek(lnOffset, STREAM_SEEK_SET, NULL) == S_OK)	{
						length = ulnSize.QuadPart;
						ULONG ulBytesRead;
						BYTE* baPicture = new BYTE[length];
						pStream->Read(baPicture, length, &ulBytesRead);
						if (baPicture && length){
							result = mBase64Encode(baPicture, length);
						}
						delete baPicture;
					}
				}
			}

		}
		pStream->Release();
	}
	return result;
}

// CWebRTCAPI
class ATL_NO_VTABLE CWebRTCAPI :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CStockPropImpl<CWebRTCAPI, IWebRTCAPI>,
	public IOleControlImpl<CWebRTCAPI>,
	public IOleObjectImpl<CWebRTCAPI>,
	public IOleInPlaceActiveObjectImpl<CWebRTCAPI>,
	public IViewObjectExImpl<CWebRTCAPI>,
	public IOleInPlaceObjectWindowlessImpl<CWebRTCAPI>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CWebRTCAPI>,
	public CProxy_IWebRTCAPIEvents<CWebRTCAPI>,
	public IQuickActivateImpl<CWebRTCAPI>,
#ifndef _WIN32_WCE
	public IDataObjectImpl<CWebRTCAPI>,
#endif
	public IProvideClassInfo2Impl<&CLSID_WebRTCAPI, &__uuidof(_IWebRTCAPIEvents), &LIBID_WebRTC_ATLLib>,
	public IObjectSafetyImpl<CWebRTCAPI, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
	public CComCoClass<CWebRTCAPI, &CLSID_WebRTCAPI>,
	public CComControl<CWebRTCAPI>,
	public JavaScriptCallback
{
public:

	CWebRTCAPI() {
		m_bWindowOnly = true;
	}

	BSTR Convert(const std::string& s) {
		return CComBSTR(s.c_str()).Detach();
	}

	//talk_base::scoped_refptr<Conductor> conductor_ = 0;

	virtual void SendToBrowser(const std::string& json);	// inherited from JavaScriptCallback

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_INSIDEOUT |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST
)

DECLARE_REGISTRY_RESOURCEID(IDR_WEBRTCAPI)


BEGIN_COM_MAP(CWebRTCAPI)
	COM_INTERFACE_ENTRY(IWebRTCAPI)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IQuickActivate)
#ifndef _WIN32_WCE
	COM_INTERFACE_ENTRY(IDataObject)
#endif
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafety)
END_COM_MAP()

BEGIN_PROP_MAP(CWebRTCAPI)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY_TYPE("Property Name", dispid, clsid, vtType)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CWebRTCAPI)
	CONNECTION_POINT_ENTRY(__uuidof(_IWebRTCAPIEvents))
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CWebRTCAPI)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)

	MESSAGE_HANDLER(33176, OnOtherDestroy)
	MESSAGE_HANDLER(WM_CLOSE, OnClose)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

	// HACK: using the message loop as a thread-safe message passing structure
	MESSAGE_HANDLER(WM_APP+1,  OnMessage)
	
END_MSG_MAP()

// Handler prototypes:
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid) {
		static const IID* const arr[] = {
			&IID_IWebRTCAPI,
		};

		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++) {
			if (InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

	MainWnd* mainWindow;
	HWND controlHwnd;


// IWebRTCAPI
public:

	/* Add exposed javascript methods here */
	
	IFACEMETHOD(run)();
	IFACEMETHOD(hello)(BSTR *pRet);
	IFACEMETHOD(pushToNative)(BSTR cmd, BSTR json);
	
	std::map<std::string, Conductor*> conductors;

	void SendSelfie();

	void SendWindowHandle(HWND wnd);

	talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct() {		
		peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();
		mainWindow = new MainWnd(peer_connection_factory_);
		return S_OK;
	}

	void FinalRelease()	{

		try {
			talk_base::CleanupSSL();
			CoUninitialize();
		}
		catch (std::exception& ex){
			LOG(LS_ERROR) << "Exception on shutdown " << ex.what();
		}

		if (mainWindow)
			delete mainWindow;

		LOG(INFO) << "Destructor hit.";
	}
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOtherDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

};

OBJECT_ENTRY_AUTO(__uuidof(WebRTCAPI), CWebRTCAPI)
