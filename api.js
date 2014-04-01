(function (window) {
	"use strict";

	/*utils*/
	/* not-implemented stub */
	function notImplemented() { throw new Error("Not implemented"); }; 

	function assert(condition, failureMessage) { if (!condition) { throw new Error("Assert failed: " + message); } }

	var idCounter = 0;

	/*
	* createActiveXDomElement - create activeX dom element, and hand it back for insertion to the DOM
	*
	*/
	function createActiveXDomElement() {

		var id = "activexelement_"+ (idCounter++); // create a sequential id for activex dom elements
		,   clsid = "CLSID:0E8D29CE-D2D0-459A-8009-3B34EFBC43F0" // this will remain static
		,   domElement = document.createElement("object");
		//,   codebase = "http://easyrtc.com/activex/codebase" // will need to be customized 

		domElement.setAttribute("id", id);
		domElement.setAttribute("classid", clsid);
		domElement.setAttribute("codebase", codebase); 

		return domElement;
	}

	/*
	*	addEventHandler(id, functionName, handler)
	*	Use eval for the power of good. (Bind event handler to native C++ event)
	*/
	function addEventHandler(id, functionName, handler) {
		try {
			var evalString = "function "id+"::"+functionName+" (event) { handler(event); }"; // Eval bind double-colon function to call handler
			eval(evalString);
		} catch (e) {
			throw new Error("Could not add custom event handler to "+ id);
			console.log(e);
		}
	}

	/* 
	*	eventhandler stub 
	*/
	var emptyEvent = (function () {});

	var RTCSignalingState = {
		"stable"             : "stable",
		"have-local-offer"   : "have-local-offer",
		"have-remote-offer"  : "have-remote-offer",
		"have-local-proffer" : "have-local-proffer",
		"have-remote-proffer": "have-remote-proffer",
		"closed"             : "closed"
	};
	
	/* 
	* ctor 
	*/
	function RTCPeerConnection(configuration, constraints) {

		function NativeBridge (parent /*ID or dom element*/) {
			var activexElement;
			this.parent = (typeof parent === 'string') 
						? document.getElementById(parent) 
						: parent;

			assert(this.parent !== undefined && this.parent != null, "NativeBridge -> this.parent does not exist.");

			/* create and append activex element needed */
			this.parent.appendChild(createActiveXDomElement());
		}

		var nativeBridge = new NativeBridge();

		// Properties
		this.remoteDescription = "";
		this.signalingState = RTCSignalingState.closed;
		
		this.onnegotiationneeded = 
		this.onicecandidate = 
		this.onsignalingstatechange = 
		this.onremovestream = 
		this.onaddstream = 
		this.ondatachannel = emptyEvent;

	}

	/*
	* Unimplemented parts of the RTCPeerConnection interface
	*/
	RTCPeerConnection.prototype.getRemoteStreams = notImplemented;
	RTCPeerConnection.prototype.getStreamById =    notImplemented;
	RTCPeerConnection.prototype.addStream = 	   notImplemented;
	RTCPeerConnection.prototype.removeStream = 	   notImplemented;


	RTCPeerConnection.prototype.createOffer = function (success, failure, constraints) {

	};

	RTCPeerConnection.prototype.createAnswer = function (success, failure, constraints) {
		
	};

	RTCPeerConnection.prototype.setLocalDescription = function (description, success, failure) {

	};

	RTCPeerConnection.prototype.setRemoteDescription = function (description, success, failure) {

	};

	RTCPeerConnection.prototype.updateIce = function () {

	};

	RTCPeerConnection.prototype.addIceCandidate = function(candidate, success, failure) {
		
	};

	RTCPeerConnection.prototype.close = function () {

	};

	RTCPeerConnection.prototype.createDataChannel = notImplemented;

	window.RTCPeerConnection = RTCPeerConnection;

})(window);