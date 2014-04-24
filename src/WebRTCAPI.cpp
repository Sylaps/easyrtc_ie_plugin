#include "stdafx.h"

#include "talk/base/json.h"
#include "talk/base/ssladapter.h"
#include "talk/base/win32socketinit.h"
#include "talk/base/win32socketserver.h"
#include "conductor.h"
#include "main_wnd.h"

#include "WebRTCAPI.h"

#define DEBUG_WEBRTC

extern const std::string gettime();

//TODO: remove 
IFACEMETHODIMP CWebRTCAPI::hello(BSTR *pRet) {
	// Allocate memory for the string.
	*pRet = ::SysAllocString(L"HelloWorld from C++");
	return pRet ? S_OK : E_OUTOFMEMORY;
}

void showDebugAlert(LPCWSTR caption, LPCWSTR text) {
#ifdef DEBUG
//	MessageBox(NULL, text, caption, MB_OK | MB_SYSTEMMODAL);
#endif // DEBUG
}

// util
std::string BSTR2string(BSTR bstr) {
	USES_CONVERSION;
	return std::string(W2A(bstr));
}

IFACEMETHODIMP CWebRTCAPI::pushToNative(BSTR bcmd, BSTR bjson) {
	std::string cmd = BSTR2string(bcmd);
	std::string json = BSTR2string(bjson);

	Json::StyledWriter writer;
	Json::Value jsonobj(json);

	LOG(INFO) << "\n" << gettime() + " push to native +++++++++++++++++++\n" << cmd << "\n" << json;

	if (cmd == "seticeservers")
		conductor_->SetIceServers(json);
	else if (cmd == "handleoffer")
		conductor_->ProcessOffer(json);
	else if (cmd == "handleanswer")
		conductor_->ProcessAnswer(json);
	else if (cmd == "hangup")
		conductor_->Hangup();
	else if (cmd == "makeoffer")
		conductor_->CreateOfferSDP();
	else if (cmd == "handlecandidate")
		conductor_->ProcessCandidate(json);
	else if (cmd == "debug")
	{
	#if WIN32
		::DebugBreak();
	#endif
	}
	return S_OK;
}

void CWebRTCAPI::SendToBrowser(const std::string& json)	{
	BSTR bjson = Convert(json);
	Fire_EventToBrowser(bjson);
}

#ifdef DEBUG
void logtofile() {
//	std::string log = "verbose";
	talk_base::LogMessage::LogToDebug(talk_base::LS_INFO);

	talk_base::FileStream *fs = new talk_base::FileStream();
	if (!fs->Open("WebRtcAx.log", "w", NULL)) {
		LOG(INFO) << "Could not open file";
	}
	else {
		talk_base::LogMessage::LogToStream(fs, talk_base::LS_INFO);
	}
}
#endif


talk_base::Win32Thread w32_thread;

// glorified Ctor 
IFACEMETHODIMP CWebRTCAPI::run() {

#ifdef DEBUG
	logtofile();
#endif
	
	talk_base::SetRandomTestMode(true); 
	talk_base::EnsureWinsockInit();
	
	DWORD ui_thread_id_ = ::GetCurrentThreadId();
	
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hr) {
		LOG(INFO) << "Coinitialze failed +++++++++++++++";
	}

	talk_base::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

	controlHwnd = m_hWnd;

	if (!talk_base::InitializeSSL(NULL) || !talk_base::InitializeSSLThread()) {
		LOG(LS_ERROR) << "error failed to init ssl";
	}
	
	conductor_->SetJSCallback(this);

	if (controlHwnd == NULL || !mainWindow.Create(controlHwnd, ui_thread_id_))	{
		showDebugAlert(_T("Error in my code :("), _T("hwnd is null!"));
		return S_OK;
	}

	return S_OK;
}

LRESULT CWebRTCAPI::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	mainWindow.OnPaint();
	return 0;
}

LRESULT CWebRTCAPI::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return S_OK;
}

LRESULT CWebRTCAPI::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	mainWindow.Destroy();
	return S_OK;
}

LRESULT CWebRTCAPI::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	mainWindow.ProcessUICallback(uMsg, wParam, lParam, bHandled);
	return S_OK;
}