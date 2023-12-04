var dialog = window.dialog || {};

dialog._show = function(title, message) {
    return new Promise(function(resolve, reject) {

        $("#dialogTitle").text(title);
        $("#dialogBody").html(message);

        $("#modalDialog").on("shown.bs.modal", function() {
            $("#modalDialog").off("shown.bs.modal");
            resolve();
        });

        $("#modalDialog").modal("show");
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

dialog.showInfo = function(message) {
    var $btnClose = $("<button>")
                    .attr("type", "button")
                    .attr("class", "btn btn-secondary")
                    .attr("data-dismiss", "modal")
                    .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-primary text-white");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Info", message);
}

dialog.showWarning = function(message) {
    var $btnClose = $("<button>")
                    .attr("type", "button")
                    .attr("class", "btn btn-secondary")
                    .attr("data-dismiss", "modal")
                    .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-warning text-dark");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Warning", message);
}

dialog.showError = function(message) {
    var $btnClose = $("<button>")
                    .attr("type", "button")
                    .attr("class", "btn btn-secondary")
                    .attr("data-dismiss", "modal")
                    .text("Ok")

    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-danger text-white");
    $("#dialogFooter").append($btnClose);

    return dialog._show("Error", message);
}

dialog.show = function(title, message) {
    $("#dialogHeader").removeClass();
    $("#dialogFooter").empty();

    $("#dialogHeader").addClass("modal-header bg-dark text-white");

    return dialog._show(title, message);
}
