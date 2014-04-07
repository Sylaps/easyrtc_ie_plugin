(function (window, document) {

	window.callback_registry = {};

	/*utils*/
	function assert(condition, failureMessage) { if (!condition) { throw new Error("Assert failed: " + message); } }

	/*
	*	addEventHandler(id, functionName, handler)
	*	Use eval for the power of good. (Bind event handler to native C++ event)
	*/
	function addEventHandler(context, id, handler) {
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
	* ctor - expecting an object tag in the markup
	*/
	function RTCPlugin(activexElement) {
		this.element = activexElement;
		addEventHandler(this, activexElement.id, this.nativeEventHandler);
		this.remoteId = null;
		this.isRunning = false;
	}

	RTCPlugin.prototype.onicecandidate = function (json){ console.log("onicecandidate"); };

	/*
	* RTCPlugin interface implementation
	*/
	RTCPlugin.prototype.createOffer = function (remoteId, success) {
		this.remoteId = remoteId;
		this.onCreateOffer = success;
		nativeCall(this, 'makeoffer', '');

	};

	RTCPlugin.prototype.handleOffer = function (remoteId, offer, success) {
		if (!this.handlingOffer) {
			this.handlingOffer = true;
			this.remoteId = remoteId;
			this.onHandleOffer = success;
			nativeCall(this, 'handleoffer', offer);
		} else {
			throw new Error ("Already handling an offer!");
		}

	};

	RTCPlugin.prototype.handleAnswer = function (answer) {
		nativeCall(this, 'handleanswer', JSON.stringify(answer));
	};

	RTCPlugin.prototype.handleCandidate = function (candidate) {
		nativeCall(this, "handlecandidate", JSON.stringify(candidate));
	};

	RTCPlugin.prototype.close = function () {
		nativeCall(this, "hangup", '');
	};

	RTCPlugin.prototype.nativeDebug = function () {
		nativeCall(this, "debug", '');
	};

	RTCPlugin.prototype.run = function () {
		if (!this.isRunning) {
			this.isRunning = true;
			this.element.run();
		} else {
			alert("RTCPlugin.run() called twice!");
		}
	};

	RTCPlugin.prototype.nativeEventHandler = function (data) {

		var json;
		var obj;
		if (typeof data == 'string'){
			dlog("(.)(.) == dont treat me like an object!");
			json = json[0];
			json = JSON.parse(data);
			dlog(json);
		} else {
			dlog("(.)(.)object");
			json = data;
			for (var i in json){
				dlog(i + " : " + json[i]);
			}
			obj = json[0];
			dlog(">>>> obj :" + obj )
			json = JSON.parse(obj);
		}

	
		dlog(this.onCreateOffer !== undefined);
		dlog(this.onHandleOffer !== undefined);
		dlog(this.onicecandidate !== undefined);
		dlog("json.type:"+json.type);
		dlog("json.candidate:"+json.candidate);


		if (json.type == 'offer' && this.onCreateOffer) {
			dlog("offer from native");
			this.onCreateOffer(json.sdp);
		
		} else if (json.type == 'answer' && this.onHandleOffer) { //back from C++

			this.onHandleOffer(json.sdp);
			this.handlingOffer = false;
		
		} else if(json.candidate && this.onicecandidate) {
			dlog('candidate from native');
			this.onicecandidate(json);
		} else {
			dlog("default case (called from native, but we dont know why)");
			dlog(json);
		}
	}

	function nativeCall(plugin, msg, data) {
		dlog("native call: " + msg + " data:" + (typeof data === 'object' ? JSON.stringify(data) : data));
		if (plugin.isRunning){
			plugin.element.pushToNative(msg, typeof data === 'object' ? JSON.stringify(data) : data);
		}
	}


	window.RTCPlugin = RTCPlugin;

})(window, document);


