// The mouse of a web page made what the program for Linux takes from a
// terminal (CLAUDE.md, the road, step 7; TODO.md, "Step 7, the web
// page"): a pointer event on the <pre> of the screen becomes reports of
// the mouse in SGR's form, CSI < b ; x ; y M, or m for a button let go,
// which the program's own decoder, the terminal's, makes the mouse
// driver's answers of. The cell is worked out from the place of the
// <pre> and its size, 80 by 25 cells, and kept to the screen.
//
// The buttons are the mouse driver's: 1 the left one, 2 the right one,
// 4 the middle one, which is what PointerEvent.buttons has too. One
// event may change several, a chord that pointerdown does not tell of;
// each change is a report of its own, in the order of the bits. The
// wheel is nothing, as in the terminal, a mouse driver of DOS having
// none, so it has no report here.
//
// A plain module of ECMAScript, which takes objects of the shape of a
// PointerEvent and of a DOMRect, so that the test runs it in Node.

import { COLUMNS, ROWS } from "./render.js";

const CSI = "\x1b[";

// SGR's number of a button of the driver's.
const SGR = { 1: 0, 4: 1, 2: 2 };

// The cell under a place of the client's, from 0: { column, row }, kept
// to the screen. `rect` is the <pre>'s (getBoundingClientRect).
export function cellOf(clientX, clientY, rect) {
    const column = Math.floor((clientX - rect.left) * COLUMNS / rect.width);
    const row = Math.floor((clientY - rect.top) * ROWS / rect.height);
    return { column: Math.min(Math.max(column, 0), COLUMNS - 1),
             row: Math.min(Math.max(row, 0), ROWS - 1) };
}

function report(b, cell, up) {
    return `${CSI}<${b};${cell.column + 1};${cell.row + 1}${up ? "m" : "M"}`;
}

// The mouse as the page last told the program of it.
export class Mouse {
    constructor() {
        this.buttons = 0;
        this.cell = null;
    }

    // The reports for a pointer event, pointerdown, pointerup,
    // pointermove or pointercancel, as one string, empty where there is
    // nothing new.
    event(event, rect) {
        const cell = cellOf(event.clientX, event.clientY, rect);
        const now = event.type === "pointercancel" ? 0 : event.buttons & 7;
        let out = "";
        for (const bit of [1, 2, 4]) {
            if (((now ^ this.buttons) & bit) === 0)
                continue;
            out += report(SGR[bit], cell, (now & bit) === 0);
        }
        const moved = this.cell === null || cell.column !== this.cell.column
            || cell.row !== this.cell.row;
        if (out === "" && moved) {
            // A motion: 32, with the lowest of the buttons held, or 3 for
            // none.
            const held = [1, 2, 4].find((bit) => (now & bit) !== 0);
            out = report(32 + (held === undefined ? 3 : SGR[held]), cell,
                         false);
        }
        this.buttons = now;
        this.cell = cell;
        return out;
    }
}
