#include "stdafx.h"

#include "talk/base/json.h"
#include "talk/base/ssladapter.h"
#include "talk/base/win32socketinit.h"
#include "talk/base/win32socketserver.h"
#include "conductor.h"
#include "main_wnd.h"
#include "peer_connection_client.h"

#include "WebRTCAPI.h"

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

// util
std::string BSTR2string(BSTR bstr)
{
	USES_CONVERSION;
	return std::string(W2A(bstr));
}

IFACEMETHODIMP CWebRTCAPI::pushToNative(BSTR bcmd, BSTR bjson)
{
	std::string cmd = BSTR2string(bcmd);
	std::string json = BSTR2string(bjson);

	Json::StyledWriter writer;
	Json::Value jsonobj(json);

	LOG(INFO) << gettime() + " push to native ***************************** " << json;

	if (cmd == "handleoffer")
		conductor_->ProcessOffer(json);
	else if (cmd == "handleanswer")
		conductor_->ProcessAnswer(json);
	else if (cmd == "hangup")
		conductor_->Hangup();
	else if (cmd == "makeoffer")
		conductor_->CreatOfferSDP();
	else if (cmd == "handlecandidate")
		conductor_->ProcessCandidate(json);
	else if (cmd == "debug")
	{
	#if WIN32
		::DebugBreak();
	#endif
	}
	else if (cmd == "seticeservers")
		conductor_->SetIceServers(json);
	return S_OK;
}

	/*
	else if (q > -1)
	{
		conductor_->Close();	// need this, or perhaps something like it
//well??		PostQuitMessage(0);
	}
	*/

void CWebRTCAPI::SendToBrowser(const std::string& json)		// from JavaScriptCallback
{
	BSTR bjson = Convert(json);
	Fire_EventToBrowser(bjson);
}

#ifdef DEBUG
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
#endif

IFACEMETHODIMP CWebRTCAPI::run()
{
#ifdef DEBUG
	logtofile();
#endif

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

	conductor_ = new talk_base::RefCountedObject<Conductor>(&client, &mainWindow);
	conductor_->SetJSCallback(this);

	// *** keep ***
	// display (only) the local video:
	//		conductor_->getlocalvideo(); 
	// but it does not restore the local video after a hangup (yet...)

	// Main loop.
	MSG msg;
	BOOL gm;

	while ((gm = ::GetMessage(&msg, NULL, 0, 0)) != 0 && gm != -1)
	{
		if (msg.message == WM_CLOSE || msg.message == 33176)
			break;

		if (!mainWindow.PreTranslateMessage(&msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	// here we have a working shutdown:

	try
	{
//		PostQuitMessage(0);			// don't do this

		client.SignOut();

		client.disconnect_all();

		conductor_->Close();

		talk_base::CleanupSSL();

// causes ie to crash on close		free(conductor_);

		CoUninitialize();
	} 
	catch (std::exception& ex)
	{
		LOG(LS_ERROR) << "Exception on shutdown " << ex.what();
	}

	return S_OK;
}

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
