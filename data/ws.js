var pixelix = pixelix | {};

pixelix = {
    ws: {}
};

pixelix.ws.Client = function(options) {

    this.socket     = null;
    this.cmd        = "";
    this.resolve    = null;
    this.reject     = null;
};

pixelix.ws.Client.prototype.connect = function(options) {
    return new Promise(function(resolve, reject) {
        var wsUrl;

        if (("string" !== typeof options.protocol) ||
            ("string" !== typeof options.hostname) ||
            ("number" !== typeof options.port) ||
            ("string" !== typeof options.endpoint)) {

            reject();
        } else {

            try {
                wsUrl = options.protocol + "://" + options.hostname + ":" + options.port + options.endpoint;
                this.socket = new WebSocket(wsUrl);

                this.socket.onopen = function(openEvent) {
                    console.debug("Websocket opened.");
                    resolve(this);
                }.bind(this);

                this.socket.onclose = function(closeEvent) {
                    console.debug("Websocket closed.");
                    if ("function" === typeof options.onClosed) {
                        options.onClosed();
                    }
                };

                this.socket.onmessage = function(messageEvent) {
                    console.debug("Websocket message: " + messageEvent.data);
                    this._onMessage(messageEvent.data);
                }.bind(this);

            } catch (exception) {
                console.error(exception);
                reject();
            }
        }
    }.bind(this));
};

pixelix.ws.Client.prototype._onMessage = function(msg) {
    var data    = msg.split(";");
    var status  = data.shift();
    var rsp     = {};

    if ("ACK" === status) {
        if ("GETDISP" === this.cmd) {
            rsp.slotId = data.shift();
            rsp.data = data;
            this.resolve(rsp);
        } else if ("INSTALL" === this.cmd) {
            rsp.slotId = parseInt(data[0]);
            this.resolve(rsp);
        } else if ("PLUGINS" === this.cmd) {
            rsp.plugins = data;
            this.resolve(rsp);
        } else if ("SLOTS" === this.cmd) {
            rsp.maxSlots = parseInt(data.shift());
            rsp.slots = data;
            this.resolve(rsp);
        } else if ("UNINSTALL" === this.cmd) {
            this.resolve(rsp);
        } else {
            console.error("Unknown command: " + this.cmd);
            this.reject();
        }
    } else {
        console.error("Command " + this.cmd + " failed.");
        this.reject();
    }

    this.cmd = "";
    this.resolve = null;
    this.reject = null;

    return;
};

pixelix.ws.Client.prototype.getDisplayContent = function() {
    return new Promise(function(resolve, reject) {

        if (null === this.socket) {
            reject();
        } else {
            this.cmd = "GETDISP";
            this.resolve = resolve;
            this.reject = reject;
            this.socket.send(this.cmd);
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getSlots = function() {
    return new Promise(function(resolve, reject) {

        if (null === this.socket) {
            reject();
        } else {
            this.cmd = "SLOTS";
            this.resolve = resolve;
            this.reject = reject;
            this.socket.send(this.cmd);
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getPlugins = function() {
    return new Promise(function(resolve, reject) {

        if (null === this.socket) {
            reject();
        } else {
            this.cmd = "PLUGINS";
            this.resolve = resolve;
            this.reject = reject;
            this.socket.send(this.cmd);
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.install = function(options) {
    return new Promise(function(resolve, reject) {

        if (null === this.socket) {
            reject();
        } else if ("string" !== typeof options.pluginName) {
            reject();
        } else {
            this.cmd = "INSTALL;" + options.pluginName;
            this.resolve = resolve;
            this.reject = reject;
            this.socket.send(this.cmd);
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.uninstall = function(options) {
    return new Promise(function(resolve, reject) {

        if (null === this.socket) {
            reject();
        } else if ("number" !== typeof options.slotId) {
            reject();
        } else {
            this.cmd = "UNINSTALL;" + options.slotId;
            this.resolve = resolve;
            this.reject = reject;
            this.socket.send(this.cmd);
        }
    }.bind(this));
};
