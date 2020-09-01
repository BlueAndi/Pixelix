var menu = [{
    title: "Home",
    link: "/"
}, {
    title: "Development",
    link: "/dev.html"
}, {
    title: "Display",
    link: "/display.html"
}, {
    title: "Filesystem",
    link: "/edit"
}, {
    title: "Network",
    link: "/network.html"
}, {
    title: "Plugins",
    link: "/plugins.html"
}, {
    title: "Settings",
    link: "/settings.html"
}, {
    title: "Update",
    link: "/update.html"
}];

function injectMenu(id, activeLink) {
    var element = document.getElementById(id);
    var index   = 0;
    var aHtml   = null;

    for(index = 0; index < menu.length; ++index) {
        aHtml = document.createElement("a");

        aHtml.setAttribute("href", menu[index].link);
        aHtml.innerHTML = menu[index].title;

        if (activeLink == menu[index].link) {
            aHtml.setAttribute("class", "active");
        }

        element.appendChild(aHtml);
    }
}
