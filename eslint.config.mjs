import js from "@eslint/js";
import html from "eslint-plugin-html";
import globals from "globals";

/* Globals provided by the shared frontend scripts and the third party libraries, which a page includes. */
const pageGlobals = {
    ace: "readonly",
    bootstrap: "readonly",
    dialog: "readonly",
    JSZip: "readonly",
    menu: "readonly",
    pixelix: "readonly",
    pluginSubMenu: "readonly",
    saveAs: "readonly",
    serviceSubMenu: "readonly",
    tagEditor: "readonly",
    utils: "readonly"
};

export default [
    {
        /* Generated or third party sources are not linted. */
        ignores: [
            "**/node_modules/**",
            ".pio/**",
            "data/plugins/**",
            "data/services/**",
            "data/js/app-menu.js",
            "scripts/pluginsSubMenu.js",
            "scripts/servicesSubMenu.js",
            "usb-installer/**",
            "doc/**"
        ]
    },
    js.configs.recommended,
    {
        rules: {
            /* Variables are initialized at their declaration by convention. */
            "no-useless-assignment": "off"
        }
    },
    {
        /* Web pages with inline scripts. */
        files: ["data/*.html", "data/cp/*.html", "lib/*/web/*.html"],
        plugins: { html },
        languageOptions: {
            sourceType: "script",
            globals: { ...globals.browser, ...pageGlobals }
        }
    },
    {
        /* Shared frontend scripts, which run in the browser. */
        files: ["data/js/*.js", "scripts/app-tag-editor.js", "scripts/menu_base.js"],
        languageOptions: {
            sourceType: "script",
            globals: { ...globals.browser, bootstrap: "readonly", module: "readonly" }
        },
        rules: {
            /* Top level definitions are the interface to the pages. */
            "no-unused-vars": ["error", { vars: "local" }]
        }
    },
    {
        /* The menu base is concatenated with the generated sub-menus. */
        files: ["scripts/menu_base.js"],
        languageOptions: {
            globals: { pluginSubMenu: "readonly", serviceSubMenu: "readonly" }
        }
    },
    {
        /* Node.js based tests. */
        files: ["scripts/test_*.js"],
        languageOptions: {
            sourceType: "commonjs",
            globals: { ...globals.node }
        }
    }
];
