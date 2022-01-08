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
    "subMenu": [{
        "title": "BTC Quote Plugin",
        "hyperRef": "/plugins/BTCQuotePlugin.html"
    }, {
        "title": "Countdown Plugin",
        "hyperRef": "/plugins/CountdownPlugin.html"
    }, {
        "title": "Date Plugin",
        "hyperRef": "/plugins/DatePlugin.html"
    }, {
        "title": "DateTime Plugin",
        "hyperRef": "/plugins/DateTimePlugin.html"
    }, {
        "title": "Fire Plugin",
        "hyperRef": "/plugins/FirePlugin.html"
    }, {
        "title": "GameOfLife Plugin",
        "hyperRef": "/plugins/GameOfLifePlugin.html"
    }, {
        "title": "Github Plugin",
        "hyperRef": "/plugins/GithubPlugin.html"
    }, {
        "title": "Gruenbeck Plugin",
        "hyperRef": "/plugins/GruenbeckPlugin.html"
    }, {
        "title": "IconTextLamp Plugin",
        "hyperRef": "/plugins/IconTextLampPlugi.html"
    }, {
        "title": "IconText Plugin",
        "hyperRef": "/plugins/IconTextPlugin.html"
    }, {
        "title": "JustText Plugin",
        "hyperRef": "/plugins/JustTextPlugin.html"
    }, {
        "title": "Matrix Plugin",
        "hyperRef": "/plugins/MatrixPlugin.html"
    }, {
        "title": "OpenWeather Plugin",
        "hyperRef": "/plugins/OpenWeatherPlugin.html"
    }, {
        "title": "Rainbow Plugin",
        "hyperRef": "/plugins/RainbowPlugin.html"
    }, {
        "title": "Sensor Plugin",
        "hyperRef": "/plugins/SensorPlugin.html"
    }, {
        "title": "ShellyPlugS Plugin",
        "hyperRef": "/plugins/ShellyPlugSPlugin.html"
    }, {
        "title": "Sunrise Plugin",
        "hyperRef": "/plugins/SunrisePlugin.html"
    }, {
        "title": "SysMsg Plugin",
        "hyperRef": "/plugins/SysMsgPlugin.html"
    }, {
        "title": "TempHumid Plugin",
        "hyperRef": "/plugins/TempHumidPlugin.html"
    }, {
        "title": "Test Plugin",
        "hyperRef": "/plugins/TestPlugin.html"
    }, {
        "title": "Time Plugin",
        "hyperRef": "/plugins/TimePlugin.html"
    }, {
        "title": "Volumio Plugin",
        "hyperRef": "/plugins/VolumioPlugin.html"
    }, {
        "title": "WifiStatus Plugin",
        "hyperRef": "/plugins/WifiStatusPlugin.html"
    }]
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

menu.create = function(ulId) {
    var index       = 0;
    var listItem    = null;
    var anchor      = null;

    for(index = 0; index < menu.data.length; ++index) {
        if ("undefined" === typeof menu.data[index].subMenu) {
            menu._createMenuItem(ulId, menu.data[index]);
        } else {
            menu._createSubMenu(ulId, menu.data[index]);
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
                    .attr("class", "dropdown-menu")
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
