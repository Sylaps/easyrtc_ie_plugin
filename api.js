(function (window, navigator) {
	"use strict";

	/*utils*/
	/* not-implemented stub */
	function notImplemented() { throw new Error("Not implemented"); }; 

	function assert(condition, failureMessage) { if (!condition) { throw new Error("Assert failed: " + message); } }

	var idCounter = 0;

	/*
	*	addEventHandler(id, functionName, handler)
	*	Use eval for the power of good. (Bind event handler to native C++ event)
	*/
	function addEventHandler(id, handler) {
		try {
			var evalString = "function "+id+"::EventToBrowser (event) { handler(event); }"; // Eval bind double-colon function to call handler
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

	/*
	*   Explicit declaration of RTCSignalingState
	*/
	var RTCSignalingState = {
		"stable"             : "stable",
		"have-local-offer"   : "have-local-offer",
		"have-remote-offer"  : "have-remote-offer",
		"have-local-proffer" : "have-local-proffer",
		"have-remote-proffer": "have-remote-proffer",
		"closed"             : "closed"
	};

	var RTCIceGatheringState = {
		"new" : "new",
		"gathering" : "gathering",
		"complete" : "complete"
	};
	
	// { type:(offer, answer, pranswer), sdp: sdpstring } 
	function RTCSessionDescription(options) {
		this.type = options.type;
		this.sdp = options.sdp;
	}

	/* 
	* ctor - expecting an object tag in the markup
	*/
	function IEPlugin(configuration, constraints, activexElement) {

		function NativeBridge (activexElement) {
			this.element = activexElement;
			addEventHandler(activexElement.id, this.nativeEventHandler);

			this.onMakeOfferSuccess = 
			this.onMakeOfferFailure = 
			this.onMakeAnswerSuccess = 
			this.onMakeAnswerFailure = emptyEvent;
		}

		NativeBridge.prototype.makeOffer = function (success, failure) {
			this.element.pushToNative('makeoffer', '');
			this.onMakeOfferSuccess = success;
			this.onMakeOfferFailure = failure;
		};

		NativeBridge.prototype.handleAnswer = function (success, failure) {
			this.element.pushToNative('handleanswer', '');
			this.onMakeAnswerSuccess = success;
			this.onMakeAnswerFailure = failure;
		};

		NativeBridge.prototype.nativeEventHandler = function (json) {

			if (json.type === 'offer') {

				if (this.onMakeOfferSuccess) {
					this.onMakeOfferSuccess(json.sdp);
				}

			} else if (json.type === 'answer') {

				if (this.onMakeAnswerSuccess) {
					this.onMakeAnswerSuccess(json.sdp);
				}

			} else if(json.candidate){
				if (this.onicecandidate) {
					this.onicecandidate(json);
				}

			}
		}

		this.nativeBridge = new NativeBridge(activexElement);

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
	* IEPlugin interface implementation
	*/
	IEPlugin.prototype.createOffer = function (success, failure, constraints) {
		this.nativeBridge.makeOffer(success, failure, constraints);
	};

	IEPlugin.prototype.createAnswer = function (success, failure, constraints) {
		this.nativeBridge.makeAnswer(success, failure, constraints);
	};

	IEPlugin.prototype.close = function () {
		this.nativeBridge.pushToNative("hangup", '');
	};

	/*
	* Unimplemented parts of the IEPlugin interface
	*/
	IEPlugin.prototype.setLocalDescription = notImplemented;
	IEPlugin.prototype.setRemoteDescription = notImplemented;
	IEPlugin.prototype.updateIce = notImplemented;
	
	IEPlugin.prototype.addIceCandidate = notImplemented;

	IEPlugin.prototype.getRemoteStreams = notImplemented;
	IEPlugin.prototype.getStreamById =    notImplemented;
	IEPlugin.prototype.addStream = 	   notImplemented;
	IEPlugin.prototype.removeStream = 	   notImplemented;
	IEPlugin.prototype.createDataChannel = notImplemented;

	window.IEPlugin = IEPlugin;

})(window, navigator);