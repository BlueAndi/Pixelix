var pixelix = window.pixelix || {};

pixelix.rest = {};

pixelix.rest.Client = function(options) {
    this._hostname  = "";
    this._baseUri   = "/rest/api/v1";

    if ("object" === typeof options) {
        if ("string" === typeof options.hostname) {
            this._hostname = options.hostname;
        }
    }
};

pixelix.rest.Client.prototype.listFiles = function(path = "/", page = "0") {
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

pixelix.rest.Client.prototype.readFile = function(filename) {
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

pixelix.rest.Client.prototype.writeFile = function(filename, content) {
    var promise     = null;
    var formData    = null;

    if ("string" !== typeof filename) {
        promise = Promise.reject();
    } else if ("string" !== typeof content) {
        promise = Promise.reject();
    } else {
        formData = new FormData();
        formData.append("file", new Blob([content], { type: "text/plain" }), filename);

        promise = utils.makeRequest({
            method: "POST",
            url: this._hostname + this._baseUri + "/fs/file",
            isJsonResponse: false,
            formData: formData
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.removeFile = function(filename) {
    var promise = null;
    if ("string" !== typeof filename) {
        promise = Promise.reject();
    } else {
        promise = utils.makeRequest({
            method: "DELETE",
            url: this._hostname + this._baseUri + "/fs/file",
            isJsonResponse: false,
            parameter: {
                path: filename
            }
        });
    }

    return promise;
};

pixelix.rest.Client.prototype.getPluginInstances = function() {
    return utils.makeRequest({
        method: "GET",
        url: "/rest/api/v1/display/slots",
        isJsonResponse: true
    });
};
