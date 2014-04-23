(function (window) {

	/*
	* Private components
	*/

	/*
	* Explicit global registry of event handlers
	* - required due to the nature of binding event handlers to ActiveX controls
	* - see setEventHandler() for more information
	*/
	window.callback_registry = {};

	/*
	* nativeEventHandler (private)
	* - handle events coming from ActiveXControl
	* - must be bound to RTCPlugin instance through closure over Function.prototype.call
	* (see setEventHandler)
	*/
	/* private */ function nativeEventHandler(data) {
		var json;
		var obj;
		if (typeof data == 'string'){
			json = JSON.parse(data);
		} else {
			json = JSON.parse(data[0]);
		}

	    dlog("!!!!!!!!!!!!!!!!!!!!!!! call from native:"+ JSON.stringify(json));

		if (json.type == 'offer' && this.onCreateOffer) {
			this.onCreateOffer(json.sdp);
		
		} else if (json.type == 'answer' && this.onHandleOffer) { //back from C++

			this.onHandleOffer(json.sdp);
			this.handlingOffer = false;
		
		} else if(json.candidate && this.onicecandidate) {
			this.onicecandidate(json);
		} 
		else
			alert('hello ' + JSON.stringify(json));
	}

	/*
	* Push a message down to native code
	*/
	/* private */ function nativeCall(plugin, msg, data) {
	    dlog("nativeCall:" + msg);
	    dlog("nativeCall (data):" + data);
		if (plugin.isRunning){
			plugin.element.pushToNative(msg, typeof data === 'object' ? JSON.stringify(data) : data);
		}
	}

	/*
	*	setEventHandler(id, functionName, handler)
	*	Use eval for the power of good. (Bind event handler to native C++ event)
	*/
	/* private */ function setEventHandler(context, id, handler) {
		try {
			
			// Eval bind double-colon function to call handler
			window.callback_registry[id+"::EventToBrowser"] = function (event) {
				handler.call(context, [event]);
			};

			var evalString = 
				"function " + id + "::EventToBrowser (event) { "+
					" window.callback_registry['" + id + "::EventToBrowser'](event); "+
				"}";

			with(window){
				window.eval(evalString);
			}

		} catch (e) {
			console.log(e);
			console.log("Could not add custom event handler to "+ id);
		}
	}

	/* 
	* ctor function - expects an object tag in the markup (activexElement)
	*/
	function RTCPlugin(activexElement) {
		this.element = activexElement;
		setEventHandler(this, activexElement.id, nativeEventHandler);
		this.remoteId = null;
		this.isRunning = false;
		this.readyState = 0;
	}

	/*
	* supportedBrowser()
	* - determine if this browser is supported (ie. IE)
	*/
	/* static */ RTCPlugin.supportedBrowser = function () {
        return window.ActiveXObject === undefined;
	};

	/*
	* Public RTCPlugin interface implementation
	* ==================================
	*/

	/*
	* handleIceServers(sdp)
	* - asks native code to note the ice servers provided by the signalling server
	*/
	RTCPlugin.prototype.handleIceServers = function (iceServers) {
		nativeCall(this, 'seticeservers', iceServers);
	};

	/*
	* createOffer(remoteId, callback)
	* - asks native code to create an offer
	* - asynchronous - value returned in arg of callback(arg)
	*/
	RTCPlugin.prototype.createOffer = function (remoteId, callback) {
		this.remoteId = remoteId;
		this.onCreateOffer = callback;
		nativeCall(this, 'makeoffer', '');
	};

	/*
	* handleOffer(remoteId, offer, callback)
	* - asks native code to handle an incoming offer
	* - asynchronous - value returned in arg of callback(arg)
	* - blocks concurrent calls; we can only handle one offer at a time
	*/
	RTCPlugin.prototype.handleOffer = function (remoteId, offer, callback) {
		if (!this.handlingOffer) {
			this.handlingOffer = true;
			this.remoteId = remoteId;
			this.onHandleOffer = callback;
			nativeCall(this, 'handleoffer', offer);
		} else {
			throw new Error ("Already handling an offer!");
		}

	};

	/*
	* handleAnswer(sdp)
	* - asks native code to handle an incoming answer (sdp only)
	*/
	RTCPlugin.prototype.handleAnswer = function (sdp) {
		nativeCall(this, 'handleanswer', sdp);
	};

	/*
	* handleCandidate(sdp)
	* - asks native code to handle an incoming candidate 
	*/
	RTCPlugin.prototype.handleCandidate = function (candidate) {
		nativeCall(this, "handlecandidate", candidate);
	};

	/*
	* close() 
	* - closes the current call
	*/
	RTCPlugin.prototype.close = function () {
		nativeCall(this, "hangup", '');
	};

	/*
	* nativeDebug()
	* - for use in Debug only (with debug native activex build)
	* - triggers a ::DebugBreak in the event loop listening to javascript
	* - useful for debugging calls to native code
	*/
	RTCPlugin.prototype.nativeDebug = function () {
		nativeCall(this, "debug", '');
	};


	/*
	* run()
	* - starts the activex plugin event loop
	* - blocks and must be run from setTimeout (bug in our message loop)
	*/
	RTCPlugin.prototype.run = function () {
        
		if (!this.isRunning) {
			this.isRunning = true;
			this.element.run();
		} else {
			alert("RTCPlugin.run() called twice!");
		}
	};


	/*
	* Explicitly export only RTCPlugin
	*/
	window.RTCPlugin = RTCPlugin;

})(window);


