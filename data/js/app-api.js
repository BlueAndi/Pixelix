"use strict";

var utils = window.utils || {};

utils.getURLParameter = function (name) {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(name);
};

utils.enableForm = function (formId, enableIt) {
    var form = document.getElementById(formId);
    var elements = form.elements;
    var index = 0;

    for (index = 0; index < elements.length; ++index) {
        elements[index].disabled = (false === enableIt) ? true : false;
    }
};

utils.obj2FormData = function (obj, formData = new FormData()) {

    this.formData = formData;

    this.createFormData = function (obj, subKeyStr = "") {

        for (let i in obj) {
            let value = obj[i];
            let subKeyStrTrans;

            if (obj instanceof Array) {
                subKeyStrTrans = subKeyStr ? subKeyStr + "._" + i + "_" : i;
            } else {
                subKeyStrTrans = subKeyStr ? subKeyStr + "." + i : i;
            }

            if ((typeof (value) === "string") || (typeof (value) === "number") || (typeof (value) === "boolean") || (value instanceof File)) {

                this.formData.append(subKeyStrTrans, value);

            } else if (typeof (value) === "object") {

                this.createFormData(value, subKeyStrTrans);
            }
        }
    }

    this.createFormData(obj);

    return this.formData;
};

utils.makeRequest = function (options) {
    return new Promise(function (resolve, reject) {
        if ("object" !== typeof options) {
            reject({ msg: "Arguments are missing." });
        } else if ("string" !== typeof options.method) {
            reject({ msg: "Request method is missing." });
        } else if ("string" !== typeof options.url) {
            reject({ msg: "URL is missing." });
        } else {
            var xhr = new XMLHttpRequest();
            var formData = null;
            var urlEncodedPar = "";
            var isJsonResponse = false;
            var isFirst = true;
            var key;

            if ("object" === typeof options.formData) {
                formData = options.formData;
            }
            else if ("object" === typeof options.parameter) {
                if ("get" === options.method.toLowerCase()) {
                    urlEncodedPar += "?";

                    for (key in options.parameter) {
                        if (true === isFirst) {
                            isFirst = false;
                        } else {
                            urlEncodedPar += "&";
                        }
                        urlEncodedPar += encodeURIComponent(key);
                        urlEncodedPar += "=";
                        urlEncodedPar += encodeURIComponent(options.parameter[key]);
                    }
                } else {
                    formData = utils.obj2FormData(options.parameter);
                }
            }

            if ("boolean" === typeof options.isJsonResponse) {
                isJsonResponse = options.isJsonResponse;
            }

            xhr.open(options.method, options.url + urlEncodedPar);

            if ("undefined" !== typeof options.headers) {
                Object.keys(options.headers).forEach(function (key) {
                    xhr.setRequestHeader(key, options.headers[key]);
                });
            }

            if ("function" === typeof options.onProgress) {
                xhr.upload.onprogress = options.onProgress;
            }

            xhr.onload = function () {
                var jsonRsp = null;

                if (200 !== xhr.status) {
                    if (true === isJsonResponse) {
                        jsonRsp = JSON.parse(xhr.response);
                        reject(jsonRsp);
                    } else {
                        reject(xhr.response);
                    }
                } else {
                    if (true === isJsonResponse) {
                        jsonRsp = JSON.parse(xhr.response);

                        if ("ok" === jsonRsp.status) {
                            resolve(jsonRsp);
                        } else {
                            reject(jsonRsp);
                        }
                    } else {
                        resolve(xhr.response);
                    }
                }
            };

            xhr.ontimeout = function () {
                console.error(xhr.statusText);
                reject("Timeout");
            };

            xhr.onerror = function () {
                console.error(xhr.statusText);
                reject("Error");
            };

            if (null === formData) {
                xhr.send();
            } else {
                xhr.send(formData);
            }
        }
    });
};

utils.readJsonFile = function (file) {
    return new Promise(function (resolve, reject) {
        var rawFile = new XMLHttpRequest();

        rawFile.overrideMimeType("application/json");
        rawFile.open("GET", file, true);
        rawFile.onreadystatechange = function () {
            if ((4 === rawFile.readyState) && ("200" === rawFile.status)) {
                resolve(rawFile.responseText);
            }
        }
        rawFile.send(null);
    });
};

utils.checkBMPFile = function (file) {
    return new Promise(function (resolve, reject) {
        var reader = new FileReader();

        reader.onload = function (e) {
            resolve(e.target.result);
        };

        reader.readAsArrayBuffer(file);
    }).then(function (buffer) {
        var bitmapHeaderSize = 54;
        var header = new Uint8Array(buffer, 0, bitmapHeaderSize);
        var planes = (header[27] << 8) | (header[26] << 0);
        var bitsPerPixel = (header[29] << 8) | (header[28] << 0);
        var compression = (header[33] << 24) | (header[32] << 16) | (header[31] << 8) | (header[30] << 0);
        var paletteColors = (header[49] << 24) | (header[48] << 16) | (header[47] << 8) | (header[46] << 0);
        var promise = null;

        if ("BM" !== String.fromCharCode.apply(null, header.subarray(0, 2))) {
            promise = Promise.reject("No bitmap file.");
        } else if (1 !== planes) {
            promise = Promise.reject("Only 1 plane is supported.");
        } else if ((24 !== bitsPerPixel) && (32 !== bitsPerPixel)) {
            promise = Promise.reject("Only 24 or 32 bpp are supported.");
        } else if (0 !== compression) {
            promise = Promise.reject("No compression is supported.");
        } else if (0 !== paletteColors) {
            promise = Promise.reject("Color palette not supported.");
        } else {
            promise = Promise.resolve();
        }

        return promise;
    });
};


var dialog = window.dialog || {};

dialog._show = function (title, message, isBlocking) {
    return new Promise(function (resolve, reject) {

        var waitOnClick = false;

        if (("boolean" === typeof isBlocking) &&
            (true == isBlocking)) {
            waitOnClick = true;
        }

        $("#dialogTitle").text(title);
        $("#dialogBody").html(message);

        $("#modalDialog").on("shown.bs.modal", function () {
            $("#modalDialog").off("shown.bs.modal");

            if (false === waitOnClick) {
                resolve();
            }
        });

        $("#modalDialog").modal("show");

        if (true === waitOnClick) {
            $("#modalDialog .btn-secondary").click(function () {
                resolve();
            });
        }
    });
}

dialog.hide = function () {
    return new Promise(function (resolve, reject) {

        $("#modalDialog").on("hidden.bs.modal", function () {
            $("#modalDialog").off("hidden.bs.modal");
            resolve();
        });

        $("#modalDialog").modal("hide");
    });
}

dialog.showInfo = function (message, isBlocking) {
    var $btnClose = $("<button>")
        .attr("type", "button")
        .attr("class", "btn btn-secondary")
        .attr("data-bs-dismiss", "modal")
        .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-primary text-white");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Info", message, isBlocking);
}

dialog.showWarning = function (message, isBlocking) {
    var $btnClose = $("<button>")
        .attr("type", "button")
        .attr("class", "btn btn-secondary")
        .attr("data-bs-dismiss", "modal")
        .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-warning");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Warning", message, isBlocking);
}

dialog.showError = function (message, isBlocking) {
    var $btnClose = $("<button>")
        .attr("type", "button")
        .attr("class", "btn btn-secondary")
        .attr("data-bs-dismiss", "modal")
        .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-danger text-white");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Error", message, isBlocking);
}

dialog.show = function (title, message, isBlocking) {
    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-dark text-white");

    return dialog._show(title, message, isBlocking);
}

dialog.showYesNo = function (title, message, onYes, onNo, isBlocking) {
    var $btnYes = $("<button>")
        .attr("type", "button")
        .attr("class", "btn btn-secondary")
        .attr("data-bs-dismiss", "modal")
        .text("Yes")
        .on("click", function () {
            if (typeof onYes === "function") onYes();
        });

    var $btnNo = $("<button>")
        .attr("type", "button")
        .attr("class", "btn btn-secondary")
        .attr("data-bs-dismiss", "modal")
        .text("No")
        .on("click", function () {
            if (typeof onNo === "function") onNo();
        });

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-warning");
    $("#dialogFooter").append($btnYes).append($btnNo);

    return dialog._show(title, message, isBlocking);
};

dialog.updateMessage = function (message) {
    $("#dialogBody").html(message);
};

var pixelix = window.pixelix || {};

pixelix.rest = {};

pixelix.rest.Client = function (options) {
    this._hostname = "";
    this._baseUri = "/rest/api/v1";

    if ("object" === typeof options) {
        if ("string" === typeof options.hostname) {
            this._hostname = options.hostname;
        }
    }
};

pixelix.rest.Client.prototype.getBaseUri = function () {
    return this._baseUri;
}

pixelix.rest.Client.prototype.listFiles = function (path = "/", page = "0") {
    return utils.makeRequest({
        method: "GET",
        url: this._hostname + this._baseUri + "/fs",
        isJsonResponse: true,
        parameter: {
            dir: path,
            page: page
        }
    });
};

pixelix.rest.Client.prototype.listAllFiles = function (path = "/") {
    var page = 0;
    var data = [];
    var client = this;
    var handler = function (rsp) {
        var promise = null;

        if (0 < rsp.data.length) {
            data = data.concat(rsp.data);
            ++page;
            promise = client.listFiles(path, page).then(handler);
        } else {
            promise = Promise.resolve(data);
        }
        return promise;
    };

    return this.listFiles(path, page).then(handler);
};

pixelix.rest.Client.prototype.listAllFilesRecursive = function (path = "/") {
    var data = [];
    var client = this;
    var handler = function (directory) {
        var promise = null;
        var idx = 0;
        var listOfDirectoryIndizes = [];

        if (0 < directory.listing.length) {

            for (idx = 0; idx < directory.listing.length; ++idx) {
                if (directory.listing[idx].type === "dir") {
                    listOfDirectoryIndizes.push(idx);
                }
            }

            if (0 < listOfDirectoryIndizes.length) {

                promise = Promise.resolve();

                for (idx = 0; idx < listOfDirectoryIndizes.length; idx++) {
                    promise = promise.then(function () {
                        var directoryIdx = listOfDirectoryIndizes.shift();

                        return client.listAllFiles(directory.listing[directoryIdx].name).then(function (rsp) {
                            var item = directory.listing[directoryIdx];
                            item.listing = rsp;

                            return Promise.resolve(item);
                        }).then(handler);
                    });
                }

                promise = promise.then(function () {
                    return Promise.resolve(directory);
                });

            } else {
                promise = Promise.resolve(directory);
            }

        } else {
            promise = Promise.resolve(directory);
        }

        return promise;
    };

    return this.listAllFiles(path).then(function (rsp) {
        return Promise.resolve({
            name: path,
            size: 0,
            type: "dir",
            listing: rsp
        });
    }).then(handler);
};

pixelix.rest.Client.prototype.readFile = function (filename) {
    var promise = null;
    if ("string" !== typeof filename) {
        promise = Promise.reject();
    } else {
        promise = utils.makeRequest({
            method: "GET",
            url: this._hostname + this._baseUri + "/fs/file",
            isJsonResponse: false,
            parameter: {
                path: filename
            }
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.writeFile = function (filename, content, mimeType) {
    var promise = null;
    var formData = null;

    if ("string" !== typeof filename) {
        promise = Promise.reject();
    } else if ("string" !== typeof mimeType) {
        promise = Promise.reject();
    } else {
        formData = new FormData();
        formData.append("file", new Blob([content], { type: mimeType }), filename);

        promise = utils.makeRequest({
            method: "POST",
            url: this._hostname + this._baseUri + "/fs/file",
            isJsonResponse: true,
            formData: formData
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.removeFile = function (filename) {
    var promise = null;

    if ("string" !== typeof filename) {
        promise = Promise.reject();
    } else {
        promise = utils.makeRequest({
            method: "DELETE",
            url: this._hostname + this._baseUri + "/fs/file",
            isJsonResponse: true,
            parameter: {
                path: filename
            }
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.getPluginInstances = function () {
    return utils.makeRequest({
        method: "GET",
        url: this._baseUri + "/display/slots",
        isJsonResponse: true
    });
};

pixelix.rest.Client.prototype.getSensors = function () {
    return utils.makeRequest({
        method: "GET",
        url: this._baseUri + "/sensors",
        isJsonResponse: true
    });
};

pixelix.rest.Client.prototype.getSettingKeys = function () {
    return utils.makeRequest({
        method: "GET",
        url: this._baseUri + "/settings",
        isJsonResponse: true
    });
};

pixelix.rest.Client.prototype.getSettingByKey = function (key) {
    var promise = null;

    if ("string" !== typeof key) {
        promise = Promise.reject();
    } else {
        promise = utils.makeRequest({
            method: "GET",
            url: this._baseUri + "/setting",
            isJsonResponse: true,
            parameter: {
                key: key
            }
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.setSetting = function (key, value) {
    var promise = null;

    if ("string" !== typeof key) {
        promise = Promise.reject();
    } else if ("undefined" === typeof value) {
        promise = Promise.reject();
    } else {
        promise = utils.makeRequest({
            method: "POST",
            url: this._baseUri + "/setting",
            isJsonResponse: true,
            parameter: {
                key: key,
                value: value
            }
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.restart = function () {
    return utils.makeRequest({
        method: "POST",
        url: this._baseUri + "/restart",
        isJsonResponse: true
    });
};

pixelix.rest.Client.prototype.fileMgrUploadFile = function (file, fileSize) {
    var promise = null;

    if ("object" !== typeof file) {
        promise = Promise.reject();
    } else if ("number" !== typeof fileSize) {
        promise = Promise.reject();
    } else {
        promise = utils.makeRequest({
            method: "POST",
            url: this._hostname + this._baseUri + "/fileMgrService/upload",
            isJsonResponse: true,
            parameter: {
                file: file
            },
            headers: {
                "X-File-Size": fileSize
            }
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.fileMgrRemoveFile = function (fileId) {
    var promise = null;

    if ("number" !== typeof fileId) {
        promise = Promise.reject();
    } else {
        promise = utils.makeRequest({
            method: "POST",
            url: this._hostname + this._baseUri + "/fileMgrService/remove",
            isJsonResponse: true,
            parameter: {
                fileId: fileId
            }
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.getDisplayState = function () {
    return utils.makeRequest({
        method: "GET",
        url: this._baseUri + "/display/power",
        isJsonResponse: true
    });
};

pixelix.rest.Client.prototype.setDisplayState = function (state) {
    var promise = null;

    if ("string" !== typeof state) {
        promise = Promise.reject();
    } else {
        promise = utils.makeRequest({
            method: "POST",
            url: this._baseUri + "/display/power",
            isJsonResponse: true,
            parameter: {
                state: state
            }
        });
    }

    return promise;
};
