#include "stdafx.h"

#include "talk/base/json.h"
#include "talk/base/ssladapter.h"
#include "talk/base/win32socketinit.h"
#include "talk/base/win32socketserver.h"
#include "conductor.h"
#include "main_wnd.h"
#include "peer_connection_client.h"

#include "WebRTCAPI.h"

//(HINSTANCE instance, DWORD dwReason, LPVOID) {

#define DEBUG_WEBRTC

extern const std::string gettime();

IFACEMETHODIMP CWebRTCAPI::hello(BSTR *pRet)
{
	// Allocate memory for the string.
	*pRet = ::SysAllocString(L"HelloWorld from C++");
	return pRet ? S_OK : E_OUTOFMEMORY;
}

void showDebugAlert(LPCWSTR caption, LPCWSTR text)
{
#ifdef DEBUG
	MessageBox(NULL, text, caption, MB_OK | MB_SYSTEMMODAL);
#endif // DEBUG
}

std::string BSTR2string(BSTR bstr)
{
	USES_CONVERSION;
	return std::string(W2A(bstr));
}

//Conductor* gdhConductor = 0;
// talk_base::scoped_refptr<Conductor> conductor = 0;	// migrate to this (make static?), remove gdhConductor

/*
void quiter()
{
	if (gdhConductor != 0)
	{
		gdhConductor->Close();
		PostQuitMessage(0);
	}
}
*/

IFACEMETHODIMP CWebRTCAPI::pushToNative(BSTR bcmd, BSTR bjson)
{
//	std::wstring ws(bjson, SysStringLen(bjson));
//	BSTR bs = SysAllocStringLen(ws.data(), ws.size());

	std::string cmd = BSTR2string(bcmd);
	std::string json = BSTR2string(bjson);

// SendToBrowser("incoming from JS to C++");
//	Fire_EventToBrowser(bjson);

	Json::StyledWriter writer;
	Json::Value jsonobj(json);

	LOG(INFO) << gettime() + " push to native ***************************** " << json;

	int m = cmd.find("makeoffer");
	int a = cmd.find("gotanswer");

	int o = cmd.find("gotoffer");

	int h = cmd.find("hangup");

	int t = cmd.find("gotcandidate");

	int q = cmd.find("quit");		// seems to cause problems
	int d = cmd.find("debug");

	if (o > -1)
		conductor_->gotoffer(json);
	else if (a > -1)
		conductor_->gotanswer(json);
	else if (h > -1)
		conductor_->hangup();
	else if (m > -1)
		conductor_->createoffer();
	else if (t > -1)
		conductor_->candidate(json);
	else if (d > -1)
	{
	#if WIN32
		::DebugBreak();
	#endif
	}
	else if (q > -1)
	{
		conductor_->Close();	// need this, or perhaps something like it
//well??		PostQuitMessage(0);
	}

	LOG(INFO) << "\n\nDONE push to native() *********************************************************";
	return S_OK;
}

void CWebRTCAPI::SendToBrowser(const std::string& json)		// from JavaScriptCallback
{
	BSTR bjson = Convert(json);
	Fire_EventToBrowser(bjson);
}

void logtofile()
{
//	std::string log = "verbose";
	talk_base::LogMessage::LogToDebug(talk_base::LS_INFO);

	talk_base::FileStream *fs = new talk_base::FileStream();
	if (!fs->Open("WebRtcAx.log", "w", NULL))
		LOG(INFO) << "Could not open file";
	else
		talk_base::LogMessage::LogToStream(fs, talk_base::LS_INFO);
}

IFACEMETHODIMP CWebRTCAPI::run()
{
	logtofile();

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	talk_base::SetRandomTestMode(true);
	talk_base::EnsureWinsockInit();
	talk_base::Win32Thread w32_thread;
	talk_base::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

	controlHwnd = m_hWnd;

	if (controlHwnd == NULL || !mainWindow.Create(controlHwnd))
	{
		showDebugAlert(_T("Error in my code :("), _T("hwnd is null!"));
		return -1;
	}

	PeerConnectionClient client;

	if (!talk_base::InitializeSSL(NULL) || !talk_base::InitializeSSLThread())
		LOG(LS_ERROR) << "error failed to init ssl";

//	talk_base::scoped_refptr<Conductor> conductor(new talk_base::RefCountedObject<Conductor>(&client, &mainWindow));

	conductor_ = new talk_base::RefCountedObject<Conductor>(&client, &mainWindow);

//	gdhConductor = conductor;	// we need to be able to call the conductor
	conductor_->javascriptCallback_ = this;		// the conductor needs to be to call us

//	conductor->getlocalvideo();   use buttons to call initpeerconnection and make offer

	// Main loop.
	MSG msg;
	BOOL gm;

	while ((gm = ::GetMessage(&msg, NULL, 0, 0)) != 0 && gm != -1)
	{
		if (msg.message == WM_CLOSE || msg.message == 33176)
			break;
/*
		if (// msg.message == 49819 ||
			msg.message == 33176 ||
//			msg.message == 33172 ||
			msg.message == 1792 ||
			msg.message == WM_CLOSE)
		{
		}
*/
/*
if (msg.message == WM_CLOSE)
{
//	client.disconnect_all();
	conductor->Close();
//	PostQuitMessage(0);
	break;
}
*/
		if (!mainWindow.PreTranslateMessage(&msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	// here we have a working shutdown:

	PostQuitMessage(0);			// ya really?

	client.SignOut();

	client.disconnect_all();

	conductor_->Close();

	talk_base::CleanupSSL();

// causes ie to crash on close		free(conductor_);

	CoUninitialize();

	return S_OK;
}

	/*
	{
		while ((conductor->connection_active() || client.is_connected()) &&
			(gm = ::GetMessage(&msg, NULL, 0, 0)) != 0 && gm != -1)
		{
			if (!mainWindow.PreTranslateMessage(&msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}
	*/

LRESULT CWebRTCAPI::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	mainWindow.OnPaint();
	return 0;
}

LRESULT CWebRTCAPI::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CWebRTCAPI::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	mainWindow.Destroy();
	return 0;
}
