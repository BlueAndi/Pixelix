"use strict";

/* Auto-generated shared menu bundle. */

"use strict";

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
    "title": "Icons",
    "hyperRef": "/icons.html"
}, {
    "title": "Services",
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
                    .attr("data-bs-toggle", "dropdown")
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


"use strict";

var pluginSubMenu = [
    {
        title: "ChicagoBusTrackerPlugin",
        hyperRef: "/plugins/ChicagoBusTrackerPlugin/ChicagoBusTrackerPlugin.html"
    },
    {
        title: "CountdownPlugin",
        hyperRef: "/plugins/CountdownPlugin/CountdownPlugin.html"
    },
    {
        title: "DateTimePlugin",
        hyperRef: "/plugins/DateTimePlugin/DateTimePlugin.html"
    },
    {
        title: "DDPPlugin",
        hyperRef: "/plugins/DDPPlugin/DDPPlugin.html"
    },
    {
        title: "FirePlugin",
        hyperRef: "/plugins/FirePlugin/FirePlugin.html"
    },
    {
        title: "GameOfLifePlugin",
        hyperRef: "/plugins/GameOfLifePlugin/GameOfLifePlugin.html"
    },
    {
        title: "GrabViaMqttPlugin",
        hyperRef: "/plugins/GrabViaMqttPlugin/GrabViaMqttPlugin.html"
    },
    {
        title: "GrabViaRestPlugin",
        hyperRef: "/plugins/GrabViaRestPlugin/GrabViaRestPlugin.html"
    },
    {
        title: "GruenbeckPlugin",
        hyperRef: "/plugins/GruenbeckPlugin/GruenbeckPlugin.html"
    },
    {
        title: "IconTextLampPlugin",
        hyperRef: "/plugins/IconTextLampPlugin/IconTextLampPlugin.html"
    },
    {
        title: "IconTextPlugin",
        hyperRef: "/plugins/IconTextPlugin/IconTextPlugin.html"
    },
    {
        title: "MatrixPlugin",
        hyperRef: "/plugins/MatrixPlugin/MatrixPlugin.html"
    },
    {
        title: "MultiIconPlugin",
        hyperRef: "/plugins/MultiIconPlugin/MultiIconPlugin.html"
    },
    {
        title: "OpenMeteoPlugin",
        hyperRef: "/plugins/OpenMeteoPlugin/OpenMeteoPlugin.html"
    },
    {
        title: "OpenWeatherPlugin",
        hyperRef: "/plugins/OpenWeatherPlugin/OpenWeatherPlugin.html"
    },
    {
        title: "RainbowPlugin",
        hyperRef: "/plugins/RainbowPlugin/RainbowPlugin.html"
    },
    {
        title: "SensorPlugin",
        hyperRef: "/plugins/SensorPlugin/SensorPlugin.html"
    },
    {
        title: "SignalDetectorPlugin",
        hyperRef: "/plugins/SignalDetectorPlugin/SignalDetectorPlugin.html"
    },
    {
        title: "SoundReactivePlugin",
        hyperRef: "/plugins/SoundReactivePlugin/SoundReactivePlugin.html"
    },
    {
        title: "SunrisePlugin",
        hyperRef: "/plugins/SunrisePlugin/SunrisePlugin.html"
    },
    {
        title: "SysMsgPlugin",
        hyperRef: "/plugins/SysMsgPlugin/SysMsgPlugin.html"
    },
    {
        title: "TempHumidPlugin",
        hyperRef: "/plugins/TempHumidPlugin/TempHumidPlugin.html"
    },
    {
        title: "VolumioPlugin",
        hyperRef: "/plugins/VolumioPlugin/VolumioPlugin.html"
    },
    {
        title: "WifiStatusPlugin",
        hyperRef: "/plugins/WifiStatusPlugin/WifiStatusPlugin.html"
    },
    {
        title: "WormPlugin",
        hyperRef: "/plugins/WormPlugin/WormPlugin.html"
    }
];


"use strict";

var serviceSubMenu = [
    {
        title: "MqttService",
        hyperRef: "/services/MqttService/MqttService.html"
    },
    {
        title: "TimerService",
        hyperRef: "/services/TimerService/TimerService.html"
    }
];
