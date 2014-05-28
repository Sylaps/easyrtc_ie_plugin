#ifndef PEERCONNECTION_VIDEO_RENDERER_CLIENT_H_
#define PEERCONNECTION_VIDEO_RENDERER_CLIENT_H_
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
#include "javascript_callback.h"


class EasyRTCVideoRenderer : public webrtc::VideoRendererInterface	{

public:
	EasyRTCVideoRenderer(JavaScriptCallback*, std::string, int, int, webrtc::VideoTrackInterface*);

	std::string easyrtcid_;
	JavaScriptCallback* callback_; 

	virtual ~EasyRTCVideoRenderer();

	void Lock() {
		::EnterCriticalSection(&buffer_lock_);
	}

	void Unlock() {
		::LeaveCriticalSection(&buffer_lock_);
	}

	// VideoRendererInterface implementation
	virtual void SetSize(int width, int height);
	virtual void RenderFrame(const cricket::VideoFrame* frame);

	const BITMAPINFO& bmi() const {
		return bmi_;
	}
	const uint8* image() const {
		return image_.get();
	}

protected:
	enum {
		SET_SIZE,
		RENDER_FRAME,
	};

	//HWND wnd_; // refactor no longer rendering to hwnd
	BITMAPINFO bmi_;
	talk_base::scoped_ptr<uint8[]> image_;
	CRITICAL_SECTION buffer_lock_;
	talk_base::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
};

// A little helper class to make sure we always to proper locking and
// unlocking when working with VideoRenderer buffers.
template <typename T>
class AutoLock {
public:
	explicit AutoLock(T* obj) : obj_(obj) {
		obj_->Lock();
	}
	~AutoLock() {
		obj_->Unlock();
	}
protected:
	T* obj_;
};

#endif