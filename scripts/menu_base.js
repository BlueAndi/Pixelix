"use strict";

/* Attach to window explicitly: pages reference "menu" (and the plugin/service
 * arrays appended by the build) as globals, which a top-level const would not
 * provide in a classic script. */
const menu = (window.menu = window.menu || {});

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

menu.addSubMenu = function (menuData, title, subMenu) {
    for (const item of menuData) {
        if (item.title === title) {
            item.subMenu = item.subMenu.concat(subMenu);
            break;
        }
    }
};

menu.create = function (ulId, menuData) {
    for (const menuItem of menuData) {
        if (typeof menuItem.subMenu === "undefined") {
            menu._createMenuItem(ulId, menuItem);
        } else {
            menu._createSubMenu(ulId, menuItem);
        }
    }
};

menu._createMenuItem = function (ulId, menuItem) {
    const listItem = document.createElement("li");
    listItem.className = "nav-item";

    const anchor = document.createElement("a");
    anchor.className = "nav-link";
    anchor.href = menuItem.hyperRef;
    anchor.textContent = menuItem.title;

    if (location.pathname === menuItem.hyperRef) {
        anchor.classList.add("active");
    }

    listItem.appendChild(anchor);
    document.getElementById(ulId).appendChild(listItem);
};

menu._createSubMenu = function (ulId, menuItem) {
    const listItem = document.createElement("li");
    listItem.className = "nav-item dropdown";

    const anchor = document.createElement("a");
    anchor.className = "nav-link dropdown-toggle";
    anchor.href = "#";
    anchor.id = menuItem.title + "-dropdown";
    anchor.setAttribute("role", "button");
    anchor.setAttribute("data-bs-toggle", "dropdown");
    anchor.setAttribute("aria-haspopup", "true");
    anchor.setAttribute("aria-expanded", "false");
    anchor.textContent = menuItem.title;

    const div = document.createElement("div");
    div.className = "dropdown-menu scrollable-menu";
    div.setAttribute("aria-labelledby", menuItem.title + "-dropdown");

    for (const subMenuItem of menuItem.subMenu) {
        if (menu._createSubMenuItem(div, subMenuItem) === true) {
            listItem.classList.add("active");
        }
    }

    listItem.appendChild(anchor);
    listItem.appendChild(div);
    document.getElementById(ulId).appendChild(listItem);
};

menu._createSubMenuItem = function (container, subMenuItem) {
    const anchor = document.createElement("a");
    anchor.className = "dropdown-item";
    anchor.href = subMenuItem.hyperRef;
    anchor.textContent = subMenuItem.title;

    container.appendChild(anchor);

    let isActive = false;

    if (location.pathname === subMenuItem.hyperRef) {
        anchor.classList.add("active");
        isActive = true;
    }

    return isActive;
};
