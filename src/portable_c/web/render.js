// The screen of the program in a web page: the 80 by 25 cells of the
// BIOS's page, which the program hands over at each yield, drawn as text
// (CLAUDE.md, the road, step 7; TODO.md, "Step 7, the web page").
//
// The page is a <pre> of 25 rows, a <span> each, with a line's end
// between them. A row is a span for each run of cells of one attribute,
// of the class that the attribute names (css below): the VGA's colours of
// mode 3, exactly, and its bit 7 a blink of the character, as the bench's
// VGA has it with blinking on. Only the rows whose cells have changed
// since the last draw are made anew.
//
// A cell is a word as the BIOS keeps it in the screen's memory and as
// the program's grid has it: the byte of the character below, the
// attribute above. The character is shown as the VGA's font shows the
// byte, by a table of 256 code points that the page makes of the
// program's own (CodePageGlyph), so that there is one source of it.
//
// The cursor is drawn over the cells by the page; this says where it is
// and whether it is a block or a line, by the BIOS's shape of it, as the
// program for a terminal decides it (ShapeOf of the terminal's code): a
// shape with bit 5 of its first line, or whose first line is below its
// last, is none, one of more than four lines is a block, the rest a line.
//
// The mouse's pointer needs nothing here: the program has it in the
// cells, the cell with its colours exchanged, as the bench's driver
// draws it.
//
// A plain module of ECMAScript, with nothing of the browser's in it but
// in `apply`, so that the test runs it in Node.

export const COLUMNS = 80;
export const ROWS = 25;
export const CELLS = COLUMNS * ROWS;

// The VGA's 16 colours of mode 3, as the terminal's code has them.
export const VGA = [
    "#000000", "#0000aa", "#00aa00", "#00aaaa",
    "#aa0000", "#aa00aa", "#aa5500", "#aaaaaa",
    "#555555", "#5555ff", "#55ff55", "#55ffff",
    "#ff5555", "#ff55ff", "#ffff55", "#ffffff",
];

// The VGA shows a blinking character for 16 of its frames and hides it
// for 16, at 70 frames a second (32 frames, 457 ms, a round); the cursor
// blinks twice as fast. The page turns BLINK_CLASS on its <pre> on and
// off every BLINK_HALF milliseconds, so that every blinking cell goes in
// step, whenever its row was made.
export const BLINK_HALF = 16 * 1000 / 70.086;
export const CURSOR_HALF = 8 * 1000 / 70.086;
export const PREFIX = "vc";
export const BLINK_CLASS = `${PREFIX}-off`;

// The class of an attribute: vc-a and its two hex digits; a blinking one
// has vc-b besides.
export function classOf(attribute) {
    const name = `${PREFIX}-a${attribute.toString(16).padStart(2, "0")}`;
    return (attribute & 0x80) !== 0 ? `${name} ${PREFIX}-b` : name;
}

// The rules of the 256 attributes: the character's colour of the low
// four bits, the background of the next three (bit 7 being the blink),
// and a blinking character made transparent while the <pre> has
// BLINK_CLASS.
export function css() {
    const rules = [];
    for (let attribute = 0; attribute < 256; attribute++) {
        const name = `${PREFIX}-a${attribute.toString(16).padStart(2, "0")}`;
        rules.push(`.${name}{color:${VGA[attribute & 0x0F]};`
                   + `background:${VGA[attribute >> 4 & 0x07]}}`);
    }
    rules.push(`.${BLINK_CLASS} .${PREFIX}-b{color:transparent}`);
    return rules.join("\n") + "\n";
}

// What a character is in HTML: the three that markup would take for its
// own escaped.
function escaped(text) {
    return text.replace(/&/g, "&amp;").replace(/</g, "&lt;")
        .replace(/>/g, "&gt;");
}

// The glyphs of the 256 bytes as text for HTML, of a table of code
// points.
export function glyphsOf(codePoints) {
    if (codePoints.length !== 256)
        throw new Error("a table of glyphs has 256 code points");
    return Array.from(codePoints, (point) => escaped(String.fromCodePoint(point)));
}

// The HTML of a row of `cells` (an array of 2000 words, or a typed one):
// a span for each run of one attribute.
export function rowHtml(cells, row, glyphs) {
    let html = "";
    let attribute = -1;
    let run = "";
    for (let at = row * COLUMNS; at < (row + 1) * COLUMNS; at++) {
        const cell = cells[at];
        if (cell >> 8 !== attribute) {
            if (attribute >= 0)
                html += `<span class="${classOf(attribute)}">${run}</span>`;
            attribute = cell >> 8;
            run = "";
        }
        run += glyphs[cell & 0xFF];
    }
    return html + `<span class="${classOf(attribute)}">${run}</span>`;
}

// The cursor of a place, the row above the column as the BIOS keeps it,
// and a shape, the first line above the last: null where none is shown,
// or where it is off the screen.
export function cursorOf(place, shape) {
    const row = place >> 8 & 0xFF, column = place & 0xFF;
    const first = shape >> 8 & 0x1F, last = shape & 0x1F;
    if ((shape & 0x2000) !== 0 || first > last || row >= ROWS
        || column >= COLUMNS)
        return null;
    return { row, column, block: last - first > 3 };
}

// What the page shows, and what it is told to change at each draw.
export class Screen {
    // `codePoints` is the table of the 256 glyphs.
    constructor(codePoints) {
        this.glyphs = glyphsOf(codePoints);
        this.shown = null;
    }

    // The cells handed over, with the cursor's place and shape: the rows
    // that have changed since the last draw, each [row, html], all of
    // them at the first, and the cursor (cursorOf). The cells are copied,
    // since the program goes on writing into its own.
    draw(cells, place, shape) {
        const rows = [];
        if (cells.length !== CELLS)
            throw new Error(`a screen has ${CELLS} cells`);
        for (let row = 0; row < ROWS; row++) {
            if (this.shown !== null) {
                let same = true;
                for (let at = row * COLUMNS; at < (row + 1) * COLUMNS; at++)
                    if (cells[at] !== this.shown[at]) {
                        same = false;
                        break;
                    }
                if (same)
                    continue;
            }
            rows.push([row, rowHtml(cells, row, this.glyphs)]);
        }
        this.shown = Uint16Array.from(cells);
        return { rows, cursor: cursorOf(place, shape) };
    }

    // Draw everything again at the next draw.
    forget() {
        this.shown = null;
    }
}

// What a draw has changed, made so in the page: `rowElements` the 25
// spans of the <pre>, `cursorElement` an element over it placed by the
// cell's size, `cell` { width, height } in pixels.
export function apply(drawn, rowElements, cursorElement, cell) {
    for (const [row, html] of drawn.rows)
        rowElements[row].innerHTML = html;
    if (cursorElement === null)
        return;
    const cursor = drawn.cursor;
    if (cursor === null) {
        cursorElement.style.display = "none";
        return;
    }
    // A line is the two lowest of the 16 lines of the VGA's cell.
    const height = cursor.block ? cell.height : cell.height * 2 / 16;
    cursorElement.style.display = "block";
    cursorElement.style.left = `${cursor.column * cell.width}px`;
    cursorElement.style.top = `${(cursor.row + 1) * cell.height - height}px`;
    cursorElement.style.width = `${cell.width}px`;
    cursorElement.style.height = `${height}px`;
}
