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

#include "stdafx.h"
#include "conductor.h"
#include <utility>
#include <algorithm> 
//#include <functional> 
//#include <locale>
//#include <cctype>
#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "talk/base/common.h"
#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "defaults.h"
#include "talk/media/devices/devicemanager.h"

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

const std::string gettime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

class DummySetSessionDescriptionObserver
	: public webrtc::SetSessionDescriptionObserver
{
public:
	static DummySetSessionDescriptionObserver* Create()
	{
		return
			new talk_base::RefCountedObject<DummySetSessionDescriptionObserver>();
	}
	virtual void OnSuccess()
	{
		LOG(INFO) << __FUNCTION__;
	}
	virtual void OnFailure(const std::string& error)
	{
		LOG(INFO) << __FUNCTION__ << " " << error;
	}

protected:
	DummySetSessionDescriptionObserver()
	{
	}
	~DummySetSessionDescriptionObserver()
	{
	}
};

Conductor::Conductor(MainWindow* main_wnd)
	: mainWindow_(main_wnd)
{
	main_wnd->RegisterObserver(this);
	SetAllowDtlsSctpDataChannels();
}

Conductor::~Conductor()
{
	if (peer_connection_ != NULL)
		if (peer_connection_.get() != NULL)
			LOG(LS_ERROR) << "leak, clean up peer_connection...";
}

bool Conductor::connection_active() const
{
	return peer_connection_.get() != NULL;
}

void Conductor::Close()
{
	DeletePeerConnection();
}

void Conductor::ProcessCandidate(std::string json)
{
	OnMessageFromPeer(0, json);
}

void Conductor::ProcessOffer(std::string remotesdp)	
{
	OnMessageFromPeer(0, remotesdp);	
}

void Conductor::Hangup()	
{
	if (peer_connection_.get())
	{

		DeletePeerConnection();
	}
}

void Conductor::ProcessAnswer(std::string remotesdp)
{
	std::string type = "answer";
	std::string json_object;

	if (peer_connection_ == NULL)
		InitializePeerConnection();

	webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, remotesdp));
	peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
}

/*
void Conductor::getlocalvideo()	
{
	InitializePeerConnection();
}
*/

void Conductor::SetIceServers(std::string icejson)
{
	iceCandidatesFromSS_ = icejson;
}

void Conductor::CreateOfferSDP()	
{
	LOG(INFO) << "createoffer ***********************";

	if (peer_connection_ == NULL)
		InitializePeerConnection();

	peer_connection_->CreateOffer(this, NULL);
}

std::string trim(std::string str)
{
	str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
	str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
	return str;
}

bool Conductor::InitializePeerConnection()
{
	LOG(INFO) << "\n InitializePeerConnection ***********************";
	ASSERT(peer_connection_factory_.get() == NULL);
	ASSERT(peer_connection_.get() == NULL);

	peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();

	if (!peer_connection_factory_.get())
	{
		mainWindow_->MessageBox("Error", "Failed to initialize PeerConnectionFactory", true);
		DeletePeerConnection();
		return false;
	}

	webrtc::PeerConnectionInterface::IceServers servers;

	Json::Reader reader;
	Json::Value jice;
	if (iceCandidatesFromSS_.length() > 0 && !reader.parse(iceCandidatesFromSS_, jice))
	{
		LOG(WARNING) << "Received unknown message. " << iceCandidatesFromSS_;
		return false;
	}

	LOG(INFO) << "incoming ice setup:";
	for (unsigned int i = 0; i < jice.size(); i++)
	{
		Json::Value jobj = jice[i];

		webrtc::PeerConnectionInterface::IceServer server; // = new webrtc::PeerConnectionInterface::IceServer();
		if (!jobj.isMember("url"))
			continue;

		server.uri = trim(jobj["url"].toStyledString());
		if (jobj.isMember("username"))
			server.username = trim(jobj["username"].toStyledString());
		if (jobj.isMember("credential"))
			server.password = trim(jobj["credential"].toStyledString());

		LOG(INFO) << "c++ " << server.uri << "|" << server.username << "|" << server.password;
		servers.push_back(server);

		server.username = "";
		server.password = "";
	}

//	webrtc::PeerConnectionInterface::IceServer server;
//	server.uri = "stun:stun.l.google.com:19302";
//	servers.push_back(server);

	peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers, this, NULL, this); // NULL: media constraints

	if (!peer_connection_.get())
	{
		mainWindow_->MessageBox("Error", "CreatePeerConnection failed", true);
		DeletePeerConnection();
	}

	AddStreams();
	return peer_connection_.get() != NULL;
}

void Conductor::DeletePeerConnection()
{
	peer_connection_ = NULL;	// doing this later cause the last video image to remain on screen
	active_streams_.clear();
	mainWindow_->StopLocalRenderer();
	mainWindow_->StopRemoteRenderer();
	peer_connection_factory_ = NULL;	
}

void Conductor::EnsureStreamingUI()
{
	ASSERT(peer_connection_.get() != NULL);
	if (mainWindow_->IsWindow())
	{
//		if (mainWindow_->current_ui() != MainWindow::STREAMING)
//			mainWindow_->SwitchToStreamingUI();
	}
}

//
// PeerConnectionObserver implementation.
//

void Conductor::OnError()
{
	LOG(LS_ERROR) << __FUNCTION__;
	mainWindow_->QueueUIThreadCallback(PEER_CONNECTION_ERROR, NULL);
}

// Called when a remote stream is added
void Conductor::OnAddStream(webrtc::MediaStreamInterface* stream)
{
	LOG(INFO) << __FUNCTION__ << " " << stream->label();

	stream->AddRef();
	mainWindow_->QueueUIThreadCallback(NEW_STREAM_ADDED, stream);
}

void Conductor::OnRemoveStream(webrtc::MediaStreamInterface* stream)
{
	LOG(INFO) << __FUNCTION__ << " " << stream->label();
	stream->AddRef();
	mainWindow_->QueueUIThreadCallback(STREAM_REMOVED, stream);
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
	Json::StyledWriter writer;
	Json::Value jmessage;

	jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
	jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
	std::string sdp;
	if (!candidate->ToString(&sdp))
	{
		LOG(LS_ERROR) << "Failed to serialize candidate";
		return;
	}
	jmessage[kCandidateSdpName] = sdp;
	PostToBrowser(writer.write(jmessage));
}

//
// PeerConnectionClientObserver implementation.
//

void Conductor::OnSignedIn()
{
	LOG(INFO) << __FUNCTION__;
}

void Conductor::OnDisconnected()
{
	LOG(INFO) << __FUNCTION__;

	DeletePeerConnection();
}

void Conductor::OnPeerConnected(int id, const std::string& name)
{
	LOG(INFO) << __FUNCTION__;
	// Refresh the list if we're showing it.
//	if (mainWindow_->current_ui() == MainWindow::LIST_PEERS)
//		mainWindow_->SwitchToPeerList(peerConnectionClient_->peers());
}

void Conductor::OnPeerDisconnected(int id)
{
	LOG(INFO) << __FUNCTION__;
//	if (id == peer_id_)
	{
		LOG(INFO) << "Our peer disconnected";
		mainWindow_->QueueUIThreadCallback(PEER_CONNECTION_CLOSED, NULL);
	}
//	else
	{
		// Refresh the list if we're showing it.
//		if (mainWindow_->current_ui() == MainWindow::LIST_PEERS)
//			mainWindow_->SwitchToPeerList(peerConnectionClient_->peers());
	}
}

void Conductor::OnMessageFromPeer(int notused, const std::string& message)
{
	LOG(INFO) << "OnMessageFromPeer: " << message;
	ASSERT(!message.empty());

	if (!peer_connection_.get())
	{
		if (!InitializePeerConnection())
		{
			LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
			return;
		}
	}

	Json::Reader reader;
	Json::Value jmessage;
	if (!reader.parse(message, jmessage))
	{
		LOG(WARNING) << "Received unknown message. " << message;
		return;
	}
	std::string type;
	std::string json_object;

	GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);
	if (!type.empty())
	{
		std::string sdp;
		if (!GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName, &sdp))
		{
			LOG(WARNING) << "Can't parse received session description message.";
			return;
		}
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp));
		if (!session_description)
		{
			LOG(WARNING) << "Can't parse received session description message.";
			return;
		}
		LOG(INFO) << " Received session description :" << message;
		peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
		if (session_description->type() == webrtc::SessionDescriptionInterface::kOffer)
			peer_connection_->CreateAnswer(this, NULL);
	}
	else
	{
		std::string sdp_mid;
		int sdp_mlineindex = 0;
		std::string sdp;

		// debug log if these are all there
		bool x = GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid);
		bool y = GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName, &sdp_mlineindex);
		bool z = GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp);
		LOG(INFO) << "OnMessageFromPeer() " << x << y << z;

		if (!GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid) ||
			!GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName, &sdp_mlineindex) || 
			!GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp))
		{
			LOG(WARNING) << "Can't parse received message.";
			return;
		}
		talk_base::scoped_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp));
		if (!candidate.get())
		{
			LOG(WARNING) << "Can't parse received candidate message.";
			return;
		}
		if (!peer_connection_->AddIceCandidate(candidate.get()))
		{
			LOG(WARNING) << "Failed to apply the received candidate";
			return;
		}
		LOG(INFO) << " Received candidate :" << message;
	}
}

void Conductor::OnMessageSent(int err)
{
	// Process the next pending message if any.
	mainWindow_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, NULL);
}

void Conductor::OnServerConnectionFailure()
{
	mainWindow_->MessageBox("Error", ("Failed to connect to " + server_).c_str(), true);
}

//
// MainWndCallback implementation.
//

// old peer client/server code to connect to server
void Conductor::StartLogin(const std::string& server, int port)
{
}


cricket::VideoCapturer* Conductor::OpenVideoCaptureDevice()
{
	talk_base::scoped_ptr<cricket::DeviceManagerInterface> dev_manager(
		cricket::DeviceManagerFactory::Create());
	if (!dev_manager->Init())
	{
		LOG(LS_ERROR) << "Can't create device manager";
		return NULL;
	}
	std::vector<cricket::Device> devs;
	if (!dev_manager->GetVideoCaptureDevices(&devs))
	{
		LOG(LS_ERROR) << "Can't enumerate video devices";
		return NULL;
	}
	std::vector<cricket::Device>::iterator dev_it = devs.begin();
	cricket::VideoCapturer* capturer = NULL;
	for (; dev_it != devs.end(); ++dev_it)
	{
		capturer = dev_manager->CreateVideoCapturer(*dev_it);
		if (capturer != NULL)
			break;
	}
	return capturer;
}

void Conductor::AddStreams()
{
	if (active_streams_.find(kStreamLabel) != active_streams_.end())
		return;  // Already added.

	talk_base::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
		peer_connection_factory_->CreateAudioTrack(
		kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL)));

	talk_base::scoped_refptr<webrtc::VideoTrackInterface> video_track(
		peer_connection_factory_->CreateVideoTrack(
		kVideoLabel,
		peer_connection_factory_->CreateVideoSource(OpenVideoCaptureDevice(),
		NULL)));
	mainWindow_->StartLocalRenderer(video_track);

	talk_base::scoped_refptr<webrtc::MediaStreamInterface> stream =
		peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);

	stream->AddTrack(audio_track);
	stream->AddTrack(video_track);
	if (!peer_connection_->AddStream(stream, NULL))
	{
		LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
	}
	typedef std::pair<std::string,
		talk_base::scoped_refptr<webrtc::MediaStreamInterface> >
		MediaStreamPair;
	active_streams_.insert(MediaStreamPair(stream->label(), stream));
}



void Conductor::UIThreadCallback(int msg_id, void* data)
{
	std::string* msg;
	switch (msg_id)
	{
	case PEER_CONNECTION_CLOSED:
		LOG(INFO) << "PEER_CONNECTION_CLOSED";
		DeletePeerConnection();

		ASSERT(active_streams_.empty());

		if (mainWindow_->IsWindow())
		{
		}
		else
		{
			//DisconnectFromServer();
		}
		break;

	case SEND_MESSAGE_TO_BROWSER:
		msg = reinterpret_cast<std::string*>(data);
		javascriptCallback_->SendToBrowser(*msg);
		LOG(INFO) << "\n+++++ send_message_to_browser thread " << *msg;
		delete msg;
		msg = NULL;
		break;

	case SEND_MESSAGE_TO_PEER:
		LOG(INFO) << "SEND_MESSAGE_TO_PEER";
		msg = reinterpret_cast<std::string*>(data);
		if (msg)
		{
			// For convenience, we always run the message through the queue.
			// This way we can be sure that messages are sent to the server
			// in the same order they were signaled without much hassle.
			pending_messages_.push_back(msg);
		}

		if (!pending_messages_.empty())
		{
			msg = pending_messages_.front();
			pending_messages_.pop_front();
			delete msg;
		}

		break;

	case PEER_CONNECTION_ERROR:
		mainWindow_->MessageBox("Error", "an unknown error occurred", true);
		break;

	case NEW_STREAM_ADDED: {
		webrtc::MediaStreamInterface* stream = reinterpret_cast<webrtc::MediaStreamInterface*>(data);
		webrtc::VideoTrackVector tracks = stream->GetVideoTracks();
		// Only render the first track.
		if (!tracks.empty())
		{
			webrtc::VideoTrackInterface* track = tracks[0];
			mainWindow_->StartRemoteRenderer(track);
		}
		stream->Release();
		break;
	}

	case STREAM_REMOVED: {
		// Remote peer stopped sending a stream.
		webrtc::MediaStreamInterface* stream = reinterpret_cast<webrtc::MediaStreamInterface*>( data);
		stream->Release();
		break;
	}

	default:
		ASSERT(false);
		break;
	}
}

// seems to be called twice
void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	LOG(INFO) << "\ngdh says: conductor - some kind of success";
	peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);
	Json::StyledWriter writer;
	Json::Value jmessage;
	jmessage[kSessionDescriptionTypeName] = desc->type();

	std::string sdp;
	desc->ToString(&sdp);

	jmessage[kSessionDescriptionSdpName] = sdp;
	PostToBrowser(writer.write(jmessage));			// <-- send to signal server (JS)
}

void Conductor::OnFailure(const std::string& error)
{
	LOG(LERROR) << error;
}

// inherited generic callback from CreateSessionDescriptionObserver (returns SDP and candidates)
void Conductor::PostToBrowser(const std::string& json_object)
{
	std::string* json = new std::string(json_object);
	LOG(INFO) << "+++++++++++++++++  SendMessage(): " + *json;


	// Threading issues present here, so we are trying out a direct call to javascript... fails as well  
	//mainWindow_->QueueUIThreadCallback(SEND_MESSAGE_TO_BROWSER, json);
	
	javascriptCallback_->SendToBrowser(*json); 
	
}
