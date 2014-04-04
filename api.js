(function (window, document) {

	window.callback_registry = {};

	/*utils*/
	function assert(condition, failureMessage) { if (!condition) { throw new Error("Assert failed: " + message); } }
	
	/*
	*	addEventHandler(id, functionName, handler)
	*	Use eval for the power of good. (Bind event handler to native C++ event)
	*/
	function addEventHandler(id, handler) {
		try {
			
			// Eval bind double-colon function to call handler
			window.callback_registry[id+"::EventToBrowser"] = handler;

			var evalString = 
				"function " + id + "::EventToBrowser (event) { "+
					" window.callback_registry['" + id + "::EventToBrowser'](event); "+
				"}";

			with(window){
				window.eval(evalString);
			}

		} catch (e) {
			console.log(e);
			throw new Error("Could not add custom event handler to "+ id);
		}
	}

	/* 
	* ctor - expecting an object tag in the markup
	*/
	function RTCPlugin(activexElement) {
		this.element = activexElement;
		addEventHandler(activexElement.id, this.nativeEventHandler);
		this.remoteId = null;
	}

	RTCPlugin.prototype.onicecandidate = function (){ console.log("onicecandidate"); };

	/*
	* RTCPlugin interface implementation
	*/
	RTCPlugin.prototype.createOffer = function (remoteId, success) {
		this.remoteId = remoteId;
		this.element.pushToNative('makeoffer', '');
		this.onCreateOffer = success;
	};

	RTCPlugin.prototype.handleOffer = function (offer, success) {
		this.element.pushToNative('handleoffer', JSON.stringify(offer));
		this.onHandleOffer = success;
	};

	RTCPlugin.prototype.handleAnswer = function (answer, success) {
		this.element.pushToNative('handleanswer', JSON.stringify(answer));
	};

	RTCPlugin.prototype.handleCandidate = function (candidate) {
		this.element.pushToNative("handlecandidate", JSON.stringify(candidate));
	};

	RTCPlugin.prototype.close = function () {
		this.element.pushToNative("hangup", '');
	};

	RTCPlugin.prototype.debugNative = function () {
		this.element.pushToNative("debug", '');
	};

	RTCPlugin.prototype.run = function () {
		this.element.run();
	};

	RTCPlugin.prototype.nativeEventHandler = function (json) {

		if (json.type === 'offer' && this.onCreateOffer) {
			this.onCreateOffer(json.sdp);
		
		} else if (json.type === 'answer' && this.onHandleOffer) { //back from C++
			this.onHandleOffer(json.sdp);
		
		} else if(json.candidate && this.onicecandidate) {
			this.onicecandidate(json);
		}
	}

	window.RTCPlugin = RTCPlugin;

})(window, document);

