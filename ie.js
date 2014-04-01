
var socket = io.connect(':8080');

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

//var fixsdp = 'c=IN IP4 172.16.40.108NEWCARa=rtcp:50592 IN IP4 172.16.40.108NEWCARa=candidate:1150697756 1 udp 2122129151 172.16.40.108 50592 typ host generation 0NEWCARa=candidate:1150697756 2 udp 2122129151 172.16.40.108 50592 typ host generation 0NEWCARa=candidate:169197036 1 tcp 1518149375 172.16.40.108 51722 typ host generation 0NEWCARa=candidate:169197036 2 tcp 1518149375 172.16.40.108 51722 typ host generation 0NEWCAR';

function asyncEvent(json) {
	// if (json.sdp) { // type = offer
	if (json.type === 'offer') {
		//var IPpos = json.sdp.indexOf('c=IN IP4 0.0.0.0');
		//if (IPpos > 0) {
			//console.log('fix sdp...');
			//json.sdp = json.sdp.replace('0.0.0.0', '172.16.40.108').replace('0.0.0.0', '172.16.40.108');
		//}
		// console.log('send easy an offer(sdp): ' + JSON.stringify(json.sdp));
		offer2(json.sdp);
	}

	if (json.type === 'answer') {
		answerdude(json.sdp);
	}

	else if (json.candidate) {
		//
		// from google:
		// {"candidate":"a=candidate:1150697756 1 udp 2122129151 172.16.40.108 61853 typ host generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}
		//
		// from easyrtc:
		// incoming easyrtcCmd:  {"senderEasyrtcid":"qUrto0iznJiABk0LIqaa","msgData":{"type":"candidate","label":0,"id":"audio","candidate":"a=candidate:1150697756 1 udp 2113937151 172.16.40.108 51104 typ host generation 0\r\n"},"easyrtcid":"NLQoUZNI8iL-fVMoIqab","msgType":"candidate","serverTime":1395698888439}
		//
		// console.log('got candidate from google' +  JSON.stringify(json));
		easycandidate.senderEasyrtcid = myrtcid;
		easycandidate.targetEasyrtcid = remotertcid; // offerjson.targetEasyrtcid;	why fix this now? gdh

// which?
		//easycandidate.msgData.candidate = json.candidate.replace('\r\n','');
		easycandidate.msgData.candidate = json.candidate;

		// id or label?
		easycandidate.msgData.id = json.sdpMid;
		easycandidate.msgData.label = json.sdpMLineIndex;

//		console.log('**************************************************************************************************');
//		console.log(JSON.stringify(easycandidate));
//		console.log('**************************************************************************************************');
		socket.emit('easyrtcCmd', easycandidate, candyCallback);
	}
// 	else
// 		console.log('unexpected from native: ' + JSON.stringify(json));
}

var authorizationCallback = function (json) { 

	document.getElementById('divLog').innerHTML += 'authorizationCallback()<br>';
// 	console.log('gotauth: ' + JSON.stringify(json)); 
	myrtcid = json.msgData.easyrtcid;
	icy = json.msgData.iceConfig.iceServers;
	var rtcids = Object.keys(json.msgData.roomData.default.clientList);
// 	console.log('rtcids |' + rtcids + '|');

	var htm = '';
	for(var i = 0; i < rtcids.length; i++) {
		var n = rtcids[i];
		if (n !== myrtcid)
			htm += '<button onclick="offer(\'' + n + '\')">' + n + '</button>';
	}
	if (htm !== '')
		document.getElementById('divPeers').innerHTML = htm;
	else
		document.getElementById('divPeers').innerHTML += 'nobody else available'; //  at http://localhost:8080/demos/demo_audio_video_simple.html';

//	if (rtcids != null && rtcids.length > 0) {
//		var client = json.msgData.roomData.default.clientList[rtcids[0]].easyrtcid;
//		console.log('other rtcid: |' + client + '|');
//		offerjson.targetEasyrtcid = client; 
//		socket.emit('easyrtcCmd', offerjson, offerCallback);
//	}

};

function hangup() {
	hangupjson.targetEasyrtcid = remotertcid;
	socket.emit('easyrtcCmd', hangupjson, function() { });
}

function offer2(sdp) {
//	document.getElementById('divLog').innerHTML += 'offer(' + rtcid + ')<br>';
	offerjson.msgData.sdp = sdp;
	offerjson.targetEasyrtcid = remotertcid;
console.log('send offer to ' + remotertcid);
	socket.emit('easyrtcCmd', offerjson, offerCallback);
}

function answerdude(sdp) {
//	document.getElementById('divLog').innerHTML += 'offer(' + rtcid + ')<br>';
	answerjson.msgData.sdp = sdp;
	answerjson.targetEasyrtcid = remotertcid;
 	console.log('hey, lets answer: ' + JSON.stringify(answerjson));
	socket.emit('easyrtcCmd', answerjson, offerCallback);
}

function offer(rtcid) {
	remotertcid = rtcid;
	offerjson.targetEasyrtcid = rtcid; 
// 	console.log('ask activex to make an sdp');
	activex.pushToNative('makeoffer', '');


	// document.getElementById('divLog').innerHTML += 'offer(' + rtcid + ')<br>';
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
		document.getElementById('divSDP').innerHTML = json.msgData.sdp;
		sdp = json.msgData.sdp;
		//console.log('____ answer sdp _______________________________________');
		//console.log(sdp);
		//console.log('_______________________________________________________');
		var sdp = json.msgData.sdp;
	//	sdp = sdp.replace('0.0.0.0', '172.16.40.108');
	//	sdp = sdp.replace('0.0.0.0', '172.16.40.108');
		//console.log('push answer: ' + JSON.stringify(json));
		activex.pushToNative('gotanswer', sdp);
	}
}

function gotOffer(json) {
	document.getElementById('divLog').innerHTML += 'gotOffer()<br>';

	remotertcid = json.senderEasyrtcid;
	console.log();
	console.log();
	console.log('incoming offer from ' + remotertcid);
	console.log();

	var sdp = JSON.stringify(json.msgData.sdp);
	if (sdp.indexOf('"') == 0) {
		sdp = sdp.replace('"', '').replace('"', '');		// why are there double quotes?
	}

	var actualnewlines = String.fromCharCode(13) + String.fromCharCode(10);
	sdp = sdp.replace(/\\r\\n/g, actualnewlines);

	// test - send the whole thing
json.msgData.sdp = sdp;	

activex.pushToNative('gotoffer', JSON.stringify(json.msgData));
// activex.pushToNative('gotoffer', sdp);
}

function gotCandidate(json) {

//	document.getElementById('divLog').innerHTML += 'gotCandidate(' + json.msgData.candidate + ')<br>';
	console.log('candidate from easy: ' + JSON.stringify(json));

	goocandidate.candidate = json.msgData.candidate;
	goocandidate.sdpMid = json.msgData.id;
	goocandidate.sdpMLineIndex = json.msgData.label;

//	console.log('send to google c++: ' + JSON.stringify(goocandidate));
	activex.pushToNative('gotcandidate', JSON.stringify(goocandidate));
}

/*
function callActiveX() {
	document.getElementById('divLog').innerHTML += 'callActiveX()<br>';
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
