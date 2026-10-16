// The web page itself, the piece W3b of step 7 (CLAUDE.md, "The web
// page"; TODO.md, "Step 7, the web page"): what ties the program, built by
// Emscripten with the page's library (library.js), to the browser. It
// draws the grid (render.js), hands over the keys and the mouse (keys.js,
// mouse.js), keeps the drives on the server (drives.js) or, served with no
// server, in the browser (kept.js), opens a file in
// a new tab, brings the user's files in by a button and by dragging them
// onto the page, has a button for the full screen, and on a phone fits the
// grid to the screen and has a row of the keys that a phone lacks. Its
// "Licences" shows the licences of all that it carries, which the build
// put into it from the repository's files (bundle.mjs, LICENCES).
//
// The build makes one classic script of this and the modules that it
// names (bundle.mjs), put into the page's shell (shell.html) in front of
// the program's own script: so it sets `Module` up before the program
// starts. The page's pre-js (pre.js) hands it Emscripten's FS and ENV
// when the program is about to start, `Module.vcBegin`.

import { COLUMNS, ROWS, BLINK_CLASS, BLINK_HALF, CURSOR_HALF, Screen, css, apply } from "./render.js";
import { keyEvent, FOCUS_OUT } from "./keys.js";
import { Mouse } from "./mouse.js";
import { Drives, ROOTS, CHANGE_ERRORS } from "./drives.js";
import { KeptDrives, openStore } from "./kept.js";

// How the page keeps its drives, chosen here and nowhere else. "server":
// the page's server (server.mjs), which every change is made on. "browser":
// a page served statically, with no server, as from GitHub Pages, whose
// drives start from what is served with it (static.mjs) and whose changes
// are kept in the browser, in IndexedDB, or in the page's memory where the
// browser gives it no IndexedDB (kept.js).
//
// Which it is, the page asks where it is served, askMode below: the project's server
// answers `api/server` with its name, and a static host has no such file,
// 404 or another refusal of 4xx. One page is built and served both ways;
// a build of its own for each would be a second page to build and test.
// An answer of 5xx, or none, is a server that should be there and is not
// (a proxy in front of a server that has stopped): the page says so and
// does not start, so that a page of the server never keeps its changes in
// the browser where the server is merely away. Unless the drives of a
// static site are served beside the page, `drives/c.json`, which the
// server never serves: then it is a static host that answers a name it
// has not otherwise than by 4xx, or that does not let the page ask for
// it at all, as a host of pages in a sandbox may (staticSite below).
const MODES = {
  server: async (fs) => new Drives({ fs, fetch: (...a) => fetch(...a),
                                     base: "" }),
  browser: async (fs) => new KeptDrives({
    fs, fetch: (...a) => fetch(...a), base: "drives/",
    store: await openStore(window.indexedDB) }),
};

// Whether the drives of a static site are served beside the page: the
// tree of C: as static.mjs writes it.
async function staticSite() {
  try {
    const answer = await fetch("drives/c.json", { cache: "no-cache" });
    if (!answer.ok)
      return false;
    const tree = await answer.json();
    return Array.isArray(tree && tree.entries);
  } catch {
    return false;
  }
}

async function askMode() {
  let answer;
  try {
    answer = await fetch("api/server", { cache: "no-store" });
  } catch (caught) {
    if (await staticSite())
      return "browser";
    throw new Error(`the page's server did not answer (${caught.message})`);
  }
  if (answer.ok) {
    let said = null;
    try {
      said = await answer.json();
    } catch {
      // a file of that name on a static host, which is no server
    }
    if (said && said.server === "vibe-commander")
      return "server";
    return "browser";
  }
  if (answer.status >= 400 && answer.status < 500)
    return "browser";
  if (await staticSite())
    return "browser";
  throw new Error(`the page's server answered ${answer.status}`);
}

// The type of a file that is opened in a new tab, by its extension.
const TYPES = {
  txt: "text/plain", doc: "text/plain", me: "text/plain", ini: "text/plain",
  bat: "text/plain", c: "text/plain", h: "text/plain", asm: "text/plain",
  inc: "text/plain", md: "text/plain", csv: "text/plain", log: "text/plain",
  js: "text/plain", py: "text/plain", ext: "text/plain", mnu: "text/plain",
  htm: "text/html", html: "text/html", json: "application/json",
  png: "image/png", jpg: "image/jpeg", jpeg: "image/jpeg",
  gif: "image/gif", bmp: "image/bmp", svg: "image/svg+xml",
  webp: "image/webp", pdf: "application/pdf", mp3: "audio/mpeg",
  wav: "audio/wav", ogg: "audio/ogg", mp4: "video/mp4", webm: "video/webm",
};

const $ = (id) => document.getElementById(id);

const page = {
  drives: null,
  mode: null,                     // a name of MODES, once it is known
  fs: null,
  where: "",                      // DOS's current directory, of the FS
  screen: null,
  cell: { width: 8, height: 16 },
  latched: { ctrl: false, alt: false },
  ended: false,
  unseen: new Set(),              // keys gone down while the licences were up
};

// What goes to the program, queued while it runs (library.js, web_input).
function queue(bytes) {
  if (!page.ended && bytes)
    Module.vcInput.push(bytes);
}

// A key as keys.js takes it, made up: for the row of keys and for the
// text of the phone's keyboard.
function made(type, code, key, held = {}) {
  return {
    type, code, key, repeat: false,
    shiftKey: !!held.shift, altKey: !!held.alt, ctrlKey: !!held.ctrl,
    metaKey: false, getModifierState: () => false,
  };
}

// A key pressed and let go, with Ctrl and Alt as the row of keys holds
// them, which then let go.
function press(code, key, shift = false) {
  const held = { shift, ctrl: page.latched.ctrl, alt: page.latched.alt };
  queue(keyEvent(made("keydown", code, key, held)).bytes);
  queue(keyEvent(made("keyup", code, key, held)).bytes);
  release();
}

function release() {
  page.latched.ctrl = page.latched.alt = false;
  for (const button of document.querySelectorAll("[data-latch]"))
    button.classList.remove("held");
}

// The browser's key event, with what the row of keys holds added.
function withLatched(event) {
  if (!page.latched.ctrl && !page.latched.alt)
    return event;
  return {
    type: event.type, code: event.code, key: event.key,
    repeat: event.repeat, shiftKey: event.shiftKey,
    altKey: event.altKey || page.latched.alt,
    ctrlKey: event.ctrlKey || page.latched.ctrl,
    metaKey: event.metaKey,
    getModifierState: (name) => event.getModifierState(name),
  };
}

function onKey(event) {
  // While the licences are up, no key is the program's, and neither is
  // the letting go of one that went down then: Esc, which closes them.
  if ($("licences").open) {
    if (event.type === "keydown")
      page.unseen.add(event.code);
    else
      page.unseen.delete(event.code);
    return;
  }
  if (event.type === "keyup" && page.unseen.delete(event.code))
    return;
  if (event.target && event.target.closest && event.target.closest("dialog"))
    return;
  const latched = page.latched.ctrl || page.latched.alt;
  const { bytes, prevent } = keyEvent(withLatched(event));
  if (prevent)
    event.preventDefault();
  queue(bytes);
  if (latched && event.type === "keyup" && bytes)
    release();
}

// The text that the phone's keyboard types into the hidden input, made
// keys: what keydown has not taken already.
function onInput(event) {
  const input = event.target;
  const type = event.inputType || "";
  if (type.startsWith("deleteContentBackward")) {
    press("Backspace", "Backspace");
  } else if (type === "insertLineBreak" || type === "insertParagraph") {
    press("Enter", "Enter");
  } else if (event.data) {
    for (const character of event.data) {
      if (character === "\n")
        press("Enter", "Enter");
      else
        press("", character, character !== character.toLowerCase());
    }
  }
  input.value = " ";              // something for Backspace to delete
}

// The grid fitted to what the page has for it: by whole multiples of the
// font's 8 by 16 where there is room for one, else as large as fits, as on
// a phone. With the page zoomed in by the fingers, nothing is changed.
function fit() {
  const viewport = window.visualViewport;
  if (viewport && Math.abs(viewport.scale - 1) > 0.01)
    return;
  const bar = $("bar").getBoundingClientRect().height;
  const width = (viewport ? viewport.width : window.innerWidth) - 8;
  const height = (viewport ? viewport.height : window.innerHeight) - bar - 8;
  let scale = Math.min(width / (COLUMNS * 8), height / (ROWS * 16));
  if (scale >= 1)
    scale = Math.floor(scale);
  scale = Math.max(scale, 0.25);
  // A row of a whole number of the page's pixels: Chromium puts a line of
  // text on a whole pixel of the page's, not of the device's, and rows
  // whose edges fall between two left notches in every upright line of a
  // frame on a phone, at every third row (a headless Chromium showed it).
  const size = Math.max(Math.floor(16 * scale), 4);
  const screen = $("screen");
  screen.style.fontSize = `${size}px`;
  screen.style.lineHeight = `${size}px`;
  screen.style.setProperty("--cell", `${size}px`);
  screen.style.width = `${COLUMNS * size / 2}px`;
  screen.style.height = `${ROWS * size}px`;
  page.cell = { width: size / 2, height: size };
  if (page.screen !== null) {
    page.screen.forget();
  }
}

function notice(text, link = null) {
  const box = $("notice");
  box.textContent = text;
  if (link !== null) {
    box.append(" ", link);
  }
  box.hidden = false;
  clearTimeout(notice.timer);
  notice.timer = setTimeout(() => { box.hidden = true; }, 15000);
}

// A file of the FS in a new tab, as the browser shows a file of its type.
function open(host, name) {
  let bytes;
  try {
    bytes = page.fs.readFile(host);
  } catch {
    notice(`${name} could not be read.`);
    return;
  }
  const dot = name.lastIndexOf(".");
  const extension = dot < 0 ? "" : name.slice(dot + 1).toLowerCase();
  const type = TYPES[extension] || "application/octet-stream";
  const url = URL.createObjectURL(new Blob([bytes], { type }));
  setTimeout(() => URL.revokeObjectURL(url), 10 * 60 * 1000);
  const tab = window.open(url, "_blank");
  if (!tab) {
    // The browser keeps a page from opening a tab but at a click: the
    // user is given one.
    const link = document.createElement("a");
    link.href = url;
    link.target = "_blank";
    link.textContent = `Open ${name}`;
    notice("The browser kept the tab from opening.", link);
  }
}

// The user's files written into DOS's current directory, on the server
// first, and then Ctrl-R, which has the panel read the directory anew.
async function bring(files) {
  if (page.drives === null || page.ended)
    return;
  let brought = 0;
  for (const file of files) {
    if (page.drives.exists(page.where, file.name)
        && !confirm(`${file.name} is there already. Write over it?`))
      continue;
    const bytes = new Uint8Array(await file.arrayBuffer());
    const answer = await page.drives.upload(page.where, file.name, bytes);
    if (answer !== 0) {
      notice(`${file.name} could not be written: ${CHANGE_ERRORS[answer]}.`);
      continue;
    }
    brought++;
  }
  if (brought > 0) {
    queue(keyEvent(made("keydown", "KeyR", "r", { ctrl: true })).bytes);
    queue(keyEvent(made("keyup", "KeyR", "r", { ctrl: true })).bytes);
  }
}

async function fullScreen() {
  try {
    if (document.fullscreenElement) {
      await document.exitFullscreen();
      return;
    }
    await document.documentElement.requestFullscreen();
    if (navigator.keyboard && navigator.keyboard.lock)
      await navigator.keyboard.lock();
  } catch {
    notice("The browser did not give the full screen.");
  }
}

// The licences of what the page carries, as the build put them into it:
// each work, its copyright, where it is from and its text. In a dialog of
// the page's, which Esc and its button close, or, in a browser that has
// no such dialog, as text in a new tab.
function licenceTexts() {
  try {
    return JSON.parse($("licences-data").textContent);
  } catch {
    return [];
  }
}

function showLicences() {
  const dialog = $("licences");
  const all = licenceTexts();
  if (typeof dialog.showModal !== "function") {
    const text = all.map((one) => `${one.work}\n${one.owner}\n` +
      `${one.home ? `${one.home}\n` : ""}\n${one.text}`).join("\n\n");
    const url = URL.createObjectURL(new Blob([text],
                                             { type: "text/plain;charset=utf-8" }));
    setTimeout(() => URL.revokeObjectURL(url), 10 * 60 * 1000);
    if (!window.open(url, "_blank"))
      notice("The browser kept the tab of the licences from opening.");
    return;
  }
  const texts = $("licences-texts");
  if (!texts.dataset.made) {
    for (const one of all) {
      const heading = document.createElement("h2");
      heading.textContent = one.work;
      const owner = document.createElement("p");
      owner.textContent = one.owner;
      if (one.home) {
        const link = document.createElement("a");
        link.href = one.home;
        link.target = "_blank";
        link.rel = "noopener";
        link.textContent = one.home;
        owner.append(", ", link);
      }
      const text = document.createElement("pre");
      text.textContent = one.text;
      texts.append(heading, owner, text);
    }
    texts.dataset.made = "1";
  }
  dialog.showModal();
}

function wireLicences() {
  const dialog = $("licences");
  $("licences-open").addEventListener("click", showLicences);
  $("licences-close").addEventListener("click", () => dialog.close());
  // A click on the backdrop, outside the dialog's box, closes it too.
  dialog.addEventListener("click", (event) => {
    if (event.target === dialog) {
      const box = dialog.getBoundingClientRect();
      if (event.clientX < box.left || event.clientX > box.right
          || event.clientY < box.top || event.clientY > box.bottom)
        dialog.close();
    }
  });
  // The focus goes back to the button, where Enter or Space would open
  // the licences again, not reach the program: it is taken off it.
  dialog.addEventListener("close", () => {
    const button = $("licences-open");
    if (button.blur)
      button.blur();
  });
}

// The row of keys that a phone lacks.
const BAR = [
  ["Esc", "Escape", "Escape"], ["Tab", "Tab", "Tab"],
  ["F1", "F1", "F1"], ["F2", "F2", "F2"], ["F3", "F3", "F3"],
  ["F4", "F4", "F4"], ["F5", "F5", "F5"], ["F6", "F6", "F6"],
  ["F7", "F7", "F7"], ["F8", "F8", "F8"], ["F9", "F9", "F9"],
  ["F10", "F10", "F10"], ["←", "ArrowLeft", "ArrowLeft"],
  ["↑", "ArrowUp", "ArrowUp"], ["↓", "ArrowDown", "ArrowDown"],
  ["→", "ArrowRight", "ArrowRight"], ["PgUp", "PageUp", "PageUp"],
  ["PgDn", "PageDown", "PageDown"],
];

function makeBar() {
  const keys = $("keys");
  for (const [label, code, key] of BAR) {
    const button = document.createElement("button");
    button.textContent = label;
    button.addEventListener("click", () => press(code, key));
    keys.append(button);
  }
  for (const name of ["ctrl", "alt"]) {
    const button = document.createElement("button");
    button.textContent = name === "ctrl" ? "Ctrl" : "Alt";
    button.dataset.latch = name;
    button.addEventListener("click", () => {
      page.latched[name] = !page.latched[name];
      button.classList.toggle("held", page.latched[name]);
    });
    keys.append(button);
  }
  // The phone's keyboard comes up for an input that has the focus, and
  // only when it is given it in the tap itself.
  $("type").addEventListener("click", () => {
    const input = $("input");
    input.value = " ";
    input.focus();
  });
  if (window.matchMedia("(pointer: coarse)").matches)
    $("keys").hidden = false;
  $("more").addEventListener("click", () => {
    $("keys").hidden = !$("keys").hidden;
    fit();
  });
}

function wire() {
  const screen = $("screen");
  const style = document.createElement("style");
  style.textContent = css();
  document.head.append(style);

  const rows = [];
  for (let row = 0; row < ROWS; row++) {
    const span = document.createElement("span");
    rows.push(span);
    screen.append(span);
    if (row < ROWS - 1)
      screen.append("\n");
  }
  const cursor = $("cursor");
  screen.append(cursor);
  Module.vcDraw = (cells, place, shape) => {
    if (page.screen !== null)
      apply(page.screen.draw(cells, place, shape), rows, cursor, page.cell);
  };

  setInterval(() => screen.classList.toggle(BLINK_CLASS), BLINK_HALF);
  setInterval(() => cursor.classList.toggle("off"), CURSOR_HALF);

  const mouse = new Mouse();
  const pointer = (event) => {
    if (event.pointerType === "touch" && event.type === "pointermove"
        && event.buttons === 0)
      return;
    queue(mouse.event(event, screen.getBoundingClientRect()));
    if (event.type === "pointerdown")
      event.preventDefault();
  };
  for (const type of ["pointerdown", "pointerup", "pointermove",
                      "pointercancel"])
    screen.addEventListener(type, pointer);
  screen.addEventListener("contextmenu", (event) => event.preventDefault());

  document.addEventListener("keydown", onKey);
  document.addEventListener("keyup", onKey);
  window.addEventListener("blur", () => queue(FOCUS_OUT));
  $("input").addEventListener("input", onInput);

  $("upload").addEventListener("click", () => $("files").click());
  $("files").addEventListener("change", (event) => {
    bring(Array.from(event.target.files));
    event.target.value = "";
  });
  document.addEventListener("dragover", (event) => event.preventDefault());
  document.addEventListener("drop", (event) => {
    event.preventDefault();
    if (event.dataTransfer && event.dataTransfer.files.length > 0)
      bring(Array.from(event.dataTransfer.files));
  });

  if (document.documentElement.requestFullscreen)
    $("full").addEventListener("click", fullScreen);
  else
    $("full").hidden = true;

  $("reset").addEventListener("click", reset);
  wireLicences();

  makeBar();
  fit();
  window.addEventListener("resize", fit);
  window.addEventListener("orientationchange", () => setTimeout(fit, 200));
  if (window.visualViewport)
    window.visualViewport.addEventListener("resize", fit);
}

// Where the page keeps the user's changes, said in its bar where they are
// kept in the browser, with the button that forgets them.
function kept(mode) {
  if (mode !== "browser")
    return;
  if (page.drives.kept) {
    $("kept").textContent = "Changes are kept in this browser.";
    $("reset").hidden = false;
  } else {
    $("kept").textContent = "This browser gives the page no storage: " +
      "changes last until the page is closed.";
  }
  $("kept").hidden = false;
}

// The drives as they are served, again: what the browser keeps forgotten,
// and the page loaded anew.
async function reset() {
  if (page.mode !== "browser" || !page.drives
      || !confirm("Forget every change kept in this browser, and start " +
                  "again from the drives as they are served?"))
    return;
  page.ended = true;
  try {
    await page.drives.reset();
  } catch {
    page.ended = false;
    notice("The browser did not forget the changes.");
    return;
  }
  window.location.reload();
}

function ended(text) {
  page.ended = true;
  $("ended").textContent = text;
  $("ended").hidden = false;
}

// What the program is given before it starts.
window.Module = {
  vcInput: [],
  vcWhere: (host) => { page.where = host; },
  vcOpen: open,
  vcSync: async () => {},
  vcChange: (what, path, other, fd, mode) =>
    page.drives.change(what, path, other, fd, mode),
  // Emscripten's FS and ENV, before the program starts: the drives
  // loaded into the FS, which the program waits for.
  vcBegin: ({ FS, ENV, addRunDependency, removeRunDependency }) => {
    page.fs = FS;
    ENV.VC_DRIVE_C = ROOTS.c;
    ENV.VC_DRIVE_D = ROOTS.d;
    addRunDependency("vc-drives");
    askMode().then(async (mode) => {
      page.mode = mode;
      page.drives = await MODES[mode](FS);
      await page.drives.load();
      kept(mode);
      removeRunDependency("vc-drives");
    }).catch((error) => {
      ended(`The drives could not be loaded: ${error.message}`);
    });
  },
  onRuntimeInitialized: () => {
    const points = [];
    for (let byte = 0; byte < 256; byte++)
      points.push(Module._CodePageGlyph(byte));
    page.screen = new Screen(points);
  },
  onExit: () => ended("Vibe Commander has ended. Reload to start again."),
  onAbort: (what) => ended(`Vibe Commander stopped: ${what}`),
  print: (text) => console.log(text),
  printErr: (text) => console.error(text),
};

if (document.readyState === "loading")
  document.addEventListener("DOMContentLoaded", wire);
else
  wire();
