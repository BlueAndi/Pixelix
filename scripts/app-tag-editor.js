"use strict";

/* Tag editor for text inputs, which contain text widget format tags.
 *
 * This file is the source. The filesystem contains data/js/app-tag-editor.js.gz,
 * which must be regenerated after every change, see doc/WEB-CONVENTIONS.md.
 *
 * Attach to window explicitly, see app-api.js. Without window (Node.js) the
 * pure functions are exported for testing purposes.
 */
const tagEditor = ("object" === typeof window) ? (window.tagEditor = window.tagEditor || {}) : {};

/* Shown in the preview instead of a special character code, because the glyph
 * depends on the font used by the firmware.
 */
tagEditor.SPECIAL_CHAR_PLACEHOLDER = "□";

/* Firmware defaults, see TextWidget.h */
tagEditor._DEFAULT_COLOR = "#FFFFFF";
tagEditor._DEFAULT_GRADIENT = { type: "gradient", direction: "vertical", startColor: "#FF0000", endColor: "#0000FF" };

tagEditor._H_ALIGN = { "{hl}": "left", "{hc}": "center", "{hr}": "right" };
tagEditor._V_ALIGN = { "{vt}": "top", "{vc}": "center", "{vb}": "bottom" };

/* Convert hex digits to "#RRGGBB" or return null if invalid. */
tagEditor._toColor = function (hexStr) {
    let color = null;

    if (true === /^[0-9a-f]{1,8}$/i.test(hexStr)) {
        color = "#" + (Number.parseInt(hexStr, 16) & 0xFFFFFF).toString(16).padStart(6, "0").toUpperCase();
    }

    return color;
};

/* Check gradient offset and length against the range the firmware accepts. */
tagEditor.isGradientRangeValid = function (offset, length) {
    const INT16_MIN = -32768;
    const INT16_MAX = 32767;
    const UINT16_MAX = 65535;

    return (true === Number.isInteger(offset)) && (true === Number.isInteger(length)) &&
        (INT16_MIN <= offset) && (INT16_MAX >= offset) &&
        (0 <= length) && (UINT16_MAX >= length);
};

/* Split the text into text and tag tokens. Its a port of TWTokenizer::parse() of the
 * firmware, including its specialities:
 * - Text which ends with an escaped character is not closed by a following tag or
 *   line feed, it continues behind.
 * - Text which ends with a single escape character is dropped.
 */
tagEditor._tokenize = function (text) {
    const tokens = [];
    let isValid = true;
    let isEscapeActive = false;
    let isInTag = false;
    let beginIdx = 0;
    let result = "";
    let idx = 0;

    while ((true === isValid) && (text.length > idx)) {
        const character = text[idx];

        if (true === isEscapeActive) {
            /* The escape character itself is not part of the text. */
            result += text.substring(beginIdx, idx - 1) + character;
            beginIdx = idx + 1;
            isEscapeActive = false;
        } else if ("\\" === character) {
            if (true === isInTag) {
                isValid = false;
            } else {
                isEscapeActive = true;
            }
        } else if ("\n" === character) {
            if (true === isInTag) {
                isValid = false;
            } else {
                tokens.push({ isTag: false, str: "\n" });
                beginIdx = idx + 1;
            }
        } else if ("{" === character) {
            if (true === isInTag) {
                isValid = false;
            } else {
                beginIdx = idx;
                isInTag = true;
            }
        } else if ("}" === character) {
            if (false === isInTag) {
                isValid = false;
            } else {
                tokens.push({ isTag: true, str: text.substring(beginIdx, idx + 1) });
                beginIdx = idx + 1;
                isInTag = false;
            }
        } else if (false === isInTag) {
            const next = text[idx + 1];

            /* End of text reached? */
            if (("{" === next) || ("}" === next) || ("\n" === next) || (undefined === next)) {
                result += text.substring(beginIdx, idx + 1);
                tokens.push({ isTag: false, str: result });
                beginIdx = idx + 1;
                result = "";
            }
        }

        if (true === isValid) {
            ++idx;
        }
    }

    /* Best effort in case of an error: Keep the rest as it is. */
    if (false === isValid) {
        result += text.substring(idx);
    }

    if (0 < result.length) {
        tokens.push({ isTag: false, str: result });
    }

    return { isValid: isValid, tokens: tokens };
};

/* Parse text with format tags.
 *
 * Returns { isValid, hAlign, vAlign, runs }, with runs as array of { text, brush }.
 * A brush is { type: "solid", color } or { type: "gradient", direction, startColor, endColor }.
 * isValid is false if the firmware would reject the text.
 */
tagEditor.parse = function (text) {
    const tokenized = tagEditor._tokenize(text);
    const result = { isValid: tokenized.isValid, hAlign: "left", vAlign: "top", runs: [] };
    let solidBrush = { type: "solid", color: tagEditor._DEFAULT_COLOR };
    let gradientBrush = tagEditor._DEFAULT_GRADIENT;
    let brush = solidBrush;
    let pendingHAlign = null;

    const addText = function (str) {
        const last = result.runs[result.runs.length - 1];

        /* The firmware applies a horizontal alignment not before text is drawn. */
        if (null !== pendingHAlign) {
            result.hAlign = pendingHAlign;
            pendingHAlign = null;
        }

        if ((undefined !== last) && (JSON.stringify(last.brush) === JSON.stringify(brush))) {
            last.text += str;
        } else {
            result.runs.push({ text: str, brush: Object.assign({}, brush) });
        }
    };

    const handleGradient = function (tag, direction) {
        const match = /^#([0-9a-f]{1,8}),#([0-9a-f]{1,8}),(-?\d+),(\d+)$/i.exec(tag.substring(5, tag.length - 1));

        if ((null !== match) && (true === tagEditor.isGradientRangeValid(Number.parseInt(match[3], 10), Number.parseInt(match[4], 10)))) {
            gradientBrush = { type: "gradient", direction: direction, startColor: tagEditor._toColor(match[1]), endColor: tagEditor._toColor(match[2]) };
            brush = gradientBrush;
        }
    };

    tokenized.tokens.forEach(function (token) {
        const tag = token.str;

        if (false === token.isTag) {
            addText(tag);
        } else if (undefined !== tagEditor._H_ALIGN[tag]) {
            pendingHAlign = tagEditor._H_ALIGN[tag];
        } else if (undefined !== tagEditor._V_ALIGN[tag]) {
            result.vAlign = tagEditor._V_ALIGN[tag];
        } else if ("{stc}" === tag) {
            brush = solidBrush;
        } else if ("{lgtc}" === tag) {
            brush = gradientBrush;
        } else if (true === tag.startsWith("{lgv ")) {
            handleGradient(tag, "vertical");
        } else if (true === tag.startsWith("{lgh ")) {
            handleGradient(tag, "horizontal");
        } else if (true === tag.startsWith("{#")) {
            const color = tagEditor._toColor(tag.substring(2, tag.length - 1));

            if (null !== color) {
                solidBrush = { type: "solid", color: color };
                brush = solidBrush;
            }
        } else if (true === /^\{0x[0-9a-fA-F]+\}$/.test(tag)) {
            /* The firmware converts it to a text, if its a valid character code. */
            if (0xFF >= Number.parseInt(tag.substring(3, tag.length - 1), 16)) {
                addText(tagEditor.SPECIAL_CHAR_PLACEHOLDER);
            }
        } else {
            /* Unknown tags are skipped. */
        }
    });

    return result;
};

/* Split the text into tags and the text between, each with its position.
 *
 * Returns an array of { start, end, str, isTag }, which covers the whole text.
 * Complete format tags and the given plugin specific placeholders are tags,
 * everything else is text, including escaped and incomplete tags.
 */
tagEditor.split = function (value, placeholders) {
    const segments = [];
    let idx = 0;

    while (value.length > idx) {
        const last = segments[segments.length - 1];
        let end = idx + 1;
        let isTag = false;

        if ("\\" === value[idx]) {
            /* The escaped character belongs to the text. */
            end = Math.min(idx + 2, value.length);
        } else if ("{" === value[idx]) {
            const match = /^\{[^{}\\\n]*\}/.exec(value.substring(idx));

            if (null !== match) {
                end = idx + match[0].length;
                isTag = true;
            }
        } else {
            (placeholders || []).forEach(function (placeholder) {
                if ((false === isTag) && (0 < placeholder.length) && (true === value.startsWith(placeholder, idx))) {
                    end = idx + placeholder.length;
                    isTag = true;
                }
            });
        }

        if ((false === isTag) && (undefined !== last) && (false === last.isTag)) {
            last.end = end;
            last.str = value.substring(last.start, end);
        } else {
            segments.push({ start: idx, end: end, str: value.substring(idx, end), isTag: isTag });
        }

        idx = end;
    }

    return segments;
};

/* Insert a tag at the selection start. Selected text is kept.
 *
 * Returns { value, cursor } or null if the max. length (optional) would be exceeded.
 */
tagEditor.insertAt = function (value, selectionStart, selectionEnd, tag, maxLength) {
    let result = null;

    if (("number" !== typeof maxLength) || (maxLength >= (value.length + tag.length))) {
        result = {
            value: value.substring(0, selectionStart) + tag + value.substring(selectionStart),
            cursor: selectionStart + tag.length
        };
    }

    return result;
};

/* Date/time placeholders according to strftime(), usable as items of a plugin specific tab. */
tagEditor.STRFTIME_ITEMS = [
    { insert: "%H", label: "Hour (24h)", description: "%H - Hour in 24h format (00-23)" },
    { insert: "%I", label: "Hour (12h)", description: "%I - Hour in 12h format (01-12)" },
    { insert: "%M", label: "Minute", description: "%M - Minute (00-59)" },
    { insert: "%S", label: "Second", description: "%S - Second (00-59)" },
    { insert: "%p", label: "AM/PM", description: "%p - AM or PM designation" },
    { insert: "%d", label: "Day", description: "%d - Day of the month, zero-padded (01-31)" },
    { insert: "%m", label: "Month", description: "%m - Month as a decimal number (01-12)" },
    { insert: "%Y", label: "Year", description: "%Y - Year" },
    { insert: "%y", label: "Year (2 digits)", description: "%y - Year, last two digits (00-99)" },
    { insert: "%b", label: "Month name (short)", description: "%b - Abbreviated month name (3 characters)" },
    { insert: "%B", label: "Month name", description: "%B - Full month name" },
    { insert: "%a", label: "Weekday (short)", description: "%a - Abbreviated weekday name (3 characters)" },
    { insert: "%A", label: "Weekday", description: "%A - Full weekday name" },
    { insert: "%%", label: "% sign", description: "%% - A % sign" }
];

tagEditor._MONTH_NAMES = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
tagEditor._WEEKDAY_NAMES = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];

/* Replace the offered strftime() placeholders in the text with the given local date/time,
 * for the preview. Other placeholders are kept as they are.
 */
tagEditor.strftime = function (text, date) {
    const pad = function (value) {
        return String(value).padStart(2, "0");
    };
    const values = {
        "H": pad(date.getHours()),
        "I": pad(((date.getHours() + 11) % 12) + 1),
        "M": pad(date.getMinutes()),
        "S": pad(date.getSeconds()),
        "p": (12 > date.getHours()) ? "AM" : "PM",
        "d": pad(date.getDate()),
        "m": pad(date.getMonth() + 1),
        "Y": String(date.getFullYear()),
        "y": pad(date.getFullYear() % 100),
        "b": tagEditor._MONTH_NAMES[date.getMonth()].substring(0, 3),
        "B": tagEditor._MONTH_NAMES[date.getMonth()],
        "a": tagEditor._WEEKDAY_NAMES[date.getDay()].substring(0, 3),
        "A": tagEditor._WEEKDAY_NAMES[date.getDay()],
        "%": "%"
    };

    return text.replace(/%(.)/g, function (match, specifier) {
        return (true === Object.prototype.hasOwnProperty.call(values, specifier)) ? values[specifier] : match;
    });
};

/* Value placeholders according to printf(), usable as items of a plugin specific tab. */
tagEditor.PRINTF_ITEMS = [
    { insert: "%s", label: "Text", description: "%s - The value as text" },
    { insert: "%.0f", label: "Number", description: "%.0f - The value as number without decimals" },
    { insert: "%.1f", label: "Number (1 decimal)", description: "%.1f - The value as number with one decimal" },
    { insert: "%.2f", label: "Number (2 decimals)", description: "%.2f - The value as number with two decimals" },
    { insert: "%%", label: "% sign", description: "%% - A % sign" }
];

/* Replace the printf() specifiers %f and %s (with optional flags, width and precision)
 * and %% in the text with the given value, for the preview. Other specifiers are kept
 * as they are.
 */
tagEditor.printf = function (text, value) {
    return text.replace(/%([-+ #0]*)(\d+)?(?:\.(\d+))?([fs%])/g, function (match, flags, width, precision, specifier) {
        let result = "%";

        if ("f" === specifier) {
            const number = Number(value);

            result = Math.abs(number).toFixed((undefined === precision) ? 6 : Number.parseInt(precision, 10));

            if (0 > number) {
                result = "-" + result;
            } else if (true === flags.includes("+")) {
                result = "+" + result;
            } else if (true === flags.includes(" ")) {
                result = " " + result;
            }
        } else if ("s" === specifier) {
            result = String(value);

            if (undefined !== precision) {
                result = result.substring(0, Number.parseInt(precision, 10));
            }
        }

        if (("%" !== specifier) && (undefined !== width)) {
            const length = Number.parseInt(width, 10);

            if (true === flags.includes("-")) {
                result = result.padEnd(length, " ");
            } else if ((true === flags.includes("0")) && ("f" === specifier)) {
                const sign = /^[-+ ]/.test(result) ? result[0] : "";

                result = sign + result.substring(sign.length).padStart(length - sign.length, "0");
            } else {
                result = result.padStart(length, " ");
            }
        }

        return result;
    });
};

tagEditor._HINT_INVALID = "Invalid tag syntax, the display will not update.";
tagEditor._BTN_CLASS = "btn btn-sm btn-secondary";
tagEditor._ROW_CLASS = "d-flex flex-wrap gap-2 align-items-center";

/* There is only one editor per page, which moves to the focused input. */
tagEditor._options = new WeakMap();
tagEditor._root = null;
tagEditor._input = null;
tagEditor._preview = null;
tagEditor._hint = null;
tagEditor._tags = null;
tagEditor._warning = "";
tagEditor._timer = null;
tagEditor._isPointerFocus = false;
tagEditor._isInitialized = false;

/* Create a HTML element with CSS classes, optional text and attributes. */
tagEditor._create = function (tagName, className, text, attributes) {
    const element = document.createElement(tagName);

    element.className = className;

    if (undefined !== text) {
        element.textContent = text;
    }

    Object.keys(attributes || {}).forEach(function (name) {
        element.setAttribute(name, attributes[name]);
    });

    return element;
};

/* Create a button, which calls the handler on click. */
tagEditor._createButton = function (text, onClick, title) {
    const button = tagEditor._create("button", tagEditor._BTN_CLASS, text, { type: "button" });

    if (undefined !== title) {
        button.title = title;
    }

    button.addEventListener("click", onClick);

    return button;
};

/* Create a button, which inserts the tag on click. */
tagEditor._createTagButton = function (text, tag, title) {
    return tagEditor._createButton(text, function () {
        tagEditor._insert(tag);
    }, title);
};

tagEditor._createColorInput = function (value, label) {
    return tagEditor._create("input", "form-control form-control-sm form-control-color", undefined, { "type": "color", "value": value, "aria-label": label });
};

tagEditor._createColorTab = function () {
    const pane = tagEditor._create("div", tagEditor._ROW_CLASS);
    const color = tagEditor._createColorInput("#ff0000", "Text color");

    pane.append(color, tagEditor._createButton("Add color", function () {
        tagEditor._insert("{" + color.value.toUpperCase() + "}");
    }));

    return pane;
};

tagEditor._createGradientTab = function () {
    const pane = tagEditor._create("div", tagEditor._ROW_CLASS);
    const startColor = tagEditor._createColorInput("#ff0000", "Gradient start color");
    const endColor = tagEditor._createColorInput("#0000ff", "Gradient end color");
    const direction = tagEditor._create("select", "form-select form-select-sm w-auto", undefined, { "aria-label": "Gradient direction" });
    const offset = tagEditor._create("input", "form-control form-control-sm tag-editor-number", undefined, { "type": "number", "value": "0", "aria-label": "Gradient offset in pixels", "title": "Offset in pixels (-32768 to 32767)" });
    const length = tagEditor._create("input", "form-control form-control-sm tag-editor-number", undefined, { "type": "number", "value": "32", "aria-label": "Gradient length in pixels", "title": "Length in pixels (0 to 65535)" });

    direction.append(tagEditor._create("option", "", "Vertical", { value: "lgv" }), tagEditor._create("option", "", "Horizontal", { value: "lgh" }));

    pane.append(startColor, endColor, direction, offset, length, tagEditor._createButton("Add gradient", function () {
        const toNumber = function (str) {
            return ("" === str.trim()) ? Number.NaN : Number(str);
        };
        const offsetValue = toNumber(offset.value);
        const lengthValue = toNumber(length.value);

        /* Mark the invalid one(s), by checking each with a valid counterpart. */
        offset.classList.toggle("is-invalid", false === tagEditor.isGradientRangeValid(offsetValue, 0));
        length.classList.toggle("is-invalid", false === tagEditor.isGradientRangeValid(0, lengthValue));

        if (true === tagEditor.isGradientRangeValid(offsetValue, lengthValue)) {
            tagEditor._insert("{" + direction.value + " " + startColor.value.toUpperCase() + "," + endColor.value.toUpperCase() + "," + offsetValue + "," + lengthValue + "}");
        }
    }), tagEditor._createTagButton("Use solid color", "{stc}", "Activate solid text color"), tagEditor._createTagButton("Use gradient", "{lgtc}", "Activate linear gradient text color"));

    return pane;
};

tagEditor._createAlignmentTab = function () {
    const pane = tagEditor._create("div", tagEditor._ROW_CLASS);

    [["Left", "{hl}"], ["Center", "{hc}"], ["Right", "{hr}"], ["Top", "{vt}"], ["Middle", "{vc}"], ["Bottom", "{vb}"]].forEach(function (item) {
        pane.append(tagEditor._createTagButton(item[0], item[1]));
    });

    return pane;
};

tagEditor._createCharacterTab = function () {
    const pane = tagEditor._create("div", tagEditor._ROW_CLASS);
    const code = tagEditor._create("input", "form-control form-control-sm tag-editor-number", undefined, { "type": "text", "maxlength": "2", "placeholder": "CC", "aria-label": "Character code in hex (00-FF)", "title": "Character code in hex (00-FF)" });

    pane.append(code, tagEditor._createButton("Add character", function () {
        const isValid = /^[0-9a-f]{1,2}$/i.test(code.value);

        code.classList.toggle("is-invalid", false === isValid);

        if (true === isValid) {
            tagEditor._insert("{0x" + code.value.toUpperCase().padStart(2, "0") + "}");
        }
    }));

    return pane;
};

/* Create a tab with plugin specific tags, one button per item. */
tagEditor._createItemsTab = function (tab) {
    const pane = tagEditor._create("div", tagEditor._ROW_CLASS);

    (tab.items || []).forEach(function (item) {
        pane.append(tagEditor._createTagButton(item.label, item.insert, item.description));
    });

    return pane;
};

/* Create a tab with presets, which replace the whole text. */
tagEditor._createPresetsTab = function (presets) {
    const pane = tagEditor._create("div", tagEditor._ROW_CLASS);

    presets.forEach(function (preset) {
        pane.append(tagEditor._createButton(preset.label, function () {
            tagEditor._replaceRange(0, tagEditor._input.value.length, preset.value);
        }, preset.value));
    });

    return pane;
};

/* Build the editor for the given options. */
tagEditor._build = function (options) {
    const root = tagEditor._create("div", "tag-editor border rounded bg-body shadow p-2");
    const nav = tagEditor._create("ul", "nav nav-tabs small mb-2");
    const tabs = [
        { title: "Color", pane: tagEditor._createColorTab() },
        { title: "Gradient", pane: tagEditor._createGradientTab() },
        { title: "Alignment", pane: tagEditor._createAlignmentTab() },
        { title: "Character", pane: tagEditor._createCharacterTab() }
    ];

    (options.extraTabs || []).forEach(function (tab) {
        tabs.push({ title: tab.title, pane: tagEditor._createItemsTab(tab) });
    });

    if (0 < (options.presets || []).length) {
        tabs.push({ title: "Presets", pane: tagEditor._createPresetsTab(options.presets) });
    }

    tagEditor._preview = tagEditor._create("div", "tag-editor-preview");
    tagEditor._tags = tagEditor._create("div", "tag-editor-tags");
    tagEditor._hint = tagEditor._create("div", "tag-editor-hint small text-warning");

    root.append(tagEditor._preview, tagEditor._hint, nav);

    tabs.forEach(function (tab, tabIdx) {
        const item = tagEditor._create("li", "nav-item");
        const link = tagEditor._create("button", "nav-link py-1 px-2", tab.title, { type: "button" });

        link.addEventListener("click", function () {
            tabs.forEach(function (other) {
                other.link.classList.toggle("active", other === tab);
                other.pane.hidden = (other !== tab);
            });
        });

        tab.link = link;
        tab.pane.hidden = (0 !== tabIdx);
        link.classList.toggle("active", 0 === tabIdx);

        item.append(link);
        nav.append(item);
        root.append(tab.pane);
    });

    root.append(tagEditor._tags);

    /* The editor is located inside the form, avoid that enter submits it. */
    root.addEventListener("keydown", function (event) {
        if (("Enter" === event.key) && ("INPUT" === event.target.tagName)) {
            event.preventDefault();
        }
    });

    return root;
};

/* Place the editor above the input or its input group. */
tagEditor._place = function () {
    if (null !== tagEditor._root) {
        const anchor = tagEditor._input.closest(".input-group") || tagEditor._input;

        tagEditor._root.style.left = anchor.offsetLeft + "px";
        tagEditor._root.style.top = anchor.offsetTop + "px";
        tagEditor._root.style.width = anchor.offsetWidth + "px";
    }
};

/* Show the parse result in the preview element. Its a separate step, so it
 * can be replaced by a more accurate one.
 */
tagEditor._render = function (preview, parsed) {
    const FLEX = { left: "flex-start", top: "flex-start", center: "center", right: "flex-end", bottom: "flex-end" };
    const text = tagEditor._create("div", "tag-editor-preview-text");

    parsed.runs.forEach(function (run) {
        const span = tagEditor._create("span", "", run.text);

        if ("gradient" === run.brush.type) {
            span.className = "tag-editor-gradient";
            span.style.backgroundImage = "linear-gradient(to " + (("vertical" === run.brush.direction) ? "bottom" : "right") + ", " + run.brush.startColor + ", " + run.brush.endColor + ")";
        } else {
            span.style.color = run.brush.color;
        }

        text.append(span);
    });

    text.style.textAlign = parsed.hAlign;
    preview.style.justifyContent = FLEX[parsed.hAlign];
    preview.style.alignItems = FLEX[parsed.vAlign];
    preview.replaceChildren(text);
};

tagEditor._updatePreview = function () {
    if (null !== tagEditor._root) {
        const options = tagEditor._options.get(tagEditor._input);
        let text = tagEditor._input.value;
        let parsed = null;

        if ("function" === typeof options.resolvePreview) {
            text = options.resolvePreview(text);
        }

        text = (options.previewPrefix || "") + text;

        (options.extraTabs || []).forEach(function (tab) {
            if ("function" === typeof tab.resolvePreview) {
                text = tab.resolvePreview(text);
            }
        });

        parsed = tagEditor.parse(text);
        tagEditor._render(tagEditor._preview, parsed);

        if ("" !== tagEditor._warning) {
            tagEditor._hint.textContent = tagEditor._warning;
        } else if (("number" === typeof options.maxLength) && (options.maxLength < tagEditor._input.value.length)) {
            /* E.g. set via REST API. The browser refuses to submit it after a change. */
            tagEditor._hint.textContent = "The text is longer than " + options.maxLength + " characters, please shorten it.";
        } else {
            tagEditor._hint.textContent = (true === parsed.isValid) ? "" : tagEditor._HINT_INVALID;
        }
    }
};

/* Select a part of the text in the input of the open editor. */
tagEditor._select = function (start, end) {
    tagEditor._input.focus();
    tagEditor._input.setSelectionRange(start, end);
};

/* Show the text of the input as row of tags. A click selects the tag or the text
 * between in the input, a tag can be removed as a whole.
 */
tagEditor._updateTags = function () {
    if (null !== tagEditor._root) {
        const options = tagEditor._options.get(tagEditor._input);
        const value = tagEditor._input.value;
        const placeholders = [];
        const row = [];

        (options.extraTabs || []).forEach(function (tab) {
            (tab.items || []).forEach(function (item) {
                placeholders.push(item.insert);
            });
        });

        tagEditor.split(value, placeholders).forEach(function (segment) {
            const onSelect = function () {
                tagEditor._select(segment.start, segment.end);
            };

            if (false === segment.isTag) {
                const text = tagEditor._create("span", "tag-editor-text text-secondary", segment.str, { title: "Select in the text field" });

                text.addEventListener("click", onSelect);
                row.push(text);
            } else {
                const tag = tagEditor._create("span", "tag-editor-tag badge text-bg-secondary", segment.str);
                const remove = tagEditor._create("button", "tag-editor-tag-remove", "×", { "type": "button", "title": "Remove " + segment.str, "aria-label": "Remove " + segment.str });
                const color = (true === segment.str.startsWith("{#")) ? tagEditor._toColor(segment.str.substring(2, segment.str.length - 1)) : null;

                if (null !== color) {
                    tag.style.borderLeft = "0.75rem solid " + color;
                }

                tag.title = "Select in the text field";
                tag.addEventListener("click", onSelect);

                remove.addEventListener("click", function (event) {
                    event.stopPropagation();
                    tagEditor._replaceRange(segment.start, segment.end, "");
                });

                tag.append(remove);
                row.push(tag);
            }
        });

        tagEditor._tags.replaceChildren(...row);
    }
};

/* Replace a part of the text of the input of the open editor and notify about it. */
tagEditor._replaceRange = function (start, end, replacement) {
    const input = tagEditor._input;
    const expected = input.value.substring(0, start) + replacement + input.value.substring(end);
    const cursor = start + replacement.length;

    input.focus();
    input.setSelectionRange(start, end);

    /* execCommand() is deprecated, but its the only way to keep the undo history of
     * the browser. It notifies the listeners by itself. If its not supported, the text
     * is set directly.
     */
    if ((expected !== input.value) &&
        ((false === document.execCommand(("" === replacement) ? "delete" : "insertText", false, replacement)) || (expected !== input.value))) {
        input.value = expected;

        /* Notify other listeners. The own one clears the warning and updates the editor. */
        input.dispatchEvent(new Event("input", { bubbles: true }));
    }

    input.setSelectionRange(cursor, cursor);
};

/* Insert the tag into the input of the open editor. */
tagEditor._insert = function (tag) {
    const input = tagEditor._input;
    const options = tagEditor._options.get(input);
    const result = tagEditor.insertAt(input.value, input.selectionStart, input.selectionEnd, tag, options.maxLength);

    if (null === result) {
        tagEditor._warning = "Not inserted: the text would exceed " + options.maxLength + " characters.";
        tagEditor._updatePreview();
    } else {
        tagEditor._replaceRange(input.selectionStart, input.selectionStart, tag);
    }
};

tagEditor._close = function () {
    if (null !== tagEditor._root) {
        window.clearInterval(tagEditor._timer);
        tagEditor._root.remove();
        tagEditor._root = null;
        tagEditor._input = null;
        tagEditor._timer = null;
        tagEditor._warning = "";
    }
};

tagEditor._open = function (input) {
    const options = tagEditor._options.get(input);
    const anchor = input.closest(".input-group") || input;
    let refreshMs = 0;

    tagEditor._close();

    tagEditor._input = input;
    tagEditor._root = tagEditor._build(options);

    anchor.parentNode.classList.add("tag-editor-host");
    /* Located behind the input, to be the next in the tab order. Its shown above, see CSS. */
    anchor.after(tagEditor._root);

    tagEditor._place();
    tagEditor._updatePreview();
    tagEditor._updateTags();

    /* Time dependent preview? Use the shortest refresh interval. */
    (options.extraTabs || []).forEach(function (tab) {
        if (("number" === typeof tab.previewRefreshMs) && (0 < tab.previewRefreshMs) && ((0 === refreshMs) || (refreshMs > tab.previewRefreshMs))) {
            refreshMs = tab.previewRefreshMs;
        }
    });

    if (0 < refreshMs) {
        tagEditor._timer = window.setInterval(tagEditor._updatePreview, refreshMs);
    }
};

/* Register the page wide listeners once. */
tagEditor._init = function () {
    const closeIfOutside = function (event) {
        if ((null !== tagEditor._root) && (event.target !== tagEditor._input) && (false === tagEditor._root.contains(event.target))) {
            tagEditor._close();
        }
    };

    document.addEventListener("pointerdown", closeIfOutside);
    document.addEventListener("focusin", closeIfOutside);
    document.addEventListener("keydown", function (event) {
        if ("Escape" === event.key) {
            tagEditor._close();
        }
    });
    window.addEventListener("resize", tagEditor._place);
};

/* Attach the tag editor to a text input. It opens if the input gets the focus.
 *
 * Options (all optional):
 * - extraTabs: Array of plugin specific tabs:
 *     { title, items: [{ insert, label, description }], resolvePreview(text), previewRefreshMs }
 *     resolvePreview replaces plugin specific placeholders for the preview.
 *     previewRefreshMs refreshes a time dependent preview periodically.
 * - presets: Array of { label, value }, offered in an own tab. A preset replaces the whole text.
 * - resolvePreview(text): Converts the input value to the text the firmware gets, for the preview.
 * - previewPrefix: Text which the firmware puts in front of the input value.
 * - maxLength: Max. length of the input value.
 */
tagEditor.attach = function (input, options) {
    const usedOptions = options || {};

    if (false === tagEditor._isInitialized) {
        tagEditor._init();
        tagEditor._isInitialized = true;
    }

    tagEditor._options.set(input, usedOptions);

    if ("number" === typeof usedOptions.maxLength) {
        input.maxLength = usedOptions.maxLength;
    }

    input.addEventListener("pointerdown", function () {
        tagEditor._isPointerFocus = true;
    });

    input.addEventListener("blur", function () {
        tagEditor._isPointerFocus = false;
    });

    input.addEventListener("focus", function () {
        /* Getting the focus back from the editor shall change nothing. */
        if (tagEditor._input !== input) {
            /* The browser selects the whole text if the focus is set by keyboard.
             * Avoid that the first insert happens in front of everything.
             */
            if (false === tagEditor._isPointerFocus) {
                const length = input.value.length;

                input.setSelectionRange(length, length);
                window.setTimeout(function () {
                    input.setSelectionRange(length, length);
                }, 0);
            }

            tagEditor._open(input);
        }
    });

    /* Reopen the editor in case it was closed while the input kept the focus. */
    input.addEventListener("click", function () {
        if (tagEditor._input !== input) {
            tagEditor._open(input);
        }
    });

    input.addEventListener("input", function () {
        if (tagEditor._input === input) {
            tagEditor._warning = "";
            tagEditor._updatePreview();
            tagEditor._updateTags();
        }
    });
};

if ("object" === typeof module) {
    module.exports = tagEditor;
}
