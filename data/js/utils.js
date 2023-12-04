var utils = window.utils || {};

utils.enableForm = function(formId, enableIt) {
    var form        = document.getElementById(formId);
    var elements    = form.elements;
    var index       = 0;

    for (index = 0; index < elements.length; ++index) {
        elements[index].disabled = (false === enableIt) ? true : false;
    }
};

utils.injectOrigin = function(name, searchFor) {
    var elements = document.getElementsByName(name);
    var index   = 0;

    for(index = 0; index < elements.length; ++index) {
        elements[index].innerHTML = elements[index].innerHTML.replace(searchFor, location.origin);
    }
};

utils.makeRequest = function(options) {
    return new Promise(function(resolve, reject) {
        if ("object" !== typeof options) {
            reject({ msg: "Arguments are missing." });
        } else if ("string" !== typeof options.method) {
            reject({ msg: "Request method is missing." });
        } else if ("string" !== typeof options.url) {
            reject({ msg: "URL is missing." });
        } else {
            var xhr             = new XMLHttpRequest();
            var formData        = null;
            var urlEncodedPar   = "";
            var isJsonResponse  = false;
            var isFirst         = true;
            var key;

            if ("object" === typeof options.formData) {
                formData = options.formData;
            }
            else if ("object" === typeof options.parameter) {
                if ("get" === options.method.toLowerCase()) {
                    urlEncodedPar += "?";

                    for(key in options.parameter) {
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
                    formData = new FormData();

                    for(key in options.parameter) {
                        formData.append(key, options.parameter[key]);
                    }
                }
            }

            if ("boolean" === typeof options.isJsonResponse) {
                isJsonResponse = options.isJsonResponse;
            }

            xhr.open(options.method, options.url + urlEncodedPar);

            if ("undefined" !== typeof options.headers) {
                Object.keys(options.headers).forEach(function(key) {
                    xhr.setRequestHeader(key, options.headers[key]);
                });
            }

            if ("function" === typeof options.onProgress) {
                xhr.upload.onprogress = options.onProgress;
            }

            xhr.onload = function() {
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

            xhr.onerror = function() {
                reject("Timeout");
            };

            if (null === formData) {
                xhr.send();
            } else {
                xhr.send(formData);
            }
        }
    });
};

utils.readJsonFile = function(file) {
    return new Promise(function(resolve, reject) {
        var rawFile = new XMLHttpRequest();

        rawFile.overrideMimeType("application/json");
        rawFile.open("GET", file, true);
        rawFile.onreadystatechange = function() {
            if ((4 == rawFile.readyState) && ("200" == rawFile.status)) {
                resolve(rawFile.responseText);
            }
        }
        rawFile.send(null);
    });
};

utils.checkBMPFile = function(file) {
    return new Promise(function(resolve, reject) {
        var reader = new FileReader();

        reader.onload = function(e) {
            resolve(e.target.result);
        };

        reader.readAsArrayBuffer(file);
    }).then(function(buffer) {
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
