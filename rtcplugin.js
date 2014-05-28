(function (window) {

    /*
	* Private components
	*/

    window.dlog = window.dlog || function () { };

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

            if (json.pluginMessage.message == 'frame') {
                this.inUseRenderSurfaces[json.pluginMessage.easyrtcid].src = json.pluginMessage.data;
                return;
            }

            /* internal plugin messages */
            if (json.pluginMessage.message == "gotDeviceAttributes") {
                this.gotDeviceAttributes(json.pluginMessage.data);
            } else if (json.pluginMessage.message == "failedToGetDeviceAttributes") {
                this.failedToGetDeviceAttributes(json.pluginMessage.data);
            } else if (json.pluginMessage.message == "gotSelfie") {
                this.gotSelfie(json.pluginMessage.data);
            } else if (json.pluginMessage.message == "failedToGetSelfie") {
                this.failedToGetSelfie(json.pluginMessage.data);
            } else if (json.pluginMessage.message == "gotWindowHandle" && this.gotWindowHandle) {
                this.gotWindowHandle(json.pluginMessage.data);
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
    function RTCPlugin(activexElement, canvii, win, fail) {
        this.element = activexElement;
        setEventHandler(this, activexElement.id, nativeEventHandler);
        this.isRunning = false;
        this.readyState = 0;

        this.externalRenderSurfaces = canvii;
        this.inUseRenderSurfaces = {};

        var self = this;

        runPlugin(this, function () {
            self.setAudioVideoDevices(null, null, win, fail); // select default devices
        });
    }

    /*
    * push a new renderer to the native queue
    */
    RTCPlugin.prototype.setupRenderSurface = function (easyRtcId) {
        dlog("Adding render surface for " + easyRtcId);
        var canvas = Array.prototype.shift.call(this.externalRenderSurfaces);
        var img = new Image();
        img.onload = function () {
            canvas.getContext("2d").drawImage(img, 0, 0, canvas.width, canvas.height);
        };
        this.inUseRenderSurfaces[easyRtcId] = img;
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
        nativeCall(this, 'makeoffer', { remoteId: remoteId });
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
            offer.remoteId = remoteId;
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
        var answer = { remoteId: remoteId, sdp: sdp };
        nativeCall(this, 'handleanswer', answer);
    };

    /*
	* handleCandidate(sdp)
	* - asks native code to handle an incoming candidate 
	*/
    RTCPlugin.prototype.handleCandidate = function (remoteId, candidate) {
        //TODO: fix for remoteId
        candidate.remoteId = remoteId;
        nativeCall(this, "handlecandidate", candidate);
    };

    /*
	* hangUp() 
	* - closes the current call
	*/
    RTCPlugin.prototype.hangUp = function (remoteId) {
        nativeCall(this, "hangup", { remoteId: remoteId });
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





    // TODO: implement these !
    /*
    * take a selfie. Cheese!
    */
    RTCPlugin.prototype.getSelfie = function (win, fail, optionalResolution) {
        this.gotSelfie = win;
        this.failedToGetSelfie = fail;
        nativeCall(this, "getSelfie", optionalResolution || "");
    };

    /*
    * returns a list of video input device metadata objects
    * names are used as keys for 
    */
    RTCPlugin.prototype.getDeviceAttributes = function (win, fail) {
        this.gotDeviceAttributes = win;
        this.failedToGetDeviceAttributes = fail;
        nativeCall(this, "getDeviceAttributes");
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

        //HACK FOR NOW - make a callback through the message loop
        this.onSetDevicesWin();
    };

    /*
    * Add a plugin as a renderable surface
    */
    RTCPlugin.prototype.addRenderTarget = function (plugin, win) {
        var self = this;
        plugin.getWindowHandle(function (handle) {
            self.addRenderHandle(handle);
            if (win) win();
        });
    };


    /*
	* Explicitly export only RTCPlugin
	*/
    window.RTCPlugin = RTCPlugin;

})(window);


