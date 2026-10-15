// The keys of a web page made what the program for Linux takes from a
// terminal with the kitty protocol of the keyboard (CLAUDE.md, the road,
// step 7; TODO.md, "Step 7, the web page"): a key event of the browser,
// keydown or keyup, becomes one of kitty's, of the shape that kitty sends
// with the flags that the program pushes (31: every key an escape code,
// with its press, repeat or release, the key of a US layout behind it,
// and the text that it types), and the program's own decoder, the one of
// the terminal, makes of it the word of the bench's BIOS and what is
// held. So there is one decoder, and the page decides nothing of the
// BIOS's words.
//
// An event of kitty's is a tuple: the key, as kitty numbers it (the
// character that the key types unshifted on the layout in use, or a code
// of kitty's own for a key that types none), the character that it types
// shifted, the key of a US layout in the same place, the modifiers, the
// kind of event, the text, and the final byte of its sequence, which is
// `u` but for the keys that keep the sequences of xterm (the arrows,
// Home, End, F1, F2 and F4 by a letter; Ins, Del, the pages and the
// other function keys by a number and `~`). `sequence` writes a tuple as
// the bytes that kitty would send.
//
// What a page cannot have: a key that the browser keeps for itself is
// never seen (TODO.md has the list); a character made of several keys,
// by a dead key or an input method, comes by no key event, and is
// dropped as a terminal of kitty's would not drop it.
//
// A plain module of ECMAScript that takes objects of the shape of a
// KeyboardEvent, so that the test runs it in Node.

export const CSI = "\x1b[";

// kitty's modifiers, as bits of the number less 1.
export const SHIFT = 1, ALT = 2, CTRL = 4, SUPER = 8, CAPS_LOCK = 64,
    NUM_LOCK = 128;
export const PRESS = 1, REPEAT = 2, RELEASE = 3;

// What a terminal sends when it has lost the focus: what was held is
// then let go, since its release goes elsewhere.
export const FOCUS_OUT = `${CSI}O`;

// The keys that keep the sequences of xterm, by `event.code`: the
// number and the final byte.
const LEGACY = {
    ArrowUp: [1, "A"], ArrowDown: [1, "B"], ArrowRight: [1, "C"],
    ArrowLeft: [1, "D"], Home: [1, "H"], End: [1, "F"],
    Insert: [2, "~"], Delete: [3, "~"], PageUp: [5, "~"],
    PageDown: [6, "~"],
    F1: [1, "P"], F2: [1, "Q"], F3: [13, "~"], F4: [1, "S"],
    F5: [15, "~"], F6: [17, "~"], F7: [18, "~"], F8: [19, "~"],
    F9: [20, "~"], F10: [21, "~"], F11: [23, "~"], F12: [24, "~"],
};

// The keys that type no character, by `event.code`, as kitty numbers
// them.
const FUNCTIONAL = {
    Escape: 27, Enter: 13, Tab: 9, Backspace: 127,
    CapsLock: 57358, ScrollLock: 57359, NumLock: 57360,
    PrintScreen: 57361, Pause: 57362, ContextMenu: 57363,
    F13: 57376, F14: 57377, F15: 57378, F16: 57379, F17: 57380,
    F18: 57381, F19: 57382, F20: 57383, F21: 57384, F22: 57385,
    F23: 57386, F24: 57387,
    ShiftLeft: 57441, ControlLeft: 57442, AltLeft: 57443, MetaLeft: 57444,
    ShiftRight: 57447, ControlRight: 57448, AltRight: 57449,
    MetaRight: 57450,
};
// The right Alt of a layout that has it as AltGraph: kitty's
// ISO_LEVEL3_SHIFT, which holds no Alt.
const ALT_GRAPH = 57453;

// The keypad: kitty's code with Num Lock on, with it off, and the text.
const PAD = {
    Numpad0: [57399, 57425], Numpad1: [57400, 57424],
    Numpad2: [57401, 57420], Numpad3: [57402, 57422],
    Numpad4: [57403, 57417], Numpad5: [57404, 57427],
    Numpad6: [57405, 57418], Numpad7: [57406, 57423],
    Numpad8: [57407, 57419], Numpad9: [57408, 57421],
    NumpadDecimal: [57409, 57426],
};
const PAD_SIGNS = {
    NumpadDivide: 57410, NumpadMultiply: 57411, NumpadSubtract: 57412,
    NumpadAdd: 57413, NumpadEnter: 57414, NumpadEqual: 57415,
    NumpadComma: 57416,
};

// The character of a US layout at a key that types one, by `event.code`.
const US = {
    Minus: "-", Equal: "=", BracketLeft: "[", BracketRight: "]",
    Backslash: "\\", Semicolon: ";", Quote: "'", Backquote: "`",
    Comma: ",", Period: ".", Slash: "/", Space: " ",
};

function usOf(code) {
    if (/^Key[A-Z]$/.test(code))
        return code.charCodeAt(3) + 0x20;
    if (/^Digit[0-9]$/.test(code))
        return code.charCodeAt(5);
    return code in US ? US[code].charCodeAt(0) : 0;
}

// The one code point of `key`, or 0 where it is a name, "Enter", or more
// than one.
function single(key) {
    if (typeof key !== "string" || key.length === 0)
        return 0;
    const point = key.codePointAt(0);
    return String.fromCodePoint(point).length === key.length ? point : 0;
}

function held(event) {
    const state = (name) => typeof event.getModifierState === "function"
        && event.getModifierState(name);
    const graph = state("AltGraph");
    let mods = 0;
    if (event.shiftKey)
        mods |= SHIFT;
    // AltGraph is a shift of the layout, not Alt: on Windows it comes as
    // Ctrl and Alt held, which are then none.
    if (event.altKey && !graph)
        mods |= ALT;
    if (event.ctrlKey && !graph)
        mods |= CTRL;
    if (event.metaKey)
        mods |= SUPER;
    if (state("CapsLock"))
        mods |= CAPS_LOCK;
    if (state("NumLock"))
        mods |= NUM_LOCK;
    return mods;
}

// The event of kitty's for a KeyboardEvent (or an object of its shape:
// type, code, key, repeat, shiftKey, altKey, ctrlKey, metaKey and
// getModifierState): null where the program has nothing to do with the
// key, and then the page leaves it to the browser.
export function kittyOf(event) {
    const kind = event.type === "keyup" ? RELEASE
        : event.repeat ? REPEAT : PRESS;
    const mods = held(event);
    const tuple = { code: 0, shifted: 0, base: 0, mods, event: kind,
                    text: 0, final: "u" };
    const code = event.code;

    if (code in LEGACY) {
        [tuple.code, tuple.final] = LEGACY[code];
        return tuple;
    }
    if (code === "AltRight" && event.key === "AltGraph") {
        tuple.code = ALT_GRAPH;
        return tuple;
    }
    if (code in FUNCTIONAL) {
        tuple.code = FUNCTIONAL[code];
        return tuple;
    }
    if (code in PAD_SIGNS) {
        tuple.code = PAD_SIGNS[code];
    } else if (code in PAD) {
        // With Num Lock off, or Shift turning it round where the system
        // does, the key types no figure.
        const figure = single(event.key);
        tuple.code = PAD[code][figure !== 0 ? 0 : 1];
    }
    if (tuple.code !== 0) {
        const typed = single(event.key);
        if (kind !== RELEASE && typed >= 0x20 && (mods & (ALT | CTRL)) === 0)
            tuple.text = typed;
        return tuple;
    }

    // A key that types a character.
    const us = usOf(code);
    const typed = single(event.key);
    if (typed === 0 && us === 0)
        return null;
    if (typed === 0 || typed < 0x20 || typed === 0x7F) {
        // A dead key, or a character that the browser gives no text of:
        // the key of the US layout.
        tuple.code = us;
    } else {
        const lower = String.fromCodePoint(typed).toLowerCase();
        const unshifted = single(lower) !== 0 ? single(lower) : typed;
        if ((mods & SHIFT) !== 0 && unshifted === typed && us !== 0) {
            // Shift with a key that is no letter: what it types unshifted
            // on this layout is not told, the US layout's is taken.
            tuple.code = us;
        } else {
            tuple.code = unshifted;
        }
        if ((mods & SHIFT) !== 0 && typed !== tuple.code)
            tuple.shifted = typed;
        if (kind !== RELEASE && (mods & (ALT | CTRL)) === 0)
            tuple.text = typed;
    }
    if (us !== 0 && us !== tuple.code)
        tuple.base = us;
    return tuple;
}

// The bytes, as a string of code units below 80h but for the text in
// decimal, that kitty sends for a tuple.
export function sequence(tuple) {
    let key = String(tuple.code);
    if (tuple.shifted !== 0 || tuple.base !== 0)
        key += `:${tuple.shifted !== 0 ? tuple.shifted : ""}`;
    if (tuple.base !== 0)
        key += `:${tuple.base}`;
    let mods = "";
    if (tuple.mods !== 0 || tuple.event !== PRESS)
        mods = String(tuple.mods + 1);
    if (tuple.event !== PRESS)
        mods += `:${tuple.event}`;
    if (tuple.final !== "u") {
        // xterm's form: CSI A, CSI 5~, or with what is held CSI 1;5A.
        if (mods === "")
            return tuple.code === 1 ? `${CSI}${tuple.final}`
                : `${CSI}${tuple.code}${tuple.final}`;
        return `${CSI}${tuple.code};${mods}${tuple.final}`;
    }
    if (tuple.text !== 0)
        return `${CSI}${key};${mods};${tuple.text}u`;
    if (mods !== "")
        return `${CSI}${key};${mods}u`;
    return `${CSI}${key}u`;
}

// What the page does with a KeyboardEvent: the bytes for the program, or
// null, and whether the browser is to be kept from doing what it does
// with the key, which is so for every key that the program takes.
export function keyEvent(event) {
    const tuple = kittyOf(event);
    if (tuple === null)
        return { bytes: null, prevent: false };
    return { bytes: sequence(tuple), prevent: true };
}
