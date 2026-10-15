// The half of the test portable_c-web-render that runs in Node: the
// modules of the web page (src/portable_c/web) handed what
// harness/webrender.py has made, and what they answer written back for
// it to hold. Run as: node webrender.mjs WEB_DIRECTORY INPUT OUTPUT.

import { readFileSync, writeFileSync } from "node:fs";
import { join } from "node:path";
import { pathToFileURL } from "node:url";

const [web, input, output] = process.argv.slice(2);
const load = (name) => import(pathToFileURL(join(web, name)).href);
const render = await load("render.js");
const keys = await load("keys.js");
const mouse = await load("mouse.js");
const given = JSON.parse(readFileSync(input, "utf8"));

// The screens, drawn one behind the other on one Screen, as the page
// draws them at its yields.
const screen = new render.Screen(given.glyphs);
const draws = given.screens.map(({ cells, place, shape }) =>
    screen.draw(Uint16Array.from(cells), place, shape));

// apply, with elements that are objects: the rows' HTML, the cursor's
// style.
const rows = Array.from({ length: render.ROWS }, () => ({ innerHTML: "" }));
const cursor = { style: {} };
const applied = draws.map((drawn) => {
    render.apply(drawn, rows, cursor, { width: 8, height: 16 });
    return { rows: rows.map((row) => row.innerHTML),
             cursor: { ...cursor.style } };
});

// A KeyboardEvent's shape, of what the test says of it.
function keyboardEvent(e) {
    const states = new Set(e.states || []);
    return { type: e.type, code: e.code, key: e.key, repeat: !!e.repeat,
             shiftKey: !!e.shift, altKey: !!e.alt, ctrlKey: !!e.ctrl,
             metaKey: !!e.meta, getModifierState: (name) => states.has(name) };
}
const keyed = given.keys.map((events) =>
    events.map((e) => keys.keyEvent(keyboardEvent(e))));

const pointed = given.mouse.map(({ rect, events }) => {
    const m = new mouse.Mouse();
    return events.map((e) => m.event(e, rect));
});
const cells = given.cells.map(([x, y, rect]) => mouse.cellOf(x, y, rect));

writeFileSync(output, JSON.stringify({
    css: render.css(), draws, applied, keys: keyed, mouse: pointed, cells,
    blink: [render.BLINK_HALF, render.CURSOR_HALF, render.BLINK_CLASS],
}));
