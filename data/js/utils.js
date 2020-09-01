var utils = {};

/* Enable/Disable all form elements. */
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
}
