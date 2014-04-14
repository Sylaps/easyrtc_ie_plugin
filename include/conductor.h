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

#ifndef PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
#define PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>

#include "main_wnd.h"
//#include "peer_connection_client.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/base/scoped_ptr.h"

namespace webrtc
{
	class VideoCaptureModule;
}  // namespace webrtc

namespace cricket
{
	class VideoRenderer;
}  // namespace cricket

class JavaScriptCallback		// aka BrowserCallback
{
public:
	virtual void SendToBrowser(const std::string& json) = 0;

protected:
	virtual ~JavaScriptCallback() { }
};

class Conductor
	: public webrtc::PeerConnectionObserver,
	public webrtc::CreateSessionDescriptionObserver,
	public webrtc::MediaConstraintsInterface,
	public MainWndCallback
{
public:
	enum CallbackID
	{
		MEDIA_CHANNELS_INITIALIZED = 1,
		PEER_CONNECTION_CLOSED,
		SEND_MESSAGE_TO_PEER,
		SEND_MESSAGE_TO_BROWSER,
		PEER_CONNECTION_ERROR,
		NEW_STREAM_ADDED,
		STREAM_REMOVED,
	};

	Conductor(MainWindow* main_wnd);

	bool connection_active() const;

	virtual void Close();

	// easyrtc incoming from JS
	void SetIceServers(std::string json);
	void CreateOfferSDP();
	void ProcessAnswer(std::string json);
	void ProcessOffer(std::string);
	void ProcessCandidate(std::string json);
	void Hangup();

	// we might want to just display local until a call is made
//	void getlocalvideo();	// just call InitializePeerConnection

	Constraints mandatory_;
	Constraints optional_;

	virtual const Constraints& GetMandatory() const
	{
		return mandatory_;
	}

	virtual const Constraints& GetOptional() const
	{
		return optional_;
	}

	void SetAllowDtlsSctpDataChannels()
	{
		mandatory_.push_back(Constraint(MediaConstraintsInterface::kEnableDtlsSrtp, "true"));
//		SetMandatory(MediaConstraintsInterface::kEnableDtlsSrtp, true);
	}

	void SetJSCallback(JavaScriptCallback *jsc)
	{
		javascriptCallback_ = jsc;
	}

protected:
	~Conductor();

	JavaScriptCallback *javascriptCallback_;

	bool InitializePeerConnection();	
	void DeletePeerConnection();
	void EnsureStreamingUI();
	void AddStreams();

	cricket::VideoCapturer* OpenVideoCaptureDevice();

	//
	// PeerConnectionObserver implementation.
	//
	virtual void OnError();
	virtual void OnStateChange(
		webrtc::PeerConnectionObserver::StateType state_changed)
	{
	}

	virtual void OnAddStream(webrtc::MediaStreamInterface* stream);

	virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);

	virtual void OnRenegotiationNeeded() { }

	virtual void OnIceChange() { }

	virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

	//
	// PeerConnectionClientObserver implementation.
	//

	virtual void OnSignedIn();

	virtual void OnDisconnected();

	virtual void OnPeerConnected(int id, const std::string& name);

	virtual void OnPeerDisconnected(int id);

	virtual void OnMessageFromPeer(int peer_id, const std::string& message);

	virtual void OnMessageSent(int err);

	virtual void OnServerConnectionFailure();

	//
	// MainWndCallback implementation.
	//

	virtual void StartLogin(const std::string& server, int port);	// old peer client/server code to connect to server

//	virtual void DisconnectFromServer();

//	virtual void ConnectToPeer(int peer_id);

//	virtual void DisconnectFromCurrentPeer();

	virtual void UIThreadCallback(int msg_id, void* data);

	// CreateSessionDescriptionObserver implementation.
	virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
	virtual void OnFailure(const std::string& error);

protected:
	// Send a message to the remote peer.
	void PostToBrowser(const std::string& json);

//	int peer_id_;
	talk_base::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface>peer_connection_factory_;

	MainWindow* mainWindow_;

	std::deque<std::string*> pending_messages_;
	std::map<std::string, talk_base::scoped_refptr<webrtc::MediaStreamInterface> > active_streams_;
	std::string iceCandidatesFromSS_;		// signaling server
	std::string server_;
};

#endif  // PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
