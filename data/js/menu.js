var menu = {}

menu.data = [{
    "title": "Home",
    "hyperRef": "/index.html"
}, {
    "title": "Display",
    "hyperRef": "/display.html"
}, {
    "title": "Plugins",
    "hyperRef": "#",
    "subMenu": []
}, {
    "title": "Settings",
    "hyperRef": "/settings.html"
}, {
    "title": "Update",
    "hyperRef": "/update.html"
}, {
    "title": "Development",
    "hyperRef": "#",
    "subMenu": [{
        "title": "Info",
        "hyperRef": "/info.html"
    }, {
        "title": "Debug",
        "hyperRef": "/debug.html"
    }, {
        "title": "File Editor",
        "hyperRef": "/edit.html"
    }]
}, {
    "title": "About",
    "hyperRef": "/about.html"
}];

menu.captivePortal = [{
    "title": "Home",
    "hyperRef": "/index.html"
}, {
    "title": "About",
    "hyperRef": "/about.html"
}];

menu.addSubMenu = function(menuData, title, subMenu) {
    var index = 0;

    for(index = 0; index < menuData.length; ++index)
    {
        if (menuData[index].title === title) {
            menuData[index].subMenu = menuData[index].subMenu.concat(subMenu);
            break;
        }
    }
}

menu.create = function(ulId, menuData) {
    var index = 0;

    for(index = 0; index < menuData.length; ++index) {
        if ("undefined" === typeof menuData[index].subMenu) {
            menu._createMenuItem(ulId, menuData[index]);
        } else {
            menu._createSubMenu(ulId, menuData[index]);
        }
    }
};

menu._createMenuItem = function(ulId, menuItem) {
    var listItem    = $("<li>").attr("class", "nav-item");
    var anchor      = $("<a>")
                    .attr("class", "nav-link")
                    .attr("href", menuItem.hyperRef)
                    .text(menuItem.title);

    if (location.pathname === menuItem.hyperRef) {
        $(anchor).addClass("active");
    }

    $(listItem).append(anchor)
    $("#" + ulId).append(listItem)
};

menu._createSubMenu = function(ulId, menuItem) {
    var listItem    = $("<li>").attr("class", "nav-item dropdown");
    var anchor      = $("<a>")
                    .attr("class", "nav-link dropdown-toggle")
                    .attr("href", "#")
                    .attr("id", menuItem.title + "-dropdown")
                    .attr("role", "button")
                    .attr("data-toggle", "dropdown")
                    .attr("aria-haspopup", "true")
                    .attr("aria-expanded", "false")
                    .text(menuItem.title);
    var div         = $("<div>")
                    .attr("class", "dropdown-menu scrollable-menu")
                    .attr("aria-labelledby", menuItem.title + "-dropdown");
    var index       = 0;

    for(index = 0; index < menuItem.subMenu.length; ++index) {
        if (true === menu._createSubMenuItem(div, menuItem.subMenu[index])) {
            $(listItem).addClass("active");
        }
    }

    $(listItem).append(anchor);
    $(listItem).append(div);
    $("#" + ulId).append(listItem);
};

menu._createSubMenuItem = function($div, subMenuItem) {
    var anchor      = $("<a>")
                    .attr("class", "dropdown-item")
                    .attr("href", subMenuItem.hyperRef)
                    .text(subMenuItem.title);
    var isActive    = false;

    $($div).append(anchor);

    if (location.pathname === subMenuItem.hyperRef) {
        $(anchor).addClass("active");
        isActive = true;
    }

    return isActive;
};
