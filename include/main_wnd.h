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
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PEERCONNECTION_ATL_CLIENT_MAIN_WND_H_
#define PEERCONNECTION_ATL_CLIENT_MAIN_WND_H_
#pragma once

#include <map>
#include <string>
#include <atlctl.h>


#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/base/win32.h"
#include "talk/media/base/mediachannel.h"
#include "talk/media/base/videocommon.h"
#include "talk/media/base/videoframe.h"
#include "talk/media/base/videorenderer.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/videosourceinterface.h"

#include "easy_rtc_video_renderer.h"

// Pure virtual interface for the main window.
// TODO: rename to something like rendering manager
class MainWindow
{
public:
	virtual ~MainWindow(){
	}

	virtual std::string* GetSelfie() = 0;
	virtual void SetVideoSource(talk_base::scoped_refptr<webrtc::VideoSourceInterface>) = 0;
	virtual talk_base::scoped_refptr<webrtc::VideoSourceInterface> GetVideoSource() = 0;
	virtual void SetAudioSource(talk_base::scoped_refptr<webrtc::AudioSourceInterface>) = 0;
	virtual talk_base::scoped_refptr<webrtc::AudioSourceInterface> GetAudioSource() = 0;
	virtual void StartLocalRenderer(JavaScriptCallback*, webrtc::VideoTrackInterface* local_video) = 0;
	virtual void StopLocalRenderer() = 0;
	virtual void AddRemoteRenderer(JavaScriptCallback*, std::string key, webrtc::VideoTrackInterface* remote_video) = 0;
	virtual void StopRemoteRenderers() = 0;
	virtual void StartCapture() = 0;
	virtual void StopCapture() = 0;
	virtual void QueueUIThreadCallback(std::string easyRtcId, int msg_id, void* data) = 0;
	virtual std::string GetIceServers() = 0;
};

#ifdef WIN32

class MainWnd : public MainWindow
{
public:
	
	enum WindowMessages {
		UI_THREAD_CALLBACK = WM_APP + 1,
	};

	enum RenderMode {
		MASTER,
		SLAVE
	};
	
	static const wchar_t kClassName[];

	MainWnd(talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf);
	~MainWnd();

	bool Create(HWND, DWORD);
	bool Destroy();

	void OnPaint();
	void CloseSources();

	cricket::VideoCapturer* OpenVideoCaptureDevice();	

	//virtual void MessageBox(const char* caption, const char* text, bool is_error);

	virtual std::string* GetSelfie();

	virtual void SetVideoSource(talk_base::scoped_refptr<webrtc::VideoSourceInterface> src){ video_source_ = src; }
	virtual talk_base::scoped_refptr<webrtc::VideoSourceInterface> GetVideoSource(){ return video_source_; }
	virtual void SetAudioSource(talk_base::scoped_refptr<webrtc::AudioSourceInterface> src){ audio_source_ = src; }
	virtual talk_base::scoped_refptr<webrtc::AudioSourceInterface> GetAudioSource(){ return audio_source_; }

	virtual void StartLocalRenderer(JavaScriptCallback* cb, webrtc::VideoTrackInterface* local_video);
	virtual void StopLocalRenderer();

	virtual void AddRemoteRenderer(JavaScriptCallback* cb, std::string key, webrtc::VideoTrackInterface* remote_videod);
	virtual void StopRemoteRenderers();

	virtual void StartCapture();
	virtual void StopCapture();

	virtual void QueueUIThreadCallback(std::string easyRtcId, int msg_id, void* data);

	void SetIceServers(std::string json) {
		iceServerCandidates_ = json;
	}

	virtual std::string GetIceServers(){
		return iceServerCandidates_;
	}	

protected:
	void OnDestroyed();

private:

	std::string iceServerCandidates_;

	talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
	talk_base::scoped_refptr<webrtc::VideoSourceInterface> video_source_;
	talk_base::scoped_refptr<webrtc::AudioSourceInterface> audio_source_;

	cricket::VideoCapturer* capturer;
	talk_base::scoped_ptr<EasyRTCVideoRenderer> local_renderer_;
	std::vector<EasyRTCVideoRenderer*> remote_renderers_;
	
	HWND wnd_;
	DWORD ui_thread_id_;
	RenderMode renderMode;

	bool destroyed_;
	void* nested_msg_;
	static ATOM wnd_class_;
};
#endif  // WIN32

#endif  // PEERCONNECTION_ATL_CLIENT_MAIN_WND_H_
