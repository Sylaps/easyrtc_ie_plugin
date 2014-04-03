(function (window, navigator) {
	"use strict";

	/*utils*/
	function assert(condition, failureMessage) { if (!condition) { throw new Error("Assert failed: " + message); } }

	/*
	*	addEventHandler(id, functionName, handler)
	*	Use eval for the power of good. (Bind event handler to native C++ event)
	*/
	function addEventHandler(id, handler) {
		try {
			// Eval bind double-colon function to call handler
			var evalString = "function " + id + "::EventToBrowser (event) { handler(event); }"; 
			eval(evalString);
		} catch (e) {
			console.log(e);
			throw new Error("Could not add custom event handler to "+ id);
		}
	}

	/* 
	* ctor - expecting an object tag in the markup
	*/
	function RTCPlugin(activexElement) {

		var self = this;

		function NativeBridge (activexElement) {
			this.element = activexElement;
			addEventHandler(activexElement.id, this.nativeEventHandler);
			this.element.run();
		}

		NativeBridge.prototype.createOffer = function (success) {
			this.element.pushToNative('makeoffer', '');
			this.onCreateOffer = success;
		};

		NativeBridge.prototype.handleOffer = function (success, offer) {
			this.element.pushToNative('handleoffer', offer);
			this.onHandleOffer = success;
		};

		NativeBridge.prototype.handleAnswer = function (success, answer) {
			this.element.pushToNative('handleanswer', answer);
			this.onHandleAnswer = success;
		};

		NativeBridge.prototype.nativeEventHandler = function (json) {

			if (json.type === 'offer' && this.onCreateOffer) {
				this.onCreateOffer(json.sdp);
			
			} else if (json.type === 'answer' && this.onCreateAnswer) {
				this.onCreateAnswer(json.sdp);
			
			} else if(json.candidate && this.onicecandidate) {
				self.onicecandidate(json);
			}
		}
		this.nativeBridge = new NativeBridge(activexElement);

	}

	RTCPlugin.prototype.onicecandidate = function (){ console.log("onicecandidate"); };

	/*
	* RTCPlugin interface implementation
	*/
	RTCPlugin.prototype.createOffer = function (success) {
		this.nativeBridge.createOffer(success);
	};

	RTCPlugin.prototype.handleOffer = function (success, offer) {
		this.nativeBridge.handleOffer(success, JSON.stringify(offer));
	};

	RTCPlugin.prototype.handleAnswer = function (success, answer) {
		this.nativeBridge.handleAnswer(success, JSON.stringify(answer));
	};

	RTCPlugin.prototype.handleCandidate = function (candidate) {
		this.nativeBridge.pushToNative("handlecandidate", JSON.stringify(candidate));
	};

	RTCPlugin.prototype.close = function () {
		this.nativeBridge.pushToNative("hangup", '');
	};

	RTCPlugin.prototype.debugNative = function () {
		this.nativeBridge.pushToNative("debug", '');
	};

	window.RTCPlugin = RTCPlugin;

})(window, navigator);

