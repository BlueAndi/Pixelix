var dialog = window.dialog || {};

dialog._show = function(title, message, isBlocking) {
    return new Promise(function(resolve, reject) {

        var waitOnClick = false;

        if (("boolean" === typeof isBlocking) &&
            (true == isBlocking)) {
            waitOnClick = true;
        }

        $("#dialogTitle").text(title);
        $("#dialogBody").html(message);

        $("#modalDialog").on("shown.bs.modal", function() {
            $("#modalDialog").off("shown.bs.modal");

            if (false === waitOnClick) {
                resolve();
            }
        });

        $("#modalDialog").modal("show");

        if (true === waitOnClick) {
            $("#modalDialog .btn-secondary").click(function() {
                resolve();
            });
        }
    });
}

dialog.hide = function() {
    return new Promise(function(resolve, reject) {

        $("#modalDialog").on("hidden.bs.modal", function() {
            $("#modalDialog").off("hidden.bs.modal");
            resolve();
        });

        $("#modalDialog").modal("hide");
    });
}

dialog.showInfo = function(message, isBlocking) {
    var $btnClose = $("<button>")
                    .attr("type", "button")
                    .attr("class", "btn btn-secondary")
                    .attr("data-dismiss", "modal")
                    .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-primary text-white");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Info", message, isBlocking);
}

dialog.showWarning = function(message, isBlocking) {
    var $btnClose = $("<button>")
                    .attr("type", "button")
                    .attr("class", "btn btn-secondary")
                    .attr("data-dismiss", "modal")
                    .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-warning text-dark");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Warning", message, isBlocking);
}

dialog.showError = function(message, isBlocking) {
    var $btnClose = $("<button>")
                    .attr("type", "button")
                    .attr("class", "btn btn-secondary")
                    .attr("data-dismiss", "modal")
                    .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-danger text-white");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Error", message, isBlocking);
}

dialog.show = function(title, message, isBlocking) {
    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-dark text-white");

    return dialog._show(title, message, isBlocking);
}
