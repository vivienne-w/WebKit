function runTest(config,qualifier) {

    // config.initData contains a list of keys. We expect those to be needed in order and get
    // one waitingforkey event for each one.

    var testname = testnamePrefix(qualifier, config.keysystem)
                                    + ', successful playback, temporary, '
                                    + /video\/([^;]*)/.exec(config.videoType)[1]
                                    + ', waitingforkey event, '
                                    + config.initData.length + ' key' + (config.initData.length > 1 ? 's' : '');

    var configuration = {   initDataTypes: [ config.initDataType ],
                            audioCapabilities: [ { contentType: config.audioType } ],
                            videoCapabilities: [ { contentType: config.videoType } ],
                            sessionTypes: [ 'temporary' ] };

    async_test(function(test) {
        // console.log("started test");
        var _video = config.video,
            _mediaKeys,
            _mediaKeySessions = [],
            _mediaSource;

        function onFailure(error) {
            // console.log("func onFailure(error)");
            // console.log(error);
            // console.log("");
            forceTestFailureFromPromise(test, error);
        }

        function onMessage(event) {
            // console.log("func onMessage(event)");
            // console.log(event);
            // console.log("");
            config.messagehandler( event.messageType, event.message ).then( function( response ) {
                return event.target.update( response );
            }).catch(onFailure);
        }

        function onWaitingForKey(event) {
            // console.log("func onWaitingForKey(event)");
            // console.log(event);
            // console.log("");
            // Expect one waitingforkey event for each initData we were given
            assert_less_than(_mediaKeySessions.length, config.initData.length);
            var mediaKeySession = _mediaKeys.createSession( 'temporary' );
            waitForEventAndRunStep('message', mediaKeySession, onMessage, test);
            _mediaKeySessions.push(mediaKeySession);
            mediaKeySession.generateRequest(config.initDataType, config.initData[_mediaKeySessions.length - 1]).catch(onFailure);
        }

        function onTimeupdate(event) {
            // console.log("func onTimeupdate(event), _video.currentTime, config.duration");
            // console.log(event);
            // console.log(_video.currentTime);
            // console.log(config.duration);
            // console.log("");
            if (_video.currentTime > (config.duration || 1)) {
                // console.log("assert_equals: _mediaKeySessions.length, config.initData.length");
                // console.log(_mediaKeySessions.length);
                // console.log(config.initData.length);
                // console.log("");
                assert_equals(_mediaKeySessions.length, config.initData.length);
                _video.removeEventListener('timeupdate', onTimeupdate);
                _video.pause();
                test.done();
            }
        }

        navigator.requestMediaKeySystemAccess(config.keysystem, [configuration]).then(function(access) {
            // console.log("closure createMediaKeys");
            return access.createMediaKeys();
        }).then(function(mediaKeys) {
            // console.log("closure _video.setMediaKeys");
            _mediaKeys = mediaKeys;
            return _video.setMediaKeys(_mediaKeys);
        }).then(function(){
            // console.log("closure waitingforkey");
            waitForEventAndRunStep('waitingforkey', _video, onWaitingForKey, test);

            // Not using waitForEventAndRunStep() to avoid too many
            // EVENT(onTimeUpdate) logs.
            // console.log("closure waitingforkey timeupdate");
            _video.addEventListener('timeupdate', onTimeupdate, true);
            // console.log("closure waitingforkey return");
            return testmediasource(config);
        }).then(function(source) {
            _mediaSource = source;
            // console.log("closure createObjectURL");
            _video.src = URL.createObjectURL(_mediaSource);
            return source.done;
        }).then(function(){
            // console.log("closure _video.play");
            _video.play();
        }).catch(onFailure);
    }, testname);
}
