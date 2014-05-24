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

	LOG(INFO) << "\n" << gettime() + " push to native +++++++++++++++++++\n" << cmd << "\n" << json;

	Json::StyledWriter writer;
	Json::Value jsonobj;
	Json::Reader reader;

	if (cmd == "getWindowHandle") {
		SendWindowHandle(m_hWnd);
		return S_OK;
	} 

	if (cmd == "getSelfie") {
		SendSelfie();
		return S_OK;
	}

#if _DEBUG
	if (!reader.parse(json, jsonobj)){
		if (cmd == "debug") {
			::DebugBreak();
		}
		return S_OK;
	}
#endif

	if (cmd == "addRenderHandle") {
		// TODO:
		// can this be done merely with a mutex? ctors could reach to compilation-unit state
		// notion of master/slave needed (if there's none, you're the master, else send your handle to the master)
		uint32_t handle = jsonobj["handle"].asUInt();
		mainWindow->AddRenderHandle(handle);
		return S_OK;
	}

	if (cmd == "seticeservers") {
		mainWindow->SetIceServers(json);
		return S_OK;
	}

	//Json dependent path

	Conductor* conductor = NULL;
	std::string easyRtcId = jsonobj["remoteId"].asString();

	// create a  new conductor for this connection if it doesn't exist
	auto finder = conductors.find(easyRtcId);
	if (finder == conductors.end()){
		std::string iceServers = mainWindow->GetIceServers();
		LOG(INFO) << "Found ice servers: " << iceServers;

		if (iceServers == ""){
			::DebugBreak();
		}
		conductor = new talk_base::RefCountedObject<Conductor>(easyRtcId, mainWindow, peer_connection_factory_, mainWindow->GetIceServers());

		conductor->SetJSCallback(this);
		conductor->AddRef();

		conductors[easyRtcId] = conductor;
		conductor->AddRef();

	} else {
		//just use it
		conductor = conductors[easyRtcId];
	}

	if (cmd == "handleoffer") {
		conductor->ProcessOffer(json);
	}
	else if (cmd == "handleanswer") {
		conductor->ProcessAnswer(json);
	}
	else if (cmd == "hangup"){
		conductor->Hangup();
	}
	else if (cmd == "makeoffer"){
		conductor->CreateOfferSDP();
	}
	else if (cmd == "handlecandidate") {
		conductor->ProcessCandidate(json);
	}

	return S_OK;
}



void CWebRTCAPI::SendSelfie(){
	std::string* base64bitmap = mainWindow->GetSelfie();

	if (base64bitmap && *base64bitmap != "") {
		Json::StyledWriter writer;
		Json::Value json;
		Json::Value pluginMessage;

		pluginMessage["data"] = *base64bitmap;
		pluginMessage["message"] = "gotSelfie";
		json["pluginMessage"] = pluginMessage;

		SendToBrowser(writer.write(json));
		delete base64bitmap;
	}
}

void CWebRTCAPI::SendWindowHandle(HWND wnd) {
	LOG(INFO) << __FUNCTION__;
	ASSERT(wnd != NULL);
	uint32_t wndPtr = reinterpret_cast<uint32_t>(wnd);

	Json::StyledWriter writer;
	Json::Value json;
	Json::Value pluginMessage;

	pluginMessage["data"] = wndPtr;
	pluginMessage["message"] = "gotWindowHandle";
	json["pluginMessage"] = pluginMessage;

	//std::string* str = new std::string(writer.write(json));

	SendToBrowser(writer.write(json) /* *str */);
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
	if (!hr) {
		LOG(INFO) << "CoInitializeEx failed with COINIT_MULTITHREADED";
	}

	talk_base::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

	controlHwnd = m_hWnd;

	if (!talk_base::InitializeSSL(NULL) || !talk_base::InitializeSSLThread()) {
		LOG(LS_ERROR) << "error failed to init ssl";
	}

	if (controlHwnd == NULL || !mainWindow->Create(controlHwnd, ui_thread_id_))	{
		showDebugAlert(_T("Error in my code :("), _T("hwnd is null!"));
		return S_OK;
	}

	return S_OK;
}

LRESULT CWebRTCAPI::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	mainWindow->OnPaint();
	return 0;
}

LRESULT CWebRTCAPI::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return S_OK;
}

LRESULT CWebRTCAPI::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LOG(INFO) << __FUNCTION__;
	if (peer_connection_factory_){
		peer_connection_factory_->Release();
		peer_connection_factory_.release();
	}
	for (auto c : conductors){
		Conductor * cd = c.second;
		c.second->Close();
		delete cd;
	}
	conductors.clear();
	
	mainWindow->StopCapture();

	return S_OK;
}

LRESULT CWebRTCAPI::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LOG(INFO) << __FUNCTION__;
 	return S_OK;
}

LRESULT CWebRTCAPI::OnOtherDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LOG(INFO) << __FUNCTION__;
	return S_OK;
}

LRESULT CWebRTCAPI::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (uMsg == MainWnd::WindowMessages::UI_THREAD_CALLBACK)	{
		//Find out which conductor this was sent from
		ConductorCallback* cb = reinterpret_cast<ConductorCallback*>(lParam);

		auto it = conductors.find(cb->easyRtcId_);
		if (it != conductors.end()) {
			it->second->UIThreadCallback(static_cast<int>(wParam), cb->data_);
		}
		else {
			LOG(INFO) << " Could not find Conductor for given id:" << cb->easyRtcId_;
		}
	}
	return S_OK;
}

