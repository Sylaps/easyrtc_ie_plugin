
var socket = io.connect('dev.easyrtc.com:80');
//var socket = io.connect(':8080');
//var socket = io.connect();

var sdp = null;
var activex = null;
var myrtcid = null;
var remotertcid = null;
var appname = 'easyrtc.audioVideo';
var icy = null;

//function easytest() {
//	console.log('do not die please');
//}

function auth() {
	console.log('auth!');
	socket.emit('easyrtcAuth', authjson, authorizationCallback);
	console.log('authed');
}

function register(ax) {
	document.getElementById('divLog').innerHTML += '<hr>register()<br>';
	activex = ax;
	window.RTCPeerConnection = RTCPeerConnection; 
	
// TODO careful with 'class' vs 'static'; see prototype for adding methods, etc etc....
//
	var something = new RTCPeerConnection({ "iceServers": [] }, null);

	addEventHandler('WebRTCAPI', asyncEvent);
}

var authjson = { "msgType" : "authenticate", "msgData" : {"apiVersion" : "1.0.10", "applicationName" : "undefined" } };

var hangupjson = { "msgType" : "hangup", "targetEasyrtcid" : "undefined" };

var offerjson = { "msgType" : "offer",
		"targetEasyrtcid" : "offer_easyrtc_todo",
		"msgData" : 
		{ "sdp" : "", "type" : "offer" }};

var answerjson = { "msgType" : "answer",
		"targetEasyrtcid" : "answer_easyrtc_todo",
		"msgData" : 
		{ "sdp" : "", "type" : "answer" }};

var easycandidate = {
  "msgType": "candidate",
  "targetEasyrtcid": "",
  "msgData": {
    "type": "candidate",
    "label": "LABEL",
    "id": 0,
    "candidate": "0\r\n"
  }
}

var goocandidate = {
  "candidate": "a=candidate:1150697756 1 udp 2122129151 172.16.40.108 61853 typ host generation 0\r\n",
  "sdpMLineIndex": 0,
  "sdpMid": "audio"
}

function addEventHandler(id, handler) {
  try {
   var evalString = "function " + id + "::EventToBrowser (event) { handler(event); }"; // Eval bind double-colon function to call handler
   console.log(evalString);
   eval(evalString);
  } 
  catch (e) {
   console.log(e);
   throw new Error("Could not add custom event handler to "+ id);
  }
 }

var ss = 1;
function asyncEvent(value) {

	console.log(typeof value == 'string' ? 'str:' + value : 'obj:' + JSON.stringify(value));

	var json = typeof value == 'string' ? JSON.parse(value) : value;

	console.log('\n+++++ asyncEvent() ' + JSON.stringify(json));

	if (json.type == 'offer') {
		console.log('incoming event, async offer');
		if (ss == 1)
			offer2(json.sdp);
		ss = 0;
	}

	else if (json.type == 'answer') {
		console.log('async answer');
		answerdude(json.sdp);
	}

	else if (json.candidate) {
		console.log('async cand');
		
		easycandidate.senderEasyrtcid = myrtcid;
		easycandidate.targetEasyrtcid = remotertcid; // offerjson.targetEasyrtcid;	why fix this now? gdh
		easycandidate.msgData.candidate = json.candidate;

		easycandidate.msgData.id = json.sdpMid;
		easycandidate.msgData.label = json.sdpMLineIndex;

		socket.emit('easyrtcCmd', easycandidate, candyCallback);
	}
	else if (json.sdp) {
		console.log('but Im I getting stp now?????????????????????');
	}
	else {
		console.log('what comes next?');
		console.log('|' + JSON.stringify(json) + '|');
	}		
}

var authorizationCallback = function (json) { 

	document.getElementById('divLog').innerHTML += 'authorizationCallback()<br>';
 	console.log('gotauth: ' + JSON.stringify(json)); 
	myrtcid = json.msgData.easyrtcid;
	icy = json.msgData.iceConfig.iceServers;
	var rtcids = Object.keys(json.msgData.roomData.default.clientList);

	var n;
	var htm = '';
	for(var i = 0; i < rtcids.length; i++) {
		n = rtcids[i];
		if (n !== myrtcid)
			htm += '<button onclick="offer(\'' + n + '\')">' + n + '</button>';
	}
	if (htm !== '')
		document.getElementById('divPeers').innerHTML = '222 ' + htm;
	else
		document.getElementById('divPeers').innerHTML += 'nobody else available';

	console.log('ice: ' + JSON.stringify(json.msgData.iceConfig.iceServers));
	activex.pushToNative('seticeservers', JSON.stringify(json.msgData.iceConfig.iceServers));
	//activex.pushToNative('seticeservers', json.msgData.iceConfig.iceServers);
};

//	if (rtcids != null && rtcids.length > 0) {
//		var client = json.msgData.roomData.default.clientList[rtcids[0]].easyrtcid;
//		console.log('other rtcid: |' + client + '|');
//		offerjson.targetEasyrtcid = client; 
//		socket.emit('easyrtcCmd', offerjson, o fferCallback);
//	}

function callmyotherhangup() {
	hangupjson.targetEasyrtcid = remotertcid;
	socket.emit('easyrtcCmd', hangupjson, function() { });
}

function offer2(sdp) {
//	document.getElementById('divLog').i nnerHTML += 'offer(' + rtcid + ')<br>';
	offerjson.msgData.sdp = sdp;
	offerjson.targetEasyrtcid = remotertcid;
console.log('offer2(), send offer to ' + remotertcid);
console.log(sdp);
	socket.emit('easyrtcCmd', offerjson, offerCallback);
}

function answerdude(sdp) {
//	document.getElementById('divLog').i nnerHTML += 'offer(' + rtcid + ')<br>';
	answerjson.msgData.sdp = sdp;
	answerjson.targetEasyrtcid = remotertcid;
 	console.log('hey, lets answer: ' + JSON.stringify(answerjson));
	socket.emit('easyrtcCmd', answerjson, offerCallback);
}

function offer(rtcid) {
	remotertcid = rtcid;
	offerjson.targetEasyrtcid = rtcid; 

console.log();
console.log('offer(): ask activex to make an sdp');
	activex.pushToNative('makeoffer', '');

	// document.getElementById('divLog').i nnerHTML += 'offer(' + rtcid + ')<br>';
//	offerjson.targetEasyrtcid = rtcid; 
//	socket.emit('easyrtcCmd', offerjson, offerCallback);
}

// just an ack, answer will come later
var offerCallback = function (json) { 
	document.getElementById('divLog').innerHTML += 'offerCallback()<br>';
// 	console.log('offer callback:' + JSON.stringify(json));
};

var candyCallback = function (json) { 
// 	console.log('candidate callback:' + JSON.stringify(json));
};


// see http://dev.w3.org/2011/webrtc/editor/webrtc.html#rtcpeerconnection-interface
function RTCPeerConnection(ice_servers, options) {
	document.getElementById('divLog').innerHTML += 'RTCPeerConnection()<br>';
		// console.log('new pc');
		this.ice_servers = ice_servers;
		this.onaddstream = function () { };
		this.addStream = function () { };
		this.createOffer = function () { };
		this.createAnswer = function () { };
		this.setLocalDescription = function () { };
		this.setRemoteDescription = function () { };

		authjson.msgData.applicationName = appname;

// just wait  ******		socket.emit('easyrtcAuth', authjson, authorizationCallback);

//		socket.emit('easyrtcAuth', JSON.parse(authjson), authorizationCallback);
}

window.onbeforeunload = function (event) { 
	socket.disconnect();
};

/*
function processJsonEvent(json)
{
	// console.log('--->');
	console.log('processJsonEvent():');
	console.log(json);
	console.log('<---');
}
*/

function loaded() {
	document.getElementById('divLog').innerHTML += 'loaded()<br>';
// 	console.log('loaded');
}

function gotAnswer(json) {
	document.getElementById('divLog').innerHTML += 'gotAnswer()<br>';

	if (json.msgData.sdp) {
		document.getElementById('divSDP').innerHTML += '111 ' + json.msgData.sdp;
		sdp = json.msgData.sdp;
		//console.log('____ answer sdp _______________________________________');
		//console.log(sdp);
		//console.log('_______________________________________________________');
		var sdp = json.msgData.sdp;
	//	sdp = sdp.r eplace('0.0.0.0', '172.16.40.108');
	//	sdp = sdp.r eplace('0.0.0.0', '172.16.40.108');
		//console.log('push answer: ' + JSON.stringify(json));
		activex.pushToNative('handleanswer', sdp);
	}
}

function gotOffer(json) {
	document.getElementById('divLog').innerHTML += 'gotOffer()<br>';

	remotertcid = json.senderEasyrtcid;
	console.log();
	console.log('incoming offer from ' + remotertcid);

	var sdp = JSON.stringify(json.msgData.sdp);
	var actualnewlines = String.fromCharCode(13) + String.fromCharCode(10);
	sdp = sdp.replace(/\\r\\n/g, actualnewlines);
	sdp = sdp.replace(/\"/g, '');
	json.msgData.sdp = sdp;	

//var fucker = JSON.stringify(json.msgData).replace(/\\r\\n/g, actualnewlines);
//activex.pushToNative('handleoffer', fucker); // JSON.stringify(json.msgData));

activex.pushToNative('handleoffer', JSON.stringify(json.msgData));

}

function gotCandidate(json) {

//	document.getElementById('divLog').i nnerHTML += 'gotCandidate(' + json.msgData.candidate + ')<br>';
	console.log('candidate from easy: ' + JSON.stringify(json));

	goocandidate.candidate = json.msgData.candidate;
	goocandidate.sdpMid = json.msgData.id;
	goocandidate.sdpMLineIndex = json.msgData.label;

console.log('send to google c++: ' + JSON.stringify(goocandidate));
	activex.pushToNative('handlecandidate', JSON.stringify(goocandidate));
}

/*
function callActiveX() {
	document.getElementById('divLog').i nnerHTML += 'callActiveX()<br>';
	console.log('hello activex');
	if (activex != null)
		activex.SetRemoteSDP(sdp);
	else
		console.log('no activex ' + sdp);
}
*/

//var ackcallback = function (x) { 
//	console.log('ack callback called |' + JSON.stringify(x) + '|'); 
//};
//


socket.on('easyrtcCmd', function (json) {
// 	console.log('incoming from easyrtc server: ', JSON.stringify(json));
	if (json.msgData)
		if (json.msgData.type)							// don't need, could just check for sdp...
			switch(json.msgData.type) {
			case 'answer': 
				gotAnswer(json);
				break;
			case 'candidate': 
				gotCandidate(json); // .msgData.candidate);
				break;
			case 'roomData': 
				alert('unexpected roomdata: ' + json.msgData.roomData);
// 				console.log('roomdata: ' + json.msgData.roomData);
				break;
			case 'offer': 
				gotOffer(json);
				break;
			default:
				alert('unexpected message from easyrtc server ' + JSON.stringify(json));
// 				console.log('huh? ' + JSON.stringify(json));
				break;
			}

});
