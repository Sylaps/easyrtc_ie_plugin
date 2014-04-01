// WebRTCAPI.h : Declaration of the CWebRTCAPI
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>
#include "..\WebRTC_ATL_i.h"
#include "_IWebRTCAPIEvents_CP.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;



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

	CWebRTCAPI()
	{
		m_bWindowOnly = true;
	}

	BSTR Convert(const std::string& s)
	{
		return CComBSTR(s.c_str()).Detach();
	}

	virtual void SendToBrowser(const std::string& json);	// from JavaScriptCallback
	/*
	{
		BSTR bjson = Convert(json);
		Fire_EventToBrowser(bjson);

//		Fire_EventToBrowser(L"{ \"fake\" : \"json\" }");
	}
	*/

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

	// these 2 lines seem to have no effect:
	//CHAIN_MSG_MAP(CComControl<CWebRTCAPI>)
	//DEFAULT_REFLECTION_HANDLER()

	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* const arr[] =
		{
			&IID_IWebRTCAPI,
		};

		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
			if (InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

	MainWnd mainWindow;
	HWND controlHwnd;

// IWebRTCAPI
public:


	/* Add exposed javascript methods here */
	
	IFACEMETHOD(run)();
	IFACEMETHOD(hello)(BSTR *pRet);
	IFACEMETHOD(pushToNative)(BSTR cmd, BSTR json);
	
/*
	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		RECT& rc = *(RECT*)di.prcBounds;
		// Set Clip region to the rectangle specified by di.prcBounds
		HRGN hRgnOld = NULL;
		if (GetClipRgn(di.hdcDraw, hRgnOld) != 1)
			hRgnOld = NULL;
		bool bSelectOldRgn = false;

		HRGN hRgnNew = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

		if (hRgnNew != NULL)
		{
			bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);
		}

		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
		SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
		LPCTSTR pszText = _T("WebRTCAPI");

		TextOut(di.hdcDraw,
			(rc.left + rc.right) / 2,
			(rc.top + rc.bottom) / 2,
			pszText,
			lstrlen(pszText));

		if (bSelectOldRgn)
			SelectClipRgn(di.hdcDraw, hRgnOld);

		DeleteObject(hRgnNew);

		return S_OK;
	}
*/
	
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

OBJECT_ENTRY_AUTO(__uuidof(WebRTCAPI), CWebRTCAPI)
