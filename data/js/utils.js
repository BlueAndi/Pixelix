var utils = {};

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
            var xhr = new XMLHttpRequest();
            var formData = null;

            if ("object" === typeof options.parameter) {
                formData = new FormData();

                for( var key in options.parameter ) {
                    formData.append(key, options.parameter[key]);
                }
            }

            xhr.open(options.method, options.url);
            xhr.onload = function() {
                var jsonRsp = JSON.parse(xhr.response);
        
                if (0 === jsonRsp.status) {
                    resolve(jsonRsp);
                } else {
                    reject(jsonRsp);
                }
            };

            if (null === formData) {
                xhr.send();
            } else {
                xhr.send(formData);
            }
        }
    });
};

