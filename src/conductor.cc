#include "stdafx.h"
#include "conductor.h"
#include <utility>
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

Conductor::Conductor(PeerConnectionClient* client, MainWindow* main_wnd)
: peer_id_(-1), peerConnectionClient_(client), mainWindow_(main_wnd)
{
	peerConnectionClient_->RegisterObserver(this);
	main_wnd->RegisterObserver(this);
	SetAllowDtlsSctpDataChannels();
}

Conductor::~Conductor()
{
	// TODO fix this clean-up, should be null
//	ASSERT(peer_connection_.get() == NULL);
	if (peer_connection_ != NULL)
		if (peer_connection_.get() != NULL)
			LOG(LS_ERROR) << "\n *********************** leak, clean up peer_connection somehow...";
}

bool Conductor::connection_active() const
{
	return peer_connection_.get() != NULL;
}

void Conductor::Close()
{
	peerConnectionClient_->SignOut();
	DeletePeerConnection();
}

void Conductor::candidate(std::string json)		// gdh
{
	OnMessageFromPeer(0, json);
}

void Conductor::gotoffer(std::string remotesdp)		// gdh
{
	// try this:
/* 
this seems to do a lot, but vid connection not established
	std::string type = "offer";
	std::string json_object;

	if (peer_connection_ == NULL)
		InitializePeerConnection();

	webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, remotesdp));
	peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
*/

	/* 
	not sure

// Wed: well??
	if (peer_connection_ == NULL)
		InitializePeerConnection();
	SendMessage(remotesdp);
	*/

// how about this one: 

	OnMessageFromPeer(0, remotesdp);		// lets try the whole json...!

// ???	OnMessageFromPeer(0, json);

	//webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, remotesdp));
	//peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
}

void Conductor::hangup()		// gdh
{
	if (peer_connection_.get())
	{
		peerConnectionClient_->SendHangUp(peer_id_);
		DeletePeerConnection();
	}
}

void Conductor::gotanswer(std::string remotesdp)		// gdh
{
	std::string type = "answer";
	std::string json_object;

	if (peer_connection_ == NULL)
		InitializePeerConnection();

	webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, remotesdp));
	peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
		
//	mainWindow_->QueueUIThreadCallback(SEND_MESSAGE_TO_DUDE, 0);
}

void Conductor::getlocalvideo()		// gdh
{
LOG(INFO) << "\n getlocalvideo ***********************";
	//	gerryInitPeerForLocal();
	InitializePeerConnection();

	peer_id_ = 4;
	peer_connection_->CreateOffer(this, NULL);
}

void Conductor::createoffer()		// gdh
{
	//	do something
	peer_id_ = 4;
	LOG(INFO) << "\n createoffer ***********************";

		// both of these next 2 lines seem to work the same way
	if (peer_connection_ == NULL)
		InitializePeerConnection();
	peer_connection_->CreateOffer(this, NULL);

//	mainWindow_->QueueUIThreadCallback(SEND_MESSAGE_TO_DUDE, 0);
}

//jconst webrtc::MediaConstraintsInterface::Constraint mcic(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "true");
//jwebrtc::MediaConstraintsInterface::Constraints dude;
//jwebrtc::MediaConstraintsInterface::Constraints nothing;

// const webrtc::MediaConstraintsInterface::Constraints &GetMandatory(void);
// const webrtc::MediaConstraintsInterface::Constraints &GetOptional(void);

/*
const webrtc::MediaConstraintsInterface::Constraints& GetMandatory(void)
{
	if (dude.empty())
		dude.push_back(mcic);
	return dude;
}
const webrtc::MediaConstraintsInterface::Constraints& GetOptional(void)
{
	return nothing;
}
*/

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
	webrtc::PeerConnectionInterface::IceServer server;
	server.uri = GetPeerConnectionString();
	servers.push_back(server);


	/*
	webrtc::FakeConstraints pc_constraints;
	pc_constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "true");

	peer_connection = peer_connection_factory->CreatePeerConnection(m_STUNandTURNServers, &pc_constraints, NULL, &observer);
	if (!peer_connection.get())
	{
		LOGGER_ERROR(__TRACESECTION__, _T("  CreatePeerConnection failed!"));
		return -1;
	}

	CreatePeerConnection(
	const PeerConnectionInterface::IceServers& configuration,

			const webrtc::MediaConstraintsInterface* constraints,

	PortAllocatorFactoryInterface* allocator_factory,
	DTLSIdentityServiceInterface* dtls_identity_service,

	*/


	/*
	webrtc::MediaConstraintsInterface::Constraints::const_iterator iter;

	webrtc::MediaConstraintsInterface* constraints = 0;

	webrtc::MediaConstraintsInterface::Constraint mcickkk("dlkj", "dk");

	webrtc::MediaConstraintsInterface::Constraint mcic(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "true");
//	this->AddOptional(mcic);


//	webrtc::MediaConstraints pcmc;
//	pcmc.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, true);
//	webrtc::MediaConstraintsInterface *constraints = new webrtc::MediaConstraints("klsfj", "klsdfj");
	*/

	peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers, this, NULL, this);	// not really a peer connection

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
	peer_connection_ = NULL;
	active_streams_.clear();
	mainWindow_->StopLocalRenderer();
	mainWindow_->StopRemoteRenderer();
	peer_connection_factory_ = NULL;
	peer_id_ = -1;
}

void Conductor::EnsureStreamingUI()
{
	ASSERT(peer_connection_.get() != NULL);
	if (mainWindow_->IsWindow())
	{
		if (mainWindow_->current_ui() != MainWindow::STREAMING)
			mainWindow_->SwitchToStreamingUI();
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
	SendMessage(writer.write(jmessage));
}

//
// PeerConnectionClientObserver implementation.
//

void Conductor::OnSignedIn()
{
	LOG(INFO) << __FUNCTION__;
	mainWindow_->SwitchToPeerList(peerConnectionClient_->peers());
}

void Conductor::OnDisconnected()
{
	LOG(INFO) << __FUNCTION__;

	DeletePeerConnection();

	if (mainWindow_->IsWindow())
		mainWindow_->SwitchToConnectUI();
}

void Conductor::OnPeerConnected(int id, const std::string& name)
{
	LOG(INFO) << __FUNCTION__;
	// Refresh the list if we're showing it.
	if (mainWindow_->current_ui() == MainWindow::LIST_PEERS)
		mainWindow_->SwitchToPeerList(peerConnectionClient_->peers());
}

void Conductor::OnPeerDisconnected(int id)
{
	LOG(INFO) << __FUNCTION__;
	if (id == peer_id_)
	{
		LOG(INFO) << "Our peer disconnected";
		mainWindow_->QueueUIThreadCallback(PEER_CONNECTION_CLOSED, NULL);
	}
	else
	{
		// Refresh the list if we're showing it.
		if (mainWindow_->current_ui() == MainWindow::LIST_PEERS)
			mainWindow_->SwitchToPeerList(peerConnectionClient_->peers());
	}
}

void Conductor::OnMessageFromPeer(int peer_id, const std::string& message)
{
	LOG(INFO) << "OnMessageFromPeer: " << message;

//	ASSERT(peer_id_ == peer_id || peer_id_ == -1);
//	ASSERT(!message.empty());

	if (!peer_connection_.get())
	{
//		ASSERT(peer_id_ == -1);
		peer_id_ = peer_id;

		if (!InitializePeerConnection())
		{
			LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
			peerConnectionClient_->SignOut();
			return;
		}
	}
//	else if (peer_id != peer_id_)
//	{
//		ASSERT(peer_id_ != -1);
//		LOG(WARNING) << "Received a message from unknown peer while already in a conversation with a different peer.";
//		return;
//	}

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

		bool x = GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid);
		bool y = GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName, &sdp_mlineindex);
		bool z = GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp);
		LOG(INFO) << x << y << z;

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
	mainWindow_->MessageBox("Error", ("Failed to connect to " + server_).c_str(),
		true);
}

//
// MainWndCallback implementation.
//

void Conductor::StartLogin(const std::string& server, int port)
{
	if (peerConnectionClient_->is_connected())
		return;
	server_ = server;
	peerConnectionClient_->Connect(server, port, GetPeerName());
}

void Conductor::DisconnectFromServer()
{
	if (peerConnectionClient_->is_connected())
		peerConnectionClient_->SignOut();
}

void Conductor::ConnectToPeer(int peer_id)
{
	ASSERT(peer_id_ == -1);
	ASSERT(peer_id != -1);

	if (peer_connection_.get())
	{
		mainWindow_->MessageBox("Error",
			"We only support connecting to one peer at a time", true);
		return;
	}

	if (InitializePeerConnection())
	{
		peer_id_ = peer_id;
		peer_connection_->CreateOffer(this, NULL);
	}
	else
	{
		mainWindow_->MessageBox("Error", "Failed to initialize PeerConnection", true);
	}
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
	mainWindow_->SwitchToStreamingUI();
}

void Conductor::DisconnectFromCurrentPeer()
{
	LOG(INFO) << __FUNCTION__;
	if (peer_connection_.get())
	{
		peerConnectionClient_->SendHangUp(peer_id_);
		DeletePeerConnection();
	}

	if (mainWindow_->IsWindow())
		mainWindow_->SwitchToPeerList(peerConnectionClient_->peers());
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
			if (peerConnectionClient_->is_connected())
			{
				mainWindow_->SwitchToPeerList(peerConnectionClient_->peers());
			}
			else
			{
				mainWindow_->SwitchToConnectUI();
			}
		}
		else
		{
			DisconnectFromServer();
		}
		break;

	case SEND_MESSAGE_TO_BROWSER:
		msg = reinterpret_cast<std::string*>(data);			// TODO you gotta delete this msg
		javascriptCallback_->SendToBrowser(*msg);
		break;

//	case SEND_MESSAGE_TO_DUDE:
//		LOG(INFO) << "SEND_MESSAGE_TO_dude";
//		peer_connection_->CreateOffer(this, NULL);
//		break;

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

		if (!pending_messages_.empty() && !peerConnectionClient_->IsSendingMessage())
		{
			msg = pending_messages_.front();
			pending_messages_.pop_front();

			if (!peerConnectionClient_->SendToPeer(peer_id_, *msg) && peer_id_ != -1)
			{
				LOG(LS_ERROR) << "SendToPeer failed";
				DisconnectFromServer();
			}
			delete msg;
		}

		if (!peer_connection_.get())
			peer_id_ = -1;

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

//	LOG(INFO) << "\nhello local SDP:\n" << sdp;
//	int zeros = sdp.find("a=rtcp:1 IN IP4 0.0.0.0");		 // ignore a=rtcp:1 IN IP4 0.0.0.0
//	if (zeros == -1)
//	{
//		LOG(INFO) << "\n\n\ngot good sdp\n\n\n";
//	}

	jmessage[kSessionDescriptionSdpName] = sdp;
	SendMessage(writer.write(jmessage));			// <-- send to signal server (JS)
}

// this gets called twice, first time with bogus local IP, 2nd time with a good SDP
/*  gdh ignore this shit
void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	LOG(INFO) << "OnSuccess - expecting sdp";

	peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);
	Json::StyledWriter writer;
	Json::Value jmessage;
	jmessage[kSessionDescriptionTypeName] = desc->type();

	std::string sdp;
	desc->ToString(&sdp);
	jmessage[kSessionDescriptionSdpName] = sdp;

	int zeros = sdp.find("a=rtcp:1 IN IP4 0.0.0.0");		 // ignore a=rtcp:1 IN IP4 0.0.0.0
	if (zeros == -1)
	{
		LOG(INFO) << "\n\n\ngot sdp\n\n\n";
		SendMessage(writer.write(jmessage));
	}
	else
	{
		LOG(INFO) << "no (good) sdp yet";
		LOG(INFO) << sdp;
	}
}
*/

void Conductor::OnFailure(const std::string& error)
{
	LOG(LERROR) << error;
}

// inherited generic callback from CreateSessionDescriptionObserver (returns SDP and candidates)
void Conductor::SendMessage(const std::string& json_object)
{
	std::string* json = new std::string(json_object);
	LOG(INFO) << "gdh says: SendMessage(): " + *json;

// obsolete?	mainWindow_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, json);		// <-- send this json to JS

// gdh: also send to browser
	std::string* msg = new std::string(json_object);
	mainWindow_->QueueUIThreadCallback(SEND_MESSAGE_TO_BROWSER, msg);
}

/*
void Conductor::SendMessage(const std::string& json_object)
{
	std::string* msg = new std::string(json_object);

	// call google signal server:	mainWindow_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, msg);

	// may need to post???

	LOG(INFO) << "****************************** send to browser:";
	LOG(INFO) << json_object;

//	that->SendToBrowser(json_object);

	mainWindow_->QueueUIThreadCallback(SEND_MESSAGE_TO_UPJS, msg);

	LOG(INFO) << "sent to browser. ******************************";
}
*/