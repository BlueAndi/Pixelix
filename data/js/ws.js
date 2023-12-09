var pixelix = window.pixelix || {};

pixelix.ws = {};

pixelix.ws.getLogLevelStr = function(logLevel) {
    var str = "";

    switch(logLevel)
    {
    case 0:
        str = "FATAL";
        break;

    case 1:
        str = "ERROR";
        break;

    case 2:
        str = "WARNING";
        break;

    case 3:
        str = "INFO";
        break;

    case 4:
        str = "DEBUG";
        break;

    case 5:
        str = "TRACE";
        break;

    default:
        str = "UNKNWON";
        break;
    }

    return str;
};

pixelix.ws.Client = function(options) {

    this._socket        = null;
    this._cmdQueue      = [];
    this._pendingCmd    = null;
    this._onEvent       = null;

    this._sendCmdFromQueue = function() {
        var msg = "";

        if (0 < this._cmdQueue.length) {
            this._pendingCmd = this._cmdQueue.shift();

            msg = this._pendingCmd.name;

            if (null !== this._pendingCmd.par) {
                msg += ";" + this._pendingCmd.par;
            }

            console.info("Websocket command: " + msg);
            this._socket.send(msg);
        }
    };

    this._sendCmd = function(cmd) {

        this._cmdQueue.push(cmd);

        if (null === this._pendingCmd) {
            this._sendCmdFromQueue();
        }
    };

    this._sendEvt = function(evt) {
        if (null !== this._onEvent) {
            this._onEvent(evt);
        }
    };
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

            if ("function" === typeof options.onEvent) {
                this._onEvent = options.onEvent;
            }

            try {
                wsUrl = options.protocol + "://" + options.hostname + ":" + options.port + options.endpoint;
                this._socket = new WebSocket(wsUrl);

                this._socket.onopen = function(openEvent) {
                    console.debug("Websocket opened.");
                    resolve(this);
                }.bind(this);

                this._socket.onclose = function(closeEvent) {
                    console.debug("Websocket closed.");
                    if ("function" === typeof options.onClosed) {
                        options.onClosed();
                    }

                    this._socket = null;
                };

                this._socket.onmessage = function(messageEvent) {
                    console.debug("Websocket message: " + messageEvent.data);
                    this._onMessage(messageEvent.data);
                }.bind(this);

            } catch (exception) {
                console.error(exception);

                if (null !== this._socket) {
                    this._socket.close();
                    this._socket = null;
                }

                reject();
            }
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.disconnect = function(options) {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._socket.close();
            resolve();
        }
    }.bind(this));
};

pixelix.ws.Client.prototype._onMessage = function(msg) {
    var data        = msg.split(";");
    var status      = data.shift();
    var rsp         = {};
    var index       = 0;
    var elements    = 0;

    if ("EVT" === status) {
        rsp.evtType = data.shift();

        if ("LOG" === rsp.evtType) {
            rsp.timestamp = parseInt(data[0]);
            rsp.level = parseInt(data[1]);
            rsp.filename = data[2].substring(1, data[2].length - 1);
            rsp.line = parseInt(data[3]);
            rsp.text = data[4].substring(1, data[4].length - 1);
        }

        this._sendEvt(rsp);
    } else {
        if (null === this._pendingCmd) {
            console.error("No pending command, but response received.");
        } else if ("ACK" === status) {
            if ("ALIAS" === this._pendingCmd.name) {
                rsp.name = data[0];
                this._pendingCmd.resolve(rsp);
            } else if ("GETDISP" === this._pendingCmd.name) {
                rsp.slotId = parseInt(data.shift());
                rsp.width = parseInt(data.shift());
                rsp.height = parseInt(data.shift());
                rsp.data = [];
                for(index = 0; index < data.length; ++index) {
                    rsp.data.push(parseInt(data[index], 16));
                }
                this._pendingCmd.resolve(rsp);
            } else if ("BRIGHTNESS" === this._pendingCmd.name) {
                rsp.brightness = parseInt(data[0]);
                rsp.automaticBrightnessControl = (1 === parseInt(data[1])) ? true : false;
                this._pendingCmd.resolve(rsp);
            } else if ("BUTTON" === this._pendingCmd.name) {
                this._pendingCmd.resolve(rsp);
            } else if ("EFFECT" === this._pendingCmd.name) {
                rsp.fadeEffect = parseInt(data[0]);
                this._pendingCmd.resolve(rsp);
            } else if ("INSTALL" === this._pendingCmd.name) {
                rsp.slotId = parseInt(data[0]);
                rsp.uid = parseInt(data[1]);
                this._pendingCmd.resolve(rsp);
            } else if ("IPERF" === this._pendingCmd.name) {
                rsp.isEnabled = (0 === parseInt(data[0])) ? false : true;
                this._pendingCmd.resolve(rsp);
            } else if ("LOG" === this._pendingCmd.name) {
                rsp.isEnabled = (0 === parseInt(data[0])) ? false : true;
                this._pendingCmd.resolve(rsp);
            } else if ("MOVE" === this._pendingCmd.name) {
                this._pendingCmd.resolve(rsp);
            } else if ("PLUGINS" === this._pendingCmd.name) {
                rsp.plugins = [];
                for(index = 0; index < data.length; ++index) {
                    rsp.plugins.push(data[index].substring(1, data[index].length - 1));
                }
                this._pendingCmd.resolve(rsp);
            } else if ("RESET" === this._pendingCmd.name) {
                this._pendingCmd.resolve(rsp);
            } else if ("SLOT_DURATION" === this._pendingCmd.name) {
                rsp.duration = parseInt(data[0]);
                this._pendingCmd.resolve(rsp);
            } else if ("SLOTS" === this._pendingCmd.name) {
                rsp.maxSlots = parseInt(data.shift());
                rsp.slots = [];
                elements = 6;
                for(index = 0; index < (data.length / elements); ++index) {
                    rsp.slots.push({
                        name: data[elements * index + 0].substring(1, data[elements * index + 0].length - 1),
                        uid: parseInt(data[elements * index + 1]),
                        alias: data[elements * index + 2].substring(1, data[elements * index + 2].length - 1),
                        isLocked: (0 == parseInt(data[elements * index + 3])) ? false : true,
                        isSticky: (0 == parseInt(data[elements * index + 4])) ? false : true,
                        duration: parseInt(data[elements * index + 5])
                    });
                }
                this._pendingCmd.resolve(rsp);
            } else if ("UNINSTALL" === this._pendingCmd.name) {
                this._pendingCmd.resolve(rsp);
            } else {
                console.error("Unknown command: " + this._pendingCmd.name);
                this._pendingCmd.reject();
            }
        } else {
            console.error("Command " + this._pendingCmd.name + " failed.");

            if (0 < data.length) {
                this._pendingCmd.reject(this._pendingCmd.name + ": " + data[0]);
            } else {
                this._pendingCmd.reject(this._pendingCmd.name + ": Unknown error.");
            }
        }

        this._pendingCmd = null;
        this._sendCmdFromQueue();
    }

    return;
};

pixelix.ws.Client.prototype.getDisplayContent = function() {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "GETDISP",
                par: null,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getSlots = function() {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "SLOTS",
                par: null,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.reset = function() {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "RESET",
                par: null,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getBrightness = function() {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "BRIGHTNESS",
                par: null,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.setBrightness = function(options) {
    return new Promise(function(resolve, reject) {
        var par = "";

        if (null === this._socket) {
            reject();
        } else if ("number" !== typeof options.brightness) {
            reject();
        } else {

            par += options.brightness;

            if ("boolean" === typeof options.automaticBrightnessControl) {
                par += ";";
                par += (false == options.automaticBrightnessControl) ? 0 : 1;
            }

            this._sendCmd({
                name: "BRIGHTNESS",
                par: par,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getPlugins = function() {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "PLUGINS",
                par: null,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.install = function(options) {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else if ("string" !== typeof options.pluginName) {
            reject();
        } else {
            this._sendCmd({
                name: "INSTALL",
                par: "\"" + options.pluginName + "\"",
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.uninstall = function(options) {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else if ("number" !== typeof options.slotId) {
            reject();
        } else {
            this._sendCmd({
                name: "UNINSTALL",
                par: options.slotId,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getLog = function(options) {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "LOG",
                par: null,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.setLog = function(options) {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else if ("boolean" !== typeof options.enable) {
            reject();
        } else {
            this._sendCmd({
                name: "LOG",
                par: (false == options.enable) ? 0 : 1,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.move = function(options) {
    return new Promise(function(resolve, reject) {
        var par = "";

        if (null === this._socket) {
            reject();
        } else if ("number" !== typeof options.uid) {
            reject();
        } else if ("number" !== typeof options.slotId) {
            reject();
        } else {

            par = options.uid;
            par += ";";
            par += options.slotId;

            this._sendCmd({
                name: "MOVE",
                par: par,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getSlotDuration = function(options) {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else if ("number" !== typeof options.slotId) {
            reject();
        } else {
            this._sendCmd({
                name: "SLOT_DURATION",
                par: options.slotId,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.setSlotDuration = function(options) {
    return new Promise(function(resolve, reject) {
        var par = "";
        if (null === this._socket) {
            reject();
        } else if ("number" !== typeof options.slotId) {
            reject();
        } else if ("number" !== typeof options.duration) {
            reject();
        } else {

            par += options.slotId;
            par += ";";
            par += options.duration;

            this._sendCmd({
                name: "SLOT_DURATION",
                par: par,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getIperf = function(options) {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "IPERF",
                par: null,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.startIperf = function(options) {
    return new Promise(function(resolve, reject) {
        var par = "START";
        if (null === this._socket) {
            reject();
        } else {

            if ("string" === typeof options) {

                if ("string" === typeof options.protocol) {
                    par += ";";
                    par += options.protocol;
                } else {
                    par += ";DEFAULT"
                }

                if ("number" === typeof options.interval) {
                    par += ";";
                    par += options.interval;
                } else {
                    par += ";DEFAULT"
                }

                if ("number" === typeof options.time) {
                    par += ";";
                    par += options.time;
                } else {
                    par += ";DEFAULT"
                }
            }

            this._sendCmd({
                name: "IPERF",
                par: par,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.stopIperf = function(options) {
    return new Promise(function(resolve, reject) {
        var par = "STOP";

        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "IPERF",
                par: par,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.triggerButton = function(options) {
    return new Promise(function(resolve, reject) {
        var par = null;

        if (null === this._socket) {
            reject();
        } else {

            if ("number" === typeof options.actionId) {
                par = options.actionId.toString();
            }

            this._sendCmd({
                name: "BUTTON",
                par: par,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.setFadeEffect = function(options) {
    return new Promise(function(resolve, reject) {
        var par = "";

        if (null === this._socket) {
            reject();
        } else if ("number" !== typeof options.fadeEffect) {
            reject();
        } else {

            par += options.fadeEffect;

            this._sendCmd({
                name: "EFFECT",
                par: par,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getFadeEffect = function() {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else {
            this._sendCmd({
                name: "EFFECT",
                par: null,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.getPluginAlias = function(options) {
    return new Promise(function(resolve, reject) {
        if (null === this._socket) {
            reject();
        } else if ("number" !== typeof options.uuid) {
            reject();
        } else {
            this._sendCmd({
                name: "ALIAS",
                par: options.uuid,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};

pixelix.ws.Client.prototype.setPluginAlias = function(options) {
    return new Promise(function(resolve, reject) {
        var par = "";
        if (null === this._socket) {
            reject();
        } else if ("number" !== typeof options.uid) {
            reject();
        } else if ("string" !== typeof options.name) {
            reject();
        } else {

            par += options.uid;
            par += ";";
            par += options.name;

            this._sendCmd({
                name: "ALIAS",
                par: par,
                resolve: resolve,
                reject: reject
            });
        }
    }.bind(this));
};
