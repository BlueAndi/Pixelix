"use strict";

/* Attach the namespaces explicitly to window: in a classic (non-module) script a
 * top-level "const" does NOT create a global property, but every page relies on
 * utils/dialog/pixelix being globally available. */
const utils = (window.utils = window.utils || {});

utils.getURLParameter = function (name) {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(name);
};

utils.enableForm = function (formId, enableIt) {
    const form = document.getElementById(formId);

    for (const element of form.elements) {
        element.disabled = (enableIt === false);
    }
};

utils.obj2FormData = function (obj, formData = new FormData()) {

    const append = (value, key) => {
        if ((typeof value === "string") || (typeof value === "number") || (typeof value === "boolean") || (value instanceof File)) {
            formData.append(key, value);
        } else if ((typeof value === "object") && (value !== null)) {
            build(value, key);
        }
    };

    const build = (data, prefix = "") => {
        const isArray = Array.isArray(data);

        for (const i in data) {
            const key = prefix
                ? (isArray ? `${prefix}._${i}_` : `${prefix}.${i}`)
                : i;

            append(data[i], key);
        }
    };

    build(obj);

    return formData;
};

utils.makeRequest = function (options) {
    return new Promise((resolve, reject) => {
        if (typeof options !== "object") {
            reject({ msg: "Arguments are missing." });
            return;
        }
        if (typeof options.method !== "string") {
            reject({ msg: "Request method is missing." });
            return;
        }
        if (typeof options.url !== "string") {
            reject({ msg: "URL is missing." });
            return;
        }

        /* XMLHttpRequest is kept on purpose: fetch() cannot report upload
         * progress, which the firmware/file upload pages rely on via onProgress. */
        const xhr = new XMLHttpRequest();
        const isJsonResponse = (typeof options.isJsonResponse === "boolean") ? options.isJsonResponse : false;
        let formData = null;
        let urlEncodedPar = "";

        if (typeof options.formData === "object") {
            formData = options.formData;
        } else if (typeof options.parameter === "object") {
            if (options.method.toLowerCase() === "get") {
                const parts = Object.entries(options.parameter).map(
                    ([key, value]) => `${encodeURIComponent(key)}=${encodeURIComponent(value)}`
                );

                urlEncodedPar = "?" + parts.join("&");
            } else {
                formData = utils.obj2FormData(options.parameter);
            }
        }

        xhr.open(options.method, options.url + urlEncodedPar);

        if (typeof options.headers !== "undefined") {
            for (const [key, value] of Object.entries(options.headers)) {
                xhr.setRequestHeader(key, value);
            }
        }

        if (typeof options.onProgress === "function") {
            xhr.upload.onprogress = options.onProgress;
        }

        xhr.onload = () => {
            if (isJsonResponse === true) {
                const jsonRsp = JSON.parse(xhr.response);

                if ((xhr.status === 200) && (jsonRsp.status === "ok")) {
                    resolve(jsonRsp);
                } else {
                    reject(jsonRsp);
                }
            } else if (xhr.status === 200) {
                resolve(xhr.response);
            } else {
                reject(xhr.response);
            }
        };

        xhr.ontimeout = () => {
            console.error(xhr.statusText);
            reject("Timeout");
        };

        xhr.onerror = () => {
            console.error(xhr.statusText);
            reject("Error");
        };

        xhr.send(formData);
    });
};

utils.readJsonFile = async function (file) {
    /* Modernized to fetch(); the previous XHR version compared xhr.status
     * (a number) against the string "200" and therefore never resolved. */
    const response = await fetch(file);

    return response.text();
};

utils.checkBMPFile = async function (file) {
    const buffer = await new Promise((resolve, reject) => {
        const reader = new FileReader();

        reader.onload = (e) => resolve(e.target.result);
        reader.onerror = () => reject("Failed to read file.");
        reader.readAsArrayBuffer(file);
    });

    const bitmapHeaderSize = 54;
    const header = new Uint8Array(buffer, 0, bitmapHeaderSize);
    const planes = (header[27] << 8) | (header[26] << 0);
    const bitsPerPixel = (header[29] << 8) | (header[28] << 0);
    const compression = (header[33] << 24) | (header[32] << 16) | (header[31] << 8) | (header[30] << 0);
    const paletteColors = (header[49] << 24) | (header[48] << 16) | (header[47] << 8) | (header[46] << 0);

    if (String.fromCharCode.apply(null, header.subarray(0, 2)) !== "BM") {
        throw "No bitmap file.";
    } else if (planes !== 1) {
        throw "Only 1 plane is supported.";
    } else if ((bitsPerPixel !== 24) && (bitsPerPixel !== 32)) {
        throw "Only 24 or 32 bpp are supported.";
    } else if (compression !== 0) {
        throw "No compression is supported.";
    } else if (paletteColors !== 0) {
        throw "Color palette not supported.";
    }
};

const dialog = (window.dialog = window.dialog || {});

dialog._getModal = function () {
    return bootstrap.Modal.getOrCreateInstance(document.getElementById("modalDialog"));
};

dialog._createCloseButton = function () {
    const button = document.createElement("button");

    button.type = "button";
    button.className = "btn btn-secondary";
    button.setAttribute("data-bs-dismiss", "modal");
    button.textContent = "Ok";

    return button;
};

dialog._prepare = function (headerClass, footerButtons) {
    const header = document.getElementById("dialogHeader");
    const footer = document.getElementById("dialogFooter");

    header.className = headerClass;
    footer.replaceChildren(...footerButtons);
};

dialog._show = function (title, message, isBlocking) {
    return new Promise((resolve) => {
        const modalElement = document.getElementById("modalDialog");
        const waitOnClick = (isBlocking === true);

        document.getElementById("dialogTitle").textContent = title;
        document.getElementById("dialogBody").innerHTML = message;

        if (waitOnClick === false) {
            modalElement.addEventListener("shown.bs.modal", () => resolve(), { once: true });
        } else {
            /* Blocking: resolve on any secondary button (e.g. both Yes and No). */
            modalElement.querySelectorAll(".btn-secondary").forEach((button) => {
                button.addEventListener("click", () => resolve(), { once: true });
            });
        }

        dialog._getModal().show();
    });
};

dialog.hide = function () {
    return new Promise((resolve) => {
        const modalElement = document.getElementById("modalDialog");

        modalElement.addEventListener("hidden.bs.modal", () => resolve(), { once: true });

        dialog._getModal().hide();
    });
};

dialog.showInfo = function (message, isBlocking) {
    dialog._prepare("modal-header bg-primary text-white", [dialog._createCloseButton()]);

    return dialog._show("Info", message, isBlocking);
};

dialog.showWarning = function (message, isBlocking) {
    dialog._prepare("modal-header bg-warning", [dialog._createCloseButton()]);

    return dialog._show("Warning", message, isBlocking);
};

dialog.showError = function (message, isBlocking) {
    dialog._prepare("modal-header bg-danger text-white", [dialog._createCloseButton()]);

    return dialog._show("Error", message, isBlocking);
};

dialog.show = function (title, message, isBlocking) {
    dialog._prepare("modal-header bg-dark text-white", []);

    return dialog._show(title, message, isBlocking);
};

dialog.showYesNo = function (title, message, onYes, onNo, isBlocking) {
    const yesButton = document.createElement("button");
    const noButton = document.createElement("button");

    yesButton.type = "button";
    yesButton.className = "btn btn-secondary";
    yesButton.setAttribute("data-bs-dismiss", "modal");
    yesButton.textContent = "Yes";
    yesButton.addEventListener("click", () => {
        if (typeof onYes === "function") {
            onYes();
        }
    });

    noButton.type = "button";
    noButton.className = "btn btn-secondary";
    noButton.setAttribute("data-bs-dismiss", "modal");
    noButton.textContent = "No";
    noButton.addEventListener("click", () => {
        if (typeof onNo === "function") {
            onNo();
        }
    });

    dialog._prepare("modal-header bg-warning", [yesButton, noButton]);

    return dialog._show(title, message, isBlocking);
};

dialog.updateMessage = function (message) {
    document.getElementById("dialogBody").innerHTML = message;
};

/* "pixelix" is a shared namespace that ws.js also declares as a top-level name.
 * In classic (non-module) scripts only "var" permits the same top-level
 * identifier across multiple files; const/let would throw on redeclaration when
 * both files are loaded on the same page (e.g. debug.html, display.html). */
var pixelix = window.pixelix || {};

pixelix.rest = pixelix.rest || {};

pixelix.rest.Client = class {
    constructor(options) {
        this._hostname = "";
        this._baseUri = "/rest/api/v1";

        if ((typeof options === "object") && (typeof options.hostname === "string")) {
            this._hostname = options.hostname;
        }
    }

    getBaseUri() {
        return this._baseUri;
    }

    listFiles(path = "/", page = "0") {
        return utils.makeRequest({
            method: "GET",
            url: this._hostname + this._baseUri + "/fs",
            isJsonResponse: true,
            parameter: {
                dir: path,
                page: page
            }
        });
    }

    async listAllFiles(path = "/") {
        const files = [];
        let page = 0;

        for (;;) {
            const rsp = await this.listFiles(path, page);

            if (rsp.data.length === 0) {
                break;
            }

            files.push(...rsp.data);
            ++page;
        }

        return files;
    }

    async listAllFilesRecursive(path = "/") {
        const walk = async (directory) => {
            for (const item of directory.listing) {
                if (item.type === "dir") {
                    item.listing = await this.listAllFiles(item.name);
                    await walk(item);
                }
            }

            return directory;
        };

        const listing = await this.listAllFiles(path);

        return walk({
            name: path,
            size: 0,
            type: "dir",
            listing: listing
        });
    }

    readFile(filename) {
        if (typeof filename !== "string") {
            return Promise.reject();
        }

        return utils.makeRequest({
            method: "GET",
            url: this._hostname + this._baseUri + "/fs/file",
            isJsonResponse: false,
            parameter: {
                path: filename
            }
        });
    }

    writeFile(filename, content, mimeType) {
        if ((typeof filename !== "string") || (typeof mimeType !== "string")) {
            return Promise.reject();
        }

        const formData = new FormData();
        formData.append("file", new Blob([content], { type: mimeType }), filename);

        return utils.makeRequest({
            method: "POST",
            url: this._hostname + this._baseUri + "/fs/file",
            isJsonResponse: true,
            formData: formData
        });
    }

    removeFile(filename) {
        if (typeof filename !== "string") {
            return Promise.reject();
        }

        return utils.makeRequest({
            method: "DELETE",
            url: this._hostname + this._baseUri + "/fs/file",
            isJsonResponse: true,
            parameter: {
                path: filename
            }
        });
    }

    getPluginInstances() {
        return utils.makeRequest({
            method: "GET",
            url: this._baseUri + "/display/slots",
            isJsonResponse: true
        });
    }

    getSensors() {
        return utils.makeRequest({
            method: "GET",
            url: this._baseUri + "/sensors",
            isJsonResponse: true
        });
    }

    getSettingKeys() {
        return utils.makeRequest({
            method: "GET",
            url: this._baseUri + "/settings",
            isJsonResponse: true
        });
    }

    getSettingByKey(key) {
        if (typeof key !== "string") {
            return Promise.reject();
        }

        return utils.makeRequest({
            method: "GET",
            url: this._baseUri + "/setting",
            isJsonResponse: true,
            parameter: {
                key: key
            }
        });
    }

    setSetting(key, value) {
        if ((typeof key !== "string") || (typeof value === "undefined")) {
            return Promise.reject();
        }

        return utils.makeRequest({
            method: "POST",
            url: this._baseUri + "/setting",
            isJsonResponse: true,
            parameter: {
                key: key,
                value: value
            }
        });
    }

    restart() {
        return utils.makeRequest({
            method: "POST",
            url: this._baseUri + "/restart",
            isJsonResponse: true
        });
    }

    fileMgrUploadFile(file, fileSize) {
        if ((typeof file !== "object") || (typeof fileSize !== "number")) {
            return Promise.reject();
        }

        return utils.makeRequest({
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

    fileMgrRemoveFile(fileId) {
        if (typeof fileId !== "number") {
            return Promise.reject();
        }

        return utils.makeRequest({
            method: "POST",
            url: this._hostname + this._baseUri + "/fileMgrService/remove",
            isJsonResponse: true,
            parameter: {
                fileId: fileId
            }
        });
    }

    getDisplayState() {
        return utils.makeRequest({
            method: "GET",
            url: this._baseUri + "/display/power",
            isJsonResponse: true
        });
    }

    setDisplayState(state) {
        if (typeof state !== "string") {
            return Promise.reject();
        }

        return utils.makeRequest({
            method: "POST",
            url: this._baseUri + "/display/power",
            isJsonResponse: true,
            parameter: {
                state: state
            }
        });
    }
};
