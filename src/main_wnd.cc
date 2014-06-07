/*
* libjingle
* Copyright 2012, Google Inc.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTEuRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"

#include "main_wnd.h"
#include "peerconnection_wrapper.h"
#include "video_renderer.h"

#include <math.h>

#include "WebRTCAPI.h"

#include "talk/base/common.h"
#include "talk/base/logging.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/media/devices/devicemanager.h"

#include "talk/base/win32socketserver.h"
#include "talk/base/json.h"
#include "defaults.h"

#include "talk/media/webrtc/webrtcvideocapturer.h"

ATOM MainWnd::wnd_class_ = 0;
const wchar_t MainWnd::kClassName[] = L"WebRTC_MainWnd";

namespace {
	const char kConnecting[] = "Connecting... ";
	const char kNoVideoStreams[] = "(no video streams either way)";
	const char kNoIncomingStream[] = "(no incoming video)";

	void CalculateWindowSizeForText(HWND wnd, const wchar_t* text,
		size_t* width, size_t* height) {

		HDC dc = ::GetDC(wnd);
		RECT text_rc = { 0 };
		::DrawText(dc, text, -1, &text_rc, DT_CALCRECT | DT_SINGLELINE);
		::ReleaseDC(wnd, dc);
		RECT client, window;
		::GetClientRect(wnd, &client);
		::GetWindowRect(wnd, &window);

		*width = text_rc.right - text_rc.left;
		*width += (window.right - window.left) -
			(client.right - client.left);
		*height = text_rc.bottom - text_rc.top;
		*height += (window.bottom - window.top) -
			(client.bottom - client.top);
	}

	HFONT GetDefaultFont() {
		static HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
		return font;
	}

	std::string GetWindowText(HWND wnd) {
		char text[MAX_PATH] = { 0 };
		::GetWindowTextA(wnd, &text[0], ARRAYSIZE(text));
		return text;
	}

	void AddListBoxItem(HWND listbox, const std::string& str, LPARAM item_data) {
		LRESULT index = ::SendMessageA(listbox, LB_ADDSTRING, 0,
			reinterpret_cast<LPARAM>(str.c_str()));
		::SendMessageA(listbox, LB_SETITEMDATA, index, item_data);
	}

}  // namespace

MainWnd::MainWnd(talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf) :
	wnd_(NULL), destroyed_(false), nested_msg_(NULL), renderMode(MASTER), peer_connection_factory_(pcf) {
}

MainWnd::~MainWnd() {
}

cricket::VideoCapturer* MainWnd::OpenVideoCaptureDevice() {
	talk_base::scoped_ptr<cricket::DeviceManagerInterface> dev_manager(
		cricket::DeviceManagerFactory::Create());
	if (!dev_manager->Init()) {
		LOG(LS_ERROR) << "Can't create device manager";
		return NULL;
	}
	std::vector<cricket::Device> devs;
	if (!dev_manager->GetVideoCaptureDevices(&devs)) {
		LOG(LS_ERROR) << "Can't enumerate video devices";
		return NULL;
	}
	std::vector<cricket::Device>::iterator dev_it = devs.begin();
	cricket::VideoCapturer* capturer = NULL;
	LOG(INFO) << "Searching for video devices...";
	for (; dev_it != devs.end(); ++dev_it) {

		LOG(INFO) << "Device Found: " << dev_it->id << " : " << dev_it->name;
		capturer = dev_manager->CreateVideoCapturer(*dev_it);
		if (capturer != NULL) break;

	}
	LOG(INFO) << "Done searching for devices.";
	return capturer;
}

void MainWnd::CloseSources() {
	//cricket::VideoCapturer * cap = 


	if (video_source_) {
		video_source_.get()->Release();
		video_source_.release();
	}
	if (audio_source_) {
		audio_source_.get()->Release();
		audio_source_.release();
	}
	
}

void MainWnd::StartCapture() {
	if (!capturer) {
		capturer = OpenVideoCaptureDevice();
	}
	if (audio_source_ == NULL) {
		audio_source_ = peer_connection_factory_->CreateAudioSource(NULL);
		ASSERT(audio_source_ != NULL);
	}
	if (video_source_ == NULL) {
		video_source_ = peer_connection_factory_->CreateVideoSource(capturer, NULL);
		ASSERT(video_source_ != NULL);
	}
}

void MainWnd::StopCapture(){
	CloseSources();
	if (capturer){
		
		cricket::WebRtcVideoCapturer* c = reinterpret_cast<cricket::WebRtcVideoCapturer*>(capturer);
		c->Stop();

		delete capturer;
		capturer = NULL;
	}
}


bool MainWnd::Create(HWND hwnd, DWORD ui_tid_) {

	ASSERT(wnd_ == NULL);

	ui_thread_id_ = ui_thread_id_;
	wnd_ = hwnd;

	::SendMessage(wnd_, WM_SETFONT, reinterpret_cast<WPARAM>(GetDefaultFont()), TRUE);

	return wnd_ != NULL;
}

bool MainWnd::Destroy() {
	BOOL ret = FALSE;
	return ret != FALSE;
}

void MainWnd::StartLocalRenderer(JavaScriptCallback* cb, webrtc::VideoTrackInterface* local_video) {
	local_renderer_.reset(new EasyRTCVideoRenderer(cb, "local", 1, 1, local_video));
}

void MainWnd::StopLocalRenderer() {
	local_renderer_.reset();
}

/*
 AddRemoteRenderer - Adds a renderer given a unique key and a video track. To succeed, there must be an 
 available HWND on the queue for this to consume as a rendering surface. (See AddRenderHandle)
*/
void MainWnd::AddRemoteRenderer(JavaScriptCallback* cb, std::string key, webrtc::VideoTrackInterface* remote_video) {
	remote_renderers_.push_back(new EasyRTCVideoRenderer(cb, key, 1, 1, remote_video));
}

//hackhackhack
void MainWnd::StopRemoteRenderers() {
}

void MainWnd::QueueUIThreadCallback(std::string easyRtcId, int msg_id, void* data) {
	
//	BOOL b = ::PostThreadMessage(ui_thread_id_, UI_THREAD_CALLBACK, static_cast<WPARAM>(msg_id), reinterpret_cast<LPARAM>(data));
	ConductorCallback* cb = new ConductorCallback(easyRtcId, data);

	BOOL b = ::SendNotifyMessage(wnd_, UI_THREAD_CALLBACK, static_cast<WPARAM>(msg_id), reinterpret_cast<LPARAM>(cb));
	if (!b) {
		LOG(INFO) << __FUNCTION__ << " failed to post to thread: " << ui_thread_id_;
	}
	else {
		LOG(INFO) << __FUNCTION__ << " posted to thread : " << ui_thread_id_;
	}
}

std::string* MainWnd::GetSelfie(){
	EasyRTCVideoRenderer* local_renderer = local_renderer_.get();
	if (local_renderer) {
		const uint8* image = local_renderer->image();
		BITMAPINFO bmi = local_renderer->bmi();
		return encodeImage(image, bmi);
	}
	return nullptr;
}

void MainWnd::OnPaint() {
}

void MainWnd::OnDestroyed() {
	PostQuitMessage(0);
}

