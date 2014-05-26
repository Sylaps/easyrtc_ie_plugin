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
#include "conductor.h"

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
	ASSERT(!IsWindow());
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
	if (IsWindow()) {
		ret = ::DestroyWindow(wnd_);
	}

	return ret != FALSE;
}

bool MainWnd::IsWindow() {
	return wnd_ && ::IsWindow(wnd_) != FALSE;
}

void MainWnd::MessageBox(const char* caption, const char* text, bool is_error) {
	DWORD flags = MB_OK;
	if (is_error)
		flags |= MB_ICONERROR;
	
	::MessageBoxA(handle(), text, caption, flags);
}

void MainWnd::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
	local_renderer_.reset(new VideoRenderer(wnd_, 1, 1, local_video));
}

void MainWnd::StopLocalRenderer() {
	local_renderer_.reset();
}

/*
 AddRemoteRenderer - Adds a renderer given a unique key and a video track. To succeed, there must be an 
 available HWND on the queue for this to consume as a rendering surface. (See AddRenderHandle)
*/
void MainWnd::AddRemoteRenderer(std::string key, webrtc::VideoTrackInterface* remote_video) {

	// pull a render handle off the global queue
	if (remote_render_handles_.size() > 0){
		HWND wnd = remote_render_handles_.front();
		remote_render_handles_.pop();

		// add an external renderer if we have a ptr to the hwnd for it
		auto iter = remote_renderers.find(key);
		if (iter == remote_renderers.end()) {
			// Wrap up the hwnd and VideoRenderer together so we can access the 
			// hwnd later, and assign it to this easyrtcid
			remote_renderers[key] =
				new ExternalRemoteRenderer(new VideoRenderer(wnd, 1, 1, remote_video), wnd);
		}
	}
	else {
		LOG(INFO) << " Unable to add remote renderer - no HWND's in the queue";
	}
}

//hackhackhack
void MainWnd::StopRemoteRenderers() {
	remote_renderers.clear();
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

void MainWnd::ProcessUICallback(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
}

void MainWnd::AddRenderHandle(uint32_t handle) {
	
	LOG(INFO) << __FUNCTION__ << " : Adding new RenderHandle : " << handle;
	
	HWND ptr = reinterpret_cast<HWND>(handle);

	if (ptr == NULL){
		LOG(INFO) << __FUNCTION__ << " : Passed a bad pointer.";
	}

	remote_render_handles_.push(ptr);

}

/*private*/ void MainWnd::renderToHwnd(HWND wnd, VideoRenderer* renderer){

	HDC externalDC = NULL;
	RECT externalRC;
	if (wnd){
		externalDC = ::GetDC(wnd);
	}

	if (renderer && wnd && externalDC){

		::GetClientRect(wnd, &externalRC);

		const BITMAPINFO& rvbmi = renderer->bmi();
		int rvheight = abs(rvbmi.bmiHeader.biHeight);		// Remove Video
		int rvwidth = rvbmi.bmiHeader.biWidth;

		const uint8* rvimage = renderer->image();

		if (rvimage != NULL) {
			HDC dc_mem = ::CreateCompatibleDC(externalDC);
			::SetStretchBltMode(dc_mem, HALFTONE);

			// Set the map mode so that the ratio will be maintained for us.
			HBITMAP bmp_mem = ::CreateCompatibleBitmap(externalDC, externalRC.right, externalRC.bottom);
			HGDIOBJ bmp_old = ::SelectObject(dc_mem, bmp_mem);

			POINT logical_area = { externalRC.right, externalRC.bottom };
			DPtoLP(externalDC, &logical_area, 1);
			HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));

			int thumb_width = 100;
			int thumb_height = 75;

			// float aspect = (float)rvheight / (float)rvwidth; // e.g. 3/4

			HDC all_dc[] = { externalDC, dc_mem };
			for (int i = 0; i < ARRAY_SIZE(all_dc); ++i) {
				SetMapMode(all_dc[i], MM_ISOTROPIC);
				SetWindowExtEx(all_dc[i], logical_area.x, logical_area.y, NULL);
				SetViewportExtEx(all_dc[i], externalRC.right, externalRC.bottom, NULL);
			}

			RECT logical_rect = { 0, 0, logical_area.x, logical_area.y };
			::FillRect(dc_mem, &logical_rect, brush);
			::DeleteObject(brush);

			int x = (logical_area.x / 2) - (rvwidth / 2);		// what does this do besides nothing?
			int y = (logical_area.y / 2) - (rvheight / 2);
			x = 0;
			y = 0;

			StretchDIBits(dc_mem, x, y, rvwidth, rvheight,
				0, 0, rvwidth, rvheight, rvimage, &rvbmi, DIB_RGB_COLORS, SRCCOPY);

			BitBlt(externalDC, 0, 0, logical_area.x, logical_area.y, dc_mem, 0, 0, SRCCOPY);

			// Cleanup.
			::SelectObject(dc_mem, bmp_old);
			::DeleteObject(bmp_mem);
			::DeleteDC(dc_mem);
		}
	}


	if (wnd && externalDC){
		::ReleaseDC(wnd, externalDC);
	}

}

std::string* MainWnd::GetSelfie(){
	VideoRenderer* local_renderer = local_renderer_.get();
	if (local_renderer) {
		const uint8* image = local_renderer->image();
		BITMAPINFO bmi = local_renderer->bmi();
		return encodeImage(image, bmi);
	}
	return nullptr;
}




void MainWnd::OnPaint() {

	VideoRenderer* local_renderer = local_renderer_.get();

	// render remote surfaces
	for (auto iter : remote_renderers){
		VideoRenderer* renderTarget = iter.second->videoRenderer;
		renderToHwnd(iter.second->hwnd, renderTarget);
	}

	PAINTSTRUCT ps;
	::BeginPaint(handle(), &ps);
		
	RECT rc;
	::GetClientRect(handle(), &rc);

	if (local_renderer) {
		AutoLock<VideoRenderer> local_lock(local_renderer);

		const BITMAPINFO& bmi = local_renderer->bmi();
		int height = abs(bmi.bmiHeader.biHeight);
		int width = bmi.bmiHeader.biWidth;

		const uint8* image = local_renderer->image();
		if (image != NULL) {
			HDC dc_mem = ::CreateCompatibleDC(ps.hdc);
			::SetStretchBltMode(dc_mem, HALFTONE);

			// Set the map mode so that the ratio will be maintained for us.
			HDC all_dc[] = { ps.hdc, dc_mem };

			for (int i = 0; i < ARRAY_SIZE(all_dc); ++i) {
				SetMapMode(all_dc[i], MM_ISOTROPIC);
				SetWindowExtEx(all_dc[i], width, height, NULL);
				SetViewportExtEx(all_dc[i], rc.right, rc.bottom, NULL);
			}

			HBITMAP bmp_mem = ::CreateCompatibleBitmap(ps.hdc, rc.right, rc.bottom);
			HGDIOBJ bmp_old = ::SelectObject(dc_mem, bmp_mem);

			POINT logical_area = { rc.right, rc.bottom };
			DPtoLP(ps.hdc, &logical_area, 1);

			HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
			RECT logical_rect = { 0, 0, logical_area.x, logical_area.y };
			::FillRect(dc_mem, &logical_rect, brush);
			::DeleteObject(brush);

			int x = (logical_area.x / 2) - (width / 2);
			int y = (logical_area.y / 2) - (height / 2);

			StretchDIBits(dc_mem, x, y, width, height,
				0, 0, width, height, image, &bmi, DIB_RGB_COLORS, SRCCOPY);

			BitBlt(ps.hdc, 0, 0, logical_area.x, logical_area.y,
				dc_mem, 0, 0, SRCCOPY);

			// Cleanup.
			::SelectObject(dc_mem, bmp_old);
			::DeleteObject(bmp_mem);
			::DeleteDC(dc_mem);
		}
	}
	::EndPaint(handle(), &ps); 
	
}

void MainWnd::OnDestroyed() {
	PostQuitMessage(0);
}


MainWnd::VideoRenderer::VideoRenderer(
	HWND wnd, int width, int height,
	webrtc::VideoTrackInterface* track_to_render)
	: wnd_(wnd), rendered_track_(track_to_render) {

	::InitializeCriticalSection(&buffer_lock_);
	ZeroMemory(&bmi_, sizeof(bmi_));
	bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi_.bmiHeader.biPlanes = 1;
	bmi_.bmiHeader.biBitCount = 32;
	bmi_.bmiHeader.biCompression = BI_RGB;
	bmi_.bmiHeader.biWidth = width;
	bmi_.bmiHeader.biHeight = -height;
	bmi_.bmiHeader.biSizeImage = width * height * (bmi_.bmiHeader.biBitCount >> 3);
	rendered_track_->AddRenderer(this);
}

MainWnd::VideoRenderer::~VideoRenderer() {
	rendered_track_->RemoveRenderer(this);
	::DeleteCriticalSection(&buffer_lock_);
}

void MainWnd::VideoRenderer::SetSize(int width, int height) {
	AutoLock<VideoRenderer> lock(this);

	bmi_.bmiHeader.biWidth = width;
	bmi_.bmiHeader.biHeight = -height;
	bmi_.bmiHeader.biSizeImage = width * height *
		(bmi_.bmiHeader.biBitCount >> 3);
	image_.reset(new uint8[bmi_.bmiHeader.biSizeImage]);
}

void MainWnd::VideoRenderer::RenderFrame(const cricket::VideoFrame* frame) {
	if (!frame)
		return;

	AutoLock<VideoRenderer> lock(this);

	ASSERT(image_.get() != NULL);
	frame->ConvertToRgbBuffer(cricket::FOURCC_ARGB,
		image_.get(),
		bmi_.bmiHeader.biSizeImage,
		bmi_.bmiHeader.biWidth *
		bmi_.bmiHeader.biBitCount / 8);

	InvalidateRect(wnd_, NULL, TRUE);
}

