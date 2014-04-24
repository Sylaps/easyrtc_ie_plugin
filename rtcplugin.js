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
       * - starts the activex plugin event loop
       * - blocks and must be run from setTimeout (bug in our message loop)
       */
    /* private */ function runPlugin(plugin, win) {

        if (!plugin.isRunning) {

            plugin.isRunning = true;
            setTimeout(function () {
                plugin.element.run();
                if (win) {
                    win();
                }
            }, 0);

        }

    };

    /*
	* nativeEventHandler (private)
	* - handle events coming from ActiveXControl
	* - must be bound to RTCPlugin instance through closure over Function.prototype.call
	* (see setEventHandler)
	*/
    /* private */ function nativeEventHandler(data) {
        var json;
        var obj;

        if (typeof data == 'string') {
            json = JSON.parse(data);
        } else {
            json = JSON.parse(data[0]);
        }

        if (json.type == 'offer' && this.onCreateOffer) {
            this.onCreateOffer(json.sdp);
        } else if (json.type == 'answer' && this.onHandleOffer) { //back from C++
            this.onHandleOffer(json.sdp);
            this.handlingOffer = false;
        } else if (json.candidate && this.onicecandidate) {
            this.onicecandidate(json);
        } else if (json.pluginMessage) {

            /* internal plugin messages */
            if (json.pluginMessage.message == "gotDeviceAttributes") {
                this.gotDeviceAttributes(json.pluginMessage.data);
            } else if (json.pluginMessage.message == "failedToGetDeviceAttributes") {
                this.failedToGetDeviceAttributes(json.pluginMessage.data);
            } else if (json.pluginMessage.message == "gotSelfie") {
                this.gotSelfie(json.pluginMessage.data);
            } else if (json.pluginMessage.message == "failedToGetSelfie") {
                this.failedToGetSelfie(json.pluginMessage.data);
            }

        }
        else {
            throw new Error("Exceptional state: unknown message: " + JSON.stringify(json));
        }
    }

    /*
	* Push a message down to native code
	*/
    /* private */ function nativeCall(plugin, msg, data) {
        dlog("nativeCall:" + msg);
        dlog("nativeCall (data):" + data);
        if (plugin.isRunning) {
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
            window.callback_registry[id + "::EventToBrowser"] = function (event) {
                handler.call(context, [event]);
            };

            var evalString =
				"function " + id + "::EventToBrowser (event) { " +
					" window.callback_registry['" + id + "::EventToBrowser'](event); " +
				"}";

            with (window) {
                window.eval(evalString);
            }

        } catch (e) {
            console.log(e);
            console.log("Could not add custom event handler to " + id);
        }
    }

    /* 
	* ctor function - expects an object tag in the markup (activexElement)
	*/
    function RTCPlugin(activexElement, win, fail) {
        this.element = activexElement;
        setEventHandler(this, activexElement.id, nativeEventHandler);
        this.remoteId = null;
        this.isRunning = false;
        this.readyState = 0;

        var self = this;

        runPlugin(this, function () {
            self.setAudioVideoDevices(null, null, win, fail); // select default devices
        });
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
            throw new Error("Already handling an offer!");
        }

    };

    /*
	* handleAnswer(sdp)
	* - asks native code to handle an incoming answer 
	*/
    RTCPlugin.prototype.handleAnswer = function (remoteId, sdp) {
        nativeCall(this, 'handleanswer', sdp);
    };

    /*
	* handleCandidate(sdp)
	* - asks native code to handle an incoming candidate 
	*/
    RTCPlugin.prototype.handleCandidate = function (remoteId, candidate) {
        nativeCall(this, "handlecandidate", candidate);
    };

    /*
	* hangUp() 
	* - closes the current call
	*/
    RTCPlugin.prototype.hangUp = function (remoteId) {
        nativeCall(this, "hangup", '');
    };

    /*
	* nativeDebug()
	* - for use in Debug only (with debug native activex build)
	* - triggers a ::DebugBreak in the event loop listening to javascript
	* - useful for debugging calls to native code */
    RTCPlugin.prototype.nativeDebug = function () {
        nativeCall(this, "debug", '');
    };

    // land of the non-implemented functions
    /*
    * take a selfie. Cheese!
    */
    RTCPlugin.prototype.getSelfie = function (optionalResolution, win, fail) {
        this.gotSelfie = win;
        this.failedToGetSelfie = fail;
        nativeCall("getSelfie", optionalResolution);
    };

    /*
    * returns a list of video input device metadata objects
    * names are used as keys for 
    */
    RTCPlugin.prototype.getDeviceAttributes = function (win, fail) {
        this.gotDeviceAttributes = win;
        this.failedToGetDeviceAttributes = fail;
        nativeCall("getDeviceAttributes");
    };

    /*
    * set the active audio/video devices
    */
    RTCPlugin.prototype.setAudioVideoDevices = function (camera, mic, win, fail) {
        camera = camera || {};
        camera.id = camera.id || "Default Camera";
        camera.resolution = camera.resolution || { w: 640, h: 480 };

        mic = mic || {};
        mic.id = mic.id || "Default Microphone";

        this.onSetDevicesWin = win;
        this.onSetDevicesFail = fail;

        //HACK FOR NOW
        this.onSetDevicesWin();
    };

    /*
	* Explicitly export only RTCPlugin
	*/
    window.RTCPlugin = RTCPlugin;

})(window);


