"use strict";

/* "pixelix" is a shared namespace (also declared in app-api.js). Keep it as var:
 * in classic scripts only var permits the same top-level name across files. */
var pixelix = window.pixelix || {};

pixelix.ws = pixelix.ws || {};

pixelix.ws.getLogLevelStr = function (logLevel) {
    switch (logLevel) {
    case 0:
        return "FATAL";
    case 1:
        return "ERROR";
    case 2:
        return "WARNING";
    case 3:
        return "INFO";
    case 4:
        return "DEBUG";
    case 5:
        return "TRACE";
    default:
        return "UNKNOWN";
    }
};

pixelix.ws.Client = class {
    constructor() {
        this._socket = null;
        this._cmdQueue = [];
        this._pendingCmd = null;
        this._onEvent = null;
    }

    _sendCmdFromQueue() {
        if (this._cmdQueue.length > 0) {
            this._pendingCmd = this._cmdQueue.shift();

            let msg = this._pendingCmd.name;

            if (this._pendingCmd.par !== null) {
                msg += ";" + this._pendingCmd.par;
            }

            console.info("Websocket command: " + msg);
            this._socket.send(msg);
        }
    }

    _sendCmd(cmd) {
        this._cmdQueue.push(cmd);

        if (this._pendingCmd === null) {
            this._sendCmdFromQueue();
        }
    }

    _sendEvt(evt) {
        if (this._onEvent !== null) {
            this._onEvent(evt);
        }
    }

    _quote(text) {
        return "\"" + text + "\"";
    }

    _removeQuotes(text) {
        let result = text;

        if ((text.length >= 2) &&
            (text.charAt(0) === "\"") &&
            (text.charAt(text.length - 1) === "\"")) {
            result = text.substring(1, text.length - 1);
        }

        return result;
    }

    _toBoolean(value) {
        return (parseInt(value) === 0) ? false : true;
    }

    _boolToInt(value) {
        return (value === false) ? 0 : 1;
    }

    connect(options) {
        return new Promise((resolve, reject) => {
            if ((typeof options.protocol !== "string") ||
                (typeof options.hostname !== "string") ||
                (typeof options.port !== "number") ||
                (typeof options.endpoint !== "string")) {
                reject();
                return;
            }

            if (typeof options.onEvent === "function") {
                this._onEvent = options.onEvent;
            }

            try {
                const wsUrl = options.protocol + "://" + options.hostname + ":" + options.port + options.endpoint;

                this._socket = new WebSocket(wsUrl);

                this._socket.onopen = () => {
                    console.debug("Websocket opened.");
                    resolve(this);
                };

                this._socket.onclose = () => {
                    console.debug("Websocket closed.");

                    if (typeof options.onClosed === "function") {
                        options.onClosed();
                    }

                    this._socket = null;
                };

                this._socket.onmessage = (messageEvent) => {
                    console.debug("Websocket message: " + messageEvent.data);
                    this._onMessage(messageEvent.data);
                };
            } catch (exception) {
                console.error(exception);

                if (this._socket !== null) {
                    this._socket.close();
                    this._socket = null;
                }

                reject();
            }
        });
    }

    disconnect() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._socket.close();
                resolve();
            }
        });
    }

    _onMessage(msg) {
        const data = msg.split(";");
        const status = data.shift();
        const rsp = {};
        let index = 0;
        let elements = 0;

        if (status === "EVT") {
            rsp.evtType = data.shift();

            if (rsp.evtType === "LOG") {
                rsp.timestamp = parseInt(data[0]);
                rsp.level = parseInt(data[1]);
                rsp.filename = data[2].substring(1, data[2].length - 1);
                rsp.line = parseInt(data[3]);
                rsp.text = data[4].substring(1, data[4].length - 1);
            }

            this._sendEvt(rsp);
        } else {
            if (this._pendingCmd === null) {
                console.error("No pending command, but response received.");
            } else if (status === "ACK") {
                if (this._pendingCmd.name === "ALIAS") {
                    rsp.name = data[0];
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "GETDISP") {
                    rsp.slotId = parseInt(data.shift());
                    rsp.data = [];
                    for (index = 0; index < data.length; ++index) {
                        rsp.data.push(parseInt(data[index], 16));
                    }
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "BRIGHTNESS") {
                    rsp.brightness = parseInt(data[0]);
                    rsp.minBrightness = parseInt(data[1]);
                    rsp.maxBrightness = parseInt(data[2]);
                    rsp.automaticBrightnessControl = this._toBoolean(data[3]);
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "BUTTON") {
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "EFFECT") {
                    rsp.fadeEffect = parseInt(data[0]);
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "FONTTYPE") {
                    rsp.fontSize = data[0];
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "INSTALL") {
                    rsp.slotId = parseInt(data[0]);
                    rsp.uid = this._removeQuotes(data[1]);
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "IPERF") {
                    rsp.isEnabled = this._toBoolean(data[0]);
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "LOG") {
                    rsp.isEnabled = this._toBoolean(data[0]);
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "MOVE") {
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "PLUGINS") {
                    rsp.plugins = [];
                    for (index = 0; index < data.length; ++index) {
                        rsp.plugins.push(this._removeQuotes(data[index]));
                    }
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "RESTART") {
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "SLOT_DURATION") {
                    rsp.duration = parseInt(data[0]);
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "SLOT") {
                    rsp.slotId = parseInt(data[0]);
                    rsp.name = this._removeQuotes(data[1]);
                    rsp.uid = parseInt(data[2]);
                    rsp.alias = this._removeQuotes(data[3]);
                    rsp.isLocked = this._toBoolean(data[4]);
                    rsp.isSticky = this._toBoolean(data[5]);
                    rsp.isDisabled = this._toBoolean(data[6]);
                    rsp.duration = parseInt(data[7]);
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "SLOTS") {
                    rsp.maxSlots = parseInt(data.shift());
                    rsp.slots = [];
                    elements = 8;
                    for (index = 0; index < (data.length / elements); ++index) {
                        rsp.slots.push({
                            name: this._removeQuotes(data[elements * index + 0]),
                            uid: parseInt(data[elements * index + 1]),
                            alias: this._removeQuotes(data[elements * index + 2]),
                            fontType: this._removeQuotes(data[elements * index + 3]),
                            isLocked: this._toBoolean(data[elements * index + 4]),
                            isSticky: this._toBoolean(data[elements * index + 5]),
                            isDisabled: this._toBoolean(data[elements * index + 6]),
                            duration: parseInt(data[elements * index + 7])
                        });
                    }
                    this._pendingCmd.resolve(rsp);
                } else if (this._pendingCmd.name === "UNINSTALL") {
                    this._pendingCmd.resolve(rsp);
                } else {
                    console.error("Unknown command: " + this._pendingCmd.name);
                    this._pendingCmd.reject();
                }
            } else {
                console.error("Command " + this._pendingCmd.name + " failed.");

                if (data.length > 0) {
                    this._pendingCmd.reject(this._pendingCmd.name + ": " + data[0]);
                } else {
                    this._pendingCmd.reject(this._pendingCmd.name + ": Unknown error.");
                }
            }

            this._pendingCmd = null;
            this._sendCmdFromQueue();
        }
    }

    getDisplayContent() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "GETDISP",
                    par: null,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    getSlots() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "SLOTS",
                    par: null,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    restart() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "RESTART",
                    par: null,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    getBrightness() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "BRIGHTNESS",
                    par: null,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    setBrightness(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.brightness !== "number") {
                reject();
            } else if (typeof options.minBrightness !== "number") {
                reject();
            } else if (typeof options.maxBrightness !== "number") {
                reject();
            } else if (typeof options.automaticBrightnessControl !== "boolean") {
                reject();
            } else {
                let par = "";

                par += options.brightness;
                par += ";";
                par += options.minBrightness;
                par += ";";
                par += options.maxBrightness;
                par += ";";
                par += this._boolToInt(options.automaticBrightnessControl);

                this._sendCmd({
                    name: "BRIGHTNESS",
                    par: par,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    getPlugins() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "PLUGINS",
                    par: null,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    install(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.pluginName !== "string") {
                reject();
            } else {
                this._sendCmd({
                    name: "INSTALL",
                    par: this._quote(options.pluginName),
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    uninstall(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.slotId !== "number") {
                reject();
            } else {
                this._sendCmd({
                    name: "UNINSTALL",
                    par: options.slotId,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    getLog() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "LOG",
                    par: null,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    setLog(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.enable !== "boolean") {
                reject();
            } else {
                this._sendCmd({
                    name: "LOG",
                    par: this._boolToInt(options.enable),
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    move(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.uid !== "number") {
                reject();
            } else if (typeof options.slotId !== "number") {
                reject();
            } else {
                let par = "";

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
        });
    }

    getSlotDuration(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.slotId !== "number") {
                reject();
            } else {
                this._sendCmd({
                    name: "SLOT_DURATION",
                    par: options.slotId,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    setSlotDuration(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.slotId !== "number") {
                reject();
            } else if (typeof options.duration !== "number") {
                reject();
            } else {
                let par = "";

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
        });
    }

    setSlot(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.slotId !== "number") {
                reject();
            } else {
                let par = "";

                par += options.slotId;
                par += ";";

                if (typeof options.sticky !== "boolean") {
                    par += "0";
                } else {
                    par += (options.sticky === false) ? "1" : "2";
                }

                par += ";";

                if (typeof options.isDisabled !== "boolean") {
                    par += "0";
                } else {
                    par += (options.isDisabled === false) ? "1" : "2";
                }

                this._sendCmd({
                    name: "SLOT",
                    par: par,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    getIperf() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "IPERF",
                    par: null,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    startIperf(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                let par = "START";

                if (typeof options === "object") {
                    if (typeof options.protocol === "string") {
                        par += ";";
                        par += options.protocol;
                    } else {
                        par += ";DEFAULT";
                    }

                    if (typeof options.interval === "number") {
                        par += ";";
                        par += options.interval;
                    } else {
                        par += ";DEFAULT";
                    }

                    if (typeof options.time === "number") {
                        par += ";";
                        par += options.time;
                    } else {
                        par += ";DEFAULT";
                    }
                }

                this._sendCmd({
                    name: "IPERF",
                    par: par,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    stopIperf() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "IPERF",
                    par: "STOP",
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    triggerButton(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                let par = null;

                if (typeof options.actionId === "number") {
                    par = options.actionId.toString();
                }

                this._sendCmd({
                    name: "BUTTON",
                    par: par,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    setFadeEffect(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.fadeEffect !== "number") {
                reject();
            } else {
                let par = "";

                par += options.fadeEffect;

                this._sendCmd({
                    name: "EFFECT",
                    par: par,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    getFadeEffect() {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else {
                this._sendCmd({
                    name: "EFFECT",
                    par: null,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    getPluginAlias(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.uuid !== "number") {
                reject();
            } else {
                this._sendCmd({
                    name: "ALIAS",
                    par: options.uuid,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    setPluginAlias(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.uid !== "number") {
                reject();
            } else if (typeof options.name !== "string") {
                reject();
            } else {
                let par = "";

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
        });
    }

    getPluginFontType(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.uuid !== "number") {
                reject();
            } else {
                this._sendCmd({
                    name: "FONTTYPE",
                    par: options.uuid,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }

    setPluginFontType(options) {
        return new Promise((resolve, reject) => {
            if (this._socket === null) {
                reject();
            } else if (typeof options.uid !== "number") {
                reject();
            } else if (typeof options.fontType !== "string") {
                reject();
            } else {
                let par = "";

                par += options.uid;
                par += ";";
                par += options.fontType;

                this._sendCmd({
                    name: "FONTTYPE",
                    par: par,
                    resolve: resolve,
                    reject: reject
                });
            }
        });
    }
};
