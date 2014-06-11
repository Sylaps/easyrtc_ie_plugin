
#ifndef PEERCONNECTION_ATL_CLIENT_DEVICE_CONTROLLER_H_
#define PEERCONNECTION_ATL_CLIENT_DEVICE_CONTROLLER_H_
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


class DeviceController
{
public:

	virtual ~DeviceController(){
	}

	virtual std::string* GetSelfie() = 0;
	virtual void SetVideoSource(talk_base::scoped_refptr<webrtc::VideoSourceInterface>) = 0;
	virtual talk_base::scoped_refptr<webrtc::VideoSourceInterface> GetVideoSource() = 0;
	virtual void SetAudioSource(talk_base::scoped_refptr<webrtc::AudioSourceInterface>) = 0;
	virtual talk_base::scoped_refptr<webrtc::AudioSourceInterface> GetAudioSource() = 0;
	virtual talk_base::scoped_refptr<webrtc::MediaStreamInterface> GetLocalMediaStream() = 0;
	virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
	virtual void StopLocalRenderer() = 0;
	virtual void AddRemoteRenderer(std::string key, webrtc::VideoTrackInterface* remote_video) = 0;
	virtual void StopRemoteRenderers() = 0;
	virtual void StartCapture() = 0;
	virtual void StopCapture() = 0;
	virtual void QueueUIThreadCallback(std::string easyRtcId, int msg_id, void* data) = 0;
	virtual std::string GetIceServers() = 0;

	enum CallbackID {
		MEDIA_CHANNELS_INITIALIZED = 1,
		PEER_CONNECTION_CLOSED,
		SEND_MESSAGE_TO_PEER,
		SEND_MESSAGE_TO_BROWSER,
		PEER_CONNECTION_ERROR,
		NEW_STREAM_ADDED,
		STREAM_REMOVED,
		SEND_EASY_RTC_ID
	};
};

#endif