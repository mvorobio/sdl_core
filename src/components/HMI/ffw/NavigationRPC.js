/*
 * Copyright (c) 2013, Ford Motor Company All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: ·
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. · Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided
 * with the distribution. · Neither the name of the Ford Motor Company nor the
 * names of its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Reference implementation of Navigation component.
 * 
 * Interface to get or set some essential information sent from SDLCore.
 * Navigation is responsible for the navigation functionality provided by the
 * application: display graphics and multimedia components, is responsible for
 * the transfer of managed manipulations, generated by the user to the server.
 * 
 */

FFW.Navigation = FFW.RPCObserver.create( {

    /**
     * If true then Navigation is present and ready to communicate with SDL.
     * 
     * @type {Boolean}
     */
    isReady: false,

    /**
     * access to basic RPC functionality
     */
    client: FFW.RPCClient.create( {
        componentName: "Navigation"
    }),

    // temp var for debug
    appID: 1,

    /**
     * connect to RPC bus
     */
    connect: function() {

        this.client.connect(this, 800); // Magic number is unique identifier for
        // component
    },

    /**
     * disconnect from RPC bus
     */
    disconnect: function() {

        this.client.disconnect();
    },

    /**
     * Client is registered - we can send request starting from this point of
     * time
     */
    onRPCRegistered: function() {

        Em.Logger.log("FFW.Navigation.onRPCRegistered");
        this._super();

        // subscribe to notifications
    },

    /**
     * Client is unregistered - no more requests
     */
    onRPCUnregistered: function() {

        Em.Logger.log("FFW.Navigation.onRPCUnregistered");
        this._super();

        // unsubscribe from notifications
    },

    /**
     * Client disconnected.
     */
    onRPCDisconnected: function() {

    },

    /**
     * when result is received from RPC component this function is called It is
     * the propriate place to check results of request execution Please use
     * previously store reuqestID to determine to which request repsonse belongs
     * to
     */
    onRPCResult: function(response) {

        Em.Logger.log("FFW.Navigation.onRPCResult");
        this._super();
    },

    /**
     * handle RPC erros here
     */
    onRPCError: function(error) {

        Em.Logger.log("FFW.Navigation.onRPCError");
        this._super();
    },

    /**
     * handle RPC notifications here
     */
    onRPCNotification: function(notification) {

        Em.Logger.log("FFW.Navigation.onRPCNotification");
        this._super();

        if (notification.method == this.onStopStreamNotification) {
            SDL.SDLModel.onStopStream(notification.params);
        }
    },

    /**
     * handle RPC requests here
     */
    onRPCRequest: function(request) {

        Em.Logger.log("FFW.Navigation.onRPCRequest");
        if (this.validationCheck(request)) {

            var resultCode = null;

            switch (request.method) {
                case "Navigation.IsReady": {

                    Em.Logger.log("FFW." + request.method + "Response");

                    // send repsonse
                    var JSONMessage = {
                        "jsonrpc": "2.0",
                        "id": request.id,
                        "result": {
                            "available": this.get('isReady'),
                            "code": SDL.SDLModel.resultCode["SUCCESS"],
                            "method": "Navigation.IsReady"
                        }
                    };

                    this.client.send(JSONMessage);

                    break;
                }
                case "Navigation.ShowConstantTBT": {

                    SDL.SDLModel.tbtActivate(request.params);
                    this.sendNavigationResult(SDL.SDLModel.resultCode["SUCCESS"],
                        request.id,
                        request.method);

                    break;
                }
                case "Navigation.UpdateTurnList": {

                    SDL.SDLModel.tbtTurnListUpdate(request.params);
                    this.sendNavigationResult(SDL.SDLModel.resultCode["SUCCESS"],
                        request.id,
                        request.method);

                    break;
                }
                case "Navigation.AlertManeuver": {

                    SDL.SDLModel.onNavigationAlertManeuver(request.params);

                    this.sendNavigationResult(SDL.SDLModel.resultCode["SUCCESS"],
                        request.id,
                        request.method);

                    break;
                }
                case "Navigation.StartStream": {

                    SDL.SDLModel.startStream(request.params);

                    this.sendNavigationResult(SDL.SDLModel.resultCode["SUCCESS"],
                        request.id,
                        request.method);

                    break;
                }
                case "Navigation.StopStream": {

                    SDL.SDLModel.stopStream(request.params);

                    this.sendNavigationResult(SDL.SDLModel.resultCode["SUCCESS"],
                        request.id,
                        request.method);

                    break;
                }
            }
        }
    },

    /**
     * Send error response from onRPCRequest
     * 
     * @param {Number}
     *            resultCode
     * @param {Number}
     *            id
     * @param {String}
     *            method
     */
    sendError: function(resultCode, id, method, message) {

        Em.Logger.log("FFW." + method + "Response");

        if (resultCode != SDL.SDLModel.resultCode["SUCCESS"]) {

            // send repsonse
            var JSONMessage = {
                "jsonrpc": "2.0",
                "id": id,
                "error": {
                    "code": resultCode, // type (enum) from SDL protocol
                    "message": message,
                    "data": {
                        "method": method
                    }
                }
            };
            this.client.send(JSONMessage);
        }
    },

    /**
     * send response from onRPCRequest
     * 
     * @param {Number}
     *            resultCode
     * @param {Number}
     *            id
     * @param {String}
     *            method
     */
    sendNavigationResult: function(resultCode, id, method) {

        Em.Logger.log("FFW.UI." + method + "Response");

        if (resultCode === SDL.SDLModel.resultCode["SUCCESS"]) {

            // send repsonse
            var JSONMessage = {
                "jsonrpc": "2.0",
                "id": id,
                "result": {
                    "code": resultCode, // type (enum) from SDL protocol
                    "method": method
                }
            };
            this.client.send(JSONMessage);
        }
    },

    /**
     * send response from onRPCRequest
     * 
     * @param {Number}
     *            resultCode
     * @param {Number}
     *            id
     */
    alertResponse: function(resultCode, id) {

        Em.Logger.log("FFW.Navigation.AlertResponse");

        if (resultCode === SDL.SDLModel.resultCode["SUCCESS"]) {

            // send repsonse
            var JSONMessage = {
                "jsonrpc": "2.0",
                "id": id,
                "result": {
                    "code": resultCode, // type (enum) from SDL protocol
                    "method": 'Navigation.Alert'
                }
            };
            this.client.send(JSONMessage);
        }
    },

    /**
     * Notifies if TBTClientState was activated
     * 
     * @param {String}
     *            state
     * @param {Number}
     *            appID
     */
    onTBTClientState: function(state, appID) {

        Em.Logger.log("FFW.Navigation.OnTBTClientState");

        // send repsonse
        var JSONMessage = {
            "jsonrpc": "2.0",
            "method": "Navigation.OnTBTClientState",
            "params": {
                "state": state
            }
        };
        this.client.send(JSONMessage);
    }
})