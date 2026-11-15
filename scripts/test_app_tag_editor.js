"use strict";

/* Tests of the pure part of the tag editor (no DOM required).
 * Run from the repository root: node --test scripts/test_app_tag_editor.js
 */

const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const zlib = require("node:zlib");
const tagEditor = require("./app-tag-editor.js");

const WHITE = { type: "solid", color: "#FFFFFF" };
const RED = { type: "solid", color: "#FF0000" };

test("parse returns plain text as one white run", () => {
    assert.deepEqual(tagEditor.parse("12:34"), { isValid: true, hAlign: "left", vAlign: "top", runs: [{ text: "12:34", brush: WHITE }] });
});

test("parse applies solid color and alignment", () => {
    const result = tagEditor.parse("{hc}{vb}{#ff0000}12{#FFFFFF}:34");
    assert.equal(result.hAlign, "center");
    assert.equal(result.vAlign, "bottom");
    assert.deepEqual(result.runs, [{ text: "12", brush: RED }, { text: ":34", brush: WHITE }]);
});

test("parse applies gradient and switches brushes", () => {
    const gradient = { type: "gradient", direction: "vertical", startColor: "#FF0000", endColor: "#0000FF" };
    const result = tagEditor.parse("{#FF0000}a{lgv #FF0000,#0000FF,0,8}b{stc}c{lgtc}d");
    assert.deepEqual(result.runs, [{ text: "a", brush: RED }, { text: "b", brush: gradient }, { text: "c", brush: RED }, { text: "d", brush: gradient }]);
});

test("parse applies horizontal alignment only if text follows", () => {
    assert.equal(tagEditor.parse("12{hr}").hAlign, "left");
    assert.equal(tagEditor.parse("{hc}12{hr}").hAlign, "center");
    assert.equal(tagEditor.parse("{hc}12{hr}{0x41}").hAlign, "right");
});

test("parse maps horizontal gradient direction", () => {
    assert.equal(tagEditor.parse("{lgh #000000,#FFFFFF,0,32}x").runs[0].brush.direction, "horizontal");
});

test("parse uses the firmware default gradient before one is set", () => {
    const gradient = { type: "gradient", direction: "vertical", startColor: "#FF0000", endColor: "#0000FF" };
    assert.deepEqual(tagEditor.parse("{lgtc}x").runs, [{ text: "x", brush: gradient }]);
});

test("parse skips unknown tags and bad parameters", () => {
    assert.deepEqual(tagEditor.parse("a{foo}b{#GGGGGG}c{lgv #FF0000}d").runs, [{ text: "abcd", brush: WHITE }]);
    assert.equal(tagEditor.parse("a{foo}b").isValid, true);
});

test("parse replaces special character codes with the placeholder", () => {
    assert.deepEqual(tagEditor.parse("a{0x7F}b").runs, [{ text: "a" + tagEditor.SPECIAL_CHAR_PLACEHOLDER + "b", brush: WHITE }]);
});

test("parse handles escaped braces", () => {
    assert.deepEqual(tagEditor.parse("a\\{b\\}c"), { isValid: true, hAlign: "left", vAlign: "top", runs: [{ text: "a{b}c", brush: WHITE }] });
});

test("parse drops an unterminated tag at the end", () => {
    assert.deepEqual(tagEditor.parse("12{#FF"), { isValid: true, hAlign: "left", vAlign: "top", runs: [{ text: "12", brush: WHITE }] });
});

test("parse reports structurally invalid text and keeps the rest raw", () => {
    assert.deepEqual(tagEditor.parse("a}b").runs, [{ text: "a}b", brush: WHITE }]);
    for (const text of ["a}b", "{#FF{hc}", "{#FF\\}", "{h\nc}"]) {
        assert.equal(tagEditor.parse(text).isValid, false, text);
    }
});

test("parse keeps line feeds and empty input", () => {
    assert.deepEqual(tagEditor.parse("a\nb").runs, [{ text: "a\nb", brush: WHITE }]);
    assert.deepEqual(tagEditor.parse("").runs, []);
});

test("insertAt inserts at the cursor", () => {
    assert.deepEqual(tagEditor.insertAt("%H:%M", 3, 3, "{hc}", undefined), { value: "%H:{hc}%M", cursor: 7 });
});

test("insertAt puts the tag in front of a selection and keeps it", () => {
    assert.deepEqual(tagEditor.insertAt("%H:%M", 0, 5, "{#FF0000}", undefined), { value: "{#FF0000}%H:%M", cursor: 9 });
});

test("insertAt refuses when the limit would be exceeded", () => {
    assert.equal(tagEditor.insertAt("12345678", 8, 8, "{hc}", 10), null);
    assert.deepEqual(tagEditor.insertAt("123456", 6, 6, "{hc}", 10), { value: "123456{hc}", cursor: 10 });
});

test("committed gz matches the source", () => {
    const normalize = (buffer) => buffer.toString("utf8").replaceAll("\r\n", "\n");
    const source = fs.readFileSync(path.join(__dirname, "app-tag-editor.js"));
    const packed = fs.readFileSync(path.join(__dirname, "..", "data", "js", "app-tag-editor.js.gz"));
    assert.equal(normalize(zlib.gunzipSync(packed)), normalize(source));
});

test("split returns tags and plugin placeholders with their positions", () => {
    assert.deepEqual(tagEditor.split("{hc}{#FF0000}%H:%M", ["%H", "%M"]), [
        { start: 0, end: 4, str: "{hc}", isTag: true },
        { start: 4, end: 13, str: "{#FF0000}", isTag: true },
        { start: 13, end: 15, str: "%H", isTag: true },
        { start: 15, end: 16, str: ":", isTag: false },
        { start: 16, end: 18, str: "%M", isTag: true }
    ]);
});

test("split treats placeholders as text if none are given", () => {
    assert.deepEqual(tagEditor.split("%H:%M"), [{ start: 0, end: 5, str: "%H:%M", isTag: false }]);
});

test("split prefers the placeholder which starts first", () => {
    assert.deepEqual(tagEditor.split("%%H", ["%H", "%%"]), [
        { start: 0, end: 2, str: "%%", isTag: true },
        { start: 2, end: 3, str: "H", isTag: false }
    ]);
});

test("split keeps escaped, stray and unterminated braces as text", () => {
    assert.deepEqual(tagEditor.split("a\\{b}"), [{ start: 0, end: 5, str: "a\\{b}", isTag: false }]);
    assert.deepEqual(tagEditor.split("12{#FF"), [{ start: 0, end: 6, str: "12{#FF", isTag: false }]);
    assert.deepEqual(tagEditor.split(""), []);
});

test("split does not search placeholders inside tags", () => {
    assert.deepEqual(tagEditor.split("{%H}", ["%H"]), [{ start: 0, end: 4, str: "{%H}", isTag: true }]);
});

test("strftime resolves time specifiers with zero padding", () => {
    assert.equal(tagEditor.strftime("%H:%M:%S", new Date(2026, 10, 5, 7, 8, 9)), "07:08:09");
});

test("strftime resolves the 12h format", () => {
    assert.equal(tagEditor.strftime("%I %p", new Date(2026, 10, 5, 0, 30, 0)), "12 AM");
    assert.equal(tagEditor.strftime("%I %p", new Date(2026, 10, 5, 12, 30, 0)), "12 PM");
    assert.equal(tagEditor.strftime("%I %p", new Date(2026, 10, 5, 13, 30, 0)), "01 PM");
});

test("strftime resolves date specifiers and english names", () => {
    assert.equal(tagEditor.strftime("%d.%m.%Y %y", new Date(2026, 10, 5, 7, 8, 9)), "05.11.2026 26");
    assert.equal(tagEditor.strftime("%a %A %b %B", new Date(2026, 10, 5, 7, 8, 9)), "Thu Thursday Nov November");
});

test("strftime keeps unknown specifiers and format tags", () => {
    assert.equal(tagEditor.strftime("%% %Q {#FF0000}", new Date(2026, 10, 5, 7, 8, 9)), "% %Q {#FF0000}");
});

test("strftime resolves every offered item", () => {
    const date = new Date(2026, 10, 5, 7, 8, 9);

    assert.equal(tagEditor.STRFTIME_ITEMS.length, 14);
    for (const item of tagEditor.STRFTIME_ITEMS) {
        assert.notEqual(tagEditor.strftime(item.insert, date), item.insert, item.insert);
        assert.ok(item.description.startsWith(item.insert + " - "), item.insert);
    }
});

test("printf formats numbers with precision, width and flags", () => {
    assert.equal(tagEditor.printf("%f", 22.5), "22.500000");
    assert.equal(tagEditor.printf("%.1f C", 22.46), "22.5 C");
    assert.equal(tagEditor.printf("%5.1f|%-6.1f|%05.1f|%+.0f", 2.3), "  2.3|2.3   |002.3|+2");
});

test("printf formats strings", () => {
    assert.equal(tagEditor.printf("%s", "abc"), "abc");
    assert.equal(tagEditor.printf("%5s|%-5s|", "abc"), "  abc|abc  |");
});

test("printf keeps percent signs, unknown specifiers and format tags", () => {
    assert.equal(tagEditor.printf("{#FF0000}%.0f%% %d", 99.6), "{#FF0000}100% %d");
});

test("printf resolves every offered item", () => {
    assert.equal(tagEditor.PRINTF_ITEMS.length, 5);
    for (const item of tagEditor.PRINTF_ITEMS) {
        assert.notEqual(tagEditor.printf(item.insert, 22.5), item.insert, item.insert);
        assert.ok(item.description.startsWith(item.insert + " - "), item.insert);
    }
});
