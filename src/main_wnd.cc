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

#include <math.h>

#include "talk/base/common.h"
#include "talk/base/logging.h"
#include "talk/base/win32socketserver.h"
#include "defaults.h"

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

MainWnd::MainWnd()
	: 
//ui_(CONNECT_TO_SERVER), 
	wnd_(NULL), destroyed_(false), callback_(NULL), nested_msg_(NULL) {
}

MainWnd::~MainWnd() {
	ASSERT(!IsWindow());
}

//TODO: move this into the class plz

bool MainWnd::Create(HWND hwnd, DWORD ui_tid_) {
	ASSERT(wnd_ == NULL);

	ui_thread_id_ = ui_thread_id_;
	wnd_ = hwnd;
	/*
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (!hr) {
		LOG(INFO) << "CoInitializeEx failed with COINIT_MULTITHREADED";
	}
*/
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

void MainWnd::RegisterObserver(MainWndCallback* callback) {
	callback_ = callback;
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
	local_renderer_.reset(new VideoRenderer(handle(), 1, 1, local_video));
}

void MainWnd::StopLocalRenderer() {
	local_renderer_.reset();
}

void MainWnd::StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video) {
	remote_renderer_.reset(new VideoRenderer(handle(), 1, 1, remote_video));
}

void MainWnd::StopRemoteRenderer() {
	remote_renderer_.reset();
}

void MainWnd::QueueUIThreadCallback(int msg_id, void* data) {
	
//	BOOL b = ::PostThreadMessage(ui_thread_id_, UI_THREAD_CALLBACK, static_cast<WPARAM>(msg_id), reinterpret_cast<LPARAM>(data));

	LRESULT b = ::SendNotifyMessage(wnd_, UI_THREAD_CALLBACK, static_cast<WPARAM>(msg_id), reinterpret_cast<LPARAM>(data));
	if (b) {
		LOG(INFO) << __FUNCTION__ << " failed to post to thread: " << ui_thread_id_;
	}
	else {
		LOG(INFO) << __FUNCTION__ << " posted to thread : " << ui_thread_id_;
	}
}

void MainWnd::ProcessUICallback(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (uMsg == UI_THREAD_CALLBACK)	{
		callback_->UIThreadCallback(static_cast<int>(wParam), reinterpret_cast<void*>(lParam));
	}
}

void MainWnd::OnPaint() {

	PAINTSTRUCT ps;
	::BeginPaint(handle(), &ps);

	RECT rc;
	::GetClientRect(handle(), &rc);

	VideoRenderer* local_renderer = local_renderer_.get();
	VideoRenderer* remote_renderer = remote_renderer_.get();

	if (remote_renderer && local_renderer) {

		AutoLock<VideoRenderer> local_lock(local_renderer);
		AutoLock<VideoRenderer> remote_lock(remote_renderer);

		const BITMAPINFO& rvbmi = remote_renderer->bmi();
		int rvheight = abs(rvbmi.bmiHeader.biHeight);		// Remove Vide
		int rvwidth = rvbmi.bmiHeader.biWidth;

		const uint8* rvimage = remote_renderer->image();

		if (rvimage != NULL) {
			HDC dc_mem = ::CreateCompatibleDC(ps.hdc);
			::SetStretchBltMode(dc_mem, HALFTONE);

			// Set the map mode so that the ratio will be maintained for us.
			HBITMAP bmp_mem = ::CreateCompatibleBitmap(ps.hdc, rc.right, rc.bottom);
			HGDIOBJ bmp_old = ::SelectObject(dc_mem, bmp_mem);

			POINT logical_area = { rc.right, rc.bottom };
			DPtoLP(ps.hdc, &logical_area, 1);
			HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));

			int thumb_width = 100;
			int thumb_height = 75;

			// float aspect = (float)rvheight / (float)rvwidth; // e.g. 3/4

			HDC all_dc[] = { ps.hdc, dc_mem };
			for (int i = 0; i < ARRAY_SIZE(all_dc); ++i) {
				SetMapMode(all_dc[i], MM_ISOTROPIC);
				SetWindowExtEx(all_dc[i], logical_area.x, logical_area.y, NULL);
				SetViewportExtEx(all_dc[i], rc.right, rc.bottom, NULL);
			}

			RECT logical_rect = { 0, 0, logical_area.x, logical_area.y };
			::FillRect(dc_mem, &logical_rect, brush);
			::DeleteObject(brush);

			int x = (logical_area.x / 2) - (rvwidth / 2);		// what does this do besides nothing?
			int y = (logical_area.y / 2) - (rvheight / 2);
			x = 0;
			y = 0;

/*			int StretchDIBits(
				_In_  HDC hdc,
				_In_  int XDest,				0
				_In_  int YDest,				0
				_In_  int nDestWidth,			remote vid width
				_In_  int nDestHeight,			remote vid height
				_In_  int XSrc,					0
				_In_  int YSrc,					0
				_In_  int nSrcWidth,			remote vid width
				_In_  int nSrcHeight,			remote vid height
				_In_  const VOID *lpBits,		actual bitmap
				_In_  const BITMAPINFO *lpBitsInfo,
				_In_  UINT iUsage,				DIB_RGB_COLORS
				_In_  DWORD dwRop				SRCCOPY
				);
*/

			StretchDIBits(dc_mem, x, y, rvwidth, rvheight,
				0, 0, rvwidth, rvheight, rvimage, &rvbmi, DIB_RGB_COLORS, SRCCOPY);

			if ((rc.right - rc.left) > 300) {
				const BITMAPINFO& lvbmi = local_renderer->bmi();
				const uint8* lvimage = local_renderer->image();
				// int thumb_width = bmi.bmiHeader.biWidth / 2;
				// int thumb_height = abs(bmi.bmiHeader.biHeight) / 2;

				if ((rc.right - rc.left) > 399)	{
					thumb_width = 140; //  160; 
					thumb_height = 105; //  120;
				}

				StretchDIBits(dc_mem,
					logical_area.x - thumb_width - 3,
					logical_area.y - thumb_height - 3,
					thumb_width, thumb_height,
					0, 0, lvbmi.bmiHeader.biWidth, abs(lvbmi.bmiHeader.biHeight),
					lvimage, &lvbmi, DIB_RGB_COLORS, SRCCOPY);
			}

			BitBlt(ps.hdc, 0, 0, logical_area.x, logical_area.y, dc_mem, 0, 0, SRCCOPY);

			// Cleanup.
			::SelectObject(dc_mem, bmp_old);
			::DeleteObject(bmp_mem);
			::DeleteDC(dc_mem);
		}
		else {
			// We're still waiting for the video stream to be initialized.
			HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
			::FillRect(ps.hdc, &rc, brush);
			::DeleteObject(brush);

			HGDIOBJ old_font = ::SelectObject(ps.hdc, GetDefaultFont());
			::SetTextColor(ps.hdc, RGB(0xff, 0xff, 0xff));
			::SetBkMode(ps.hdc, TRANSPARENT);

			std::string text(kConnecting);
			if (!local_renderer->image()) {
				text += kNoVideoStreams;
			}
			else {
				text += kNoIncomingStream;
			}
			::DrawTextA(ps.hdc, text.c_str(), -1, &rc,
				DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			::SelectObject(ps.hdc, old_font);
		}
	}
	else if (local_renderer && false) {
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
			for (int i = 0; i < ARRAY_SIZE(all_dc); ++i)
			{
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
	else {
		HBRUSH brush = ::CreateSolidBrush(::GetSysColor(COLOR_WINDOW));
		::FillRect(ps.hdc, &rc, brush);
		::DeleteObject(brush);

		LPCTSTR pszText = _T("EasyRTC.com IE Plugin waiting for a connection");
		TextOut(ps.hdc, 20, 20, pszText, lstrlen(pszText));
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

