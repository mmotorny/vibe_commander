// A scenario taken through the web page itself, vc.html, in the machine's
// own Chromium, headless (harness/browser.mjs): the program's keys and
// mouse come as a user's do, by the browser's key and pointer events
// through keys.js and mouse.js, and each screen is read from what the
// page shows, twice: from its <pre>, the characters and the classes of
// its spans, and from its pixels, a screenshot of the grid held cell by
// cell to the glyphs of the page's own font. harness/webpagerun.py lays
// the bench out and makes the steps; this runs one scenario:
//
//   node harness/webpagerun.mjs JOB.json OUT.json
//
// JOB has `page` (vc.html), `chromium`, `profile` (a directory of the
// build for Chromium's profile: a snap reads no hidden directory and not
// the real /tmp), `drives` { c, d } (the bench's directories, which the
// page's own server, server.mjs, serves as it serves a user's drives),
// `environment` (the test channel's variables, which the page's test
// hook puts into the program's environment), `keys` (the channel's keys),
// `actions` (for each event of the keys, by its number, what the browser
// is to be given for it, or null), `cwd` (the directory of the page's
// file system that the program starts in), `standIns` (files that the
// page's file system cannot hold, by their paths there and the sizes
// that they have: see webpagerun.py), `limit` (seconds for the whole run)
// and `shots` (a directory where the screenshot of a screen whose pixels
// are not its cells is saved, or null).
//
// The page is the page as a user gets it. What the test adds is the
// page's test hook, window.vcTestHook (page.js), put there before the
// page's scripts run: it is handed the environment and the file system
// before the program starts, and is the page's side of the test channel
// (sys_web.c, VC_TEST_PAGE): the program asks it for each event and
// tells it each screen, and waits each time until this has answered.
//
// OUT has, for each record of the channel in order, the kind and the
// number, the cells as the <pre> has them (`dom`), as the pixels show
// them (`pixels`), the cursor as the page draws it, and what did not
// fit: rows of the <pre> that are not 80 cells, cells whose pixels are
// none of what they should be; and the channel's own records (base64),
// what the program said on its standard error (the console), and how the
// page ended.

import { readFileSync, writeFileSync, mkdirSync } from "node:fs";
import path from "node:path";
import { inflateSync } from "node:zlib";
import { fileURLToPath } from "node:url";
import { Browser, VIEWPORTS, sleep } from "./browser.mjs";

const here = path.dirname(fileURLToPath(import.meta.url));
const { makeServer } = await import(
  path.join(here, "..", "src", "portable_c", "web", "server.mjs"));

const COLUMNS = 80, ROWS = 25, WIDTH = 8, HEIGHT = 16;
const SHADES = new Set([0xB0, 0xB1, 0xB2]);
const RECORD = 4 + 4 + COLUMNS * ROWS * 2 + 8;   // of the channel's screens
const VGA = [
  [0x00, 0x00, 0x00], [0x00, 0x00, 0xaa], [0x00, 0xaa, 0x00], [0x00, 0xaa, 0xaa],
  [0xaa, 0x00, 0x00], [0xaa, 0x00, 0xaa], [0xaa, 0x55, 0x00], [0xaa, 0xaa, 0xaa],
  [0x55, 0x55, 0x55], [0x55, 0x55, 0xff], [0x55, 0xff, 0x55], [0x55, 0xff, 0xff],
  [0xff, 0x55, 0x55], [0xff, 0x55, 0xff], [0xff, 0xff, 0x55], [0xff, 0xff, 0xff],
];
// How far a pixel may be from what it is to be, in any of its colours:
// the font's edges are smoothed by a little.
const TOLERANCE = 48;

// The hook, as a script that runs in the page before its own: `config`
// is put in as JSON.
function hookSource(config) {
  return `(() => {
  const config = ${JSON.stringify(config)};
  let fs = null;
  const pending = [];             // requests that the harness has not taken
  let taken = null;               // the one that it has, with its resolve
  let waiter = null;              // the harness waiting for a request
  let points = null;              // code point -> byte, of the page's glyphs
  function wake() {
    if (waiter && pending.length > 0) {
      const resolve = waiter;
      waiter = null;
      taken = pending.shift();
      resolve(taken.request);
    }
  }
  function ask(request) {
    // The program has taken where it starts from the file system's
    // current directory (loaded, below), and that is the root again, as
    // on a user's page: Emscripten's file system does not rename or
    // remove its current directory, where a host's does.
    if (fs.cwd() !== "/")
      fs.chdir("/");
    return new Promise((resolve) => {
      pending.push({ request, resolve });
      wake();
    });
  }
  function glyphs() {
    if (points === null) {
      points = new Map();
      for (let byte = 0; byte < 256; byte++)
        points.set(Module._CodePageGlyph(byte), byte);
    }
    return points;
  }
  // The cells of the screen that the channel has just recorded, the last
  // record of its file (machine_posix.c, Record).
  function recorded() {
    const bytes = fs.readFile("/test/screens");
    const at = bytes.length - ${RECORD} + 8;
    const cells = [];
    for (let i = 0; i < ${COLUMNS * ROWS}; i++)
      cells.push(bytes[at + 2 * i] | bytes[at + 2 * i + 1] << 8);
    return cells;
  }
  // The cells as the <pre> has them: each row a span of spans, each of
  // the class of its attribute, with the glyphs of its characters. Some
  // bytes have one glyph, a blank or another: a glyph that the byte of
  // the recorded cell has too is read as that byte.
  function readDom() {
    const map = glyphs();
    const truth = recorded();
    const screen = document.getElementById("screen");
    const rows = Array.from(screen.children).filter((e) => e.tagName === "SPAN");
    const cells = [], wrong = [];
    if (rows.length !== ${ROWS})
      wrong.push("the <pre> has " + rows.length + " rows");
    rows.forEach((row, at) => {
      const line = [];
      for (const run of row.children) {
        const found = /vc-a([0-9a-f]{2})/.exec(run.className);
        const attribute = found ? parseInt(found[1], 16) : -1;
        for (const character of run.textContent) {
          const byte = map.get(character.codePointAt(0));
          const want = truth[cells.length + line.length];
          const same = byte !== undefined && (want & 0xFF) !== byte
            && Module._CodePageGlyph(want & 0xFF) === character.codePointAt(0);
          line.push(byte === undefined || attribute < 0 ? -1
                    : (attribute << 8 | (same ? want & 0xFF : byte)));
        }
      }
      if (line.length !== ${COLUMNS})
        wrong.push("row " + at + " of the <pre> has " + line.length + " cells");
      for (let column = 0; column < ${COLUMNS}; column++)
        cells.push(column < line.length ? line[column] : -1);
    });
    const cursor = document.getElementById("cursor");
    let drawn = null;
    if (cursor && cursor.style.display !== "none" && cursor.style.display !== "") {
      const box = screen.getBoundingClientRect();
      const cell = box.width / ${COLUMNS}, high = box.height / ${ROWS};
      const left = parseFloat(cursor.style.left), top = parseFloat(cursor.style.top);
      const height = parseFloat(cursor.style.height);
      drawn = { column: Math.round(left / cell),
                row: Math.round((top + height) / high) - 1,
                block: height > high / 2 };
    }
    return { cells, wrong, cursor: drawn };
  }
  window.vcTestHook = {
    begin({ FS, ENV }) {
      fs = FS;
      Object.assign(ENV, config.environment);
      FS.mkdir("/test");
      FS.writeFile("/test/keys", config.keys);
    },
    async loaded({ FS }) {
      // The page has one drive, C:, and makes C:\VC for the program's
      // own files where it is not there; the bench has those in D:\VC,
      // and no C:\VC, which a page made is taken away again with the
      // root's time as the bench has it. D: is the bench's, written into
      // the file system from the job: its changes stay there, where no
      // server is asked (drives.js, locate), and are read at the end.
      FS.chdir(config.cwd);
      if (!config.cHasVc) {
        try {
          FS.rmdir("/drive/c/vc");
        } catch {
          // not there
        }
        FS.utime("/drive/c", config.cTime, config.cTime);
      }
      FS.mkdirTree(config.d.root);
      for (const entry of config.d.entries) {
        const at = config.d.root + "/" + entry.path;
        if (entry.directory)
          FS.mkdirTree(at);
        else
          FS.writeFile(at, Uint8Array.from(atob(entry.data), (c) => c.charCodeAt(0)));
        if (entry.readonly)
          FS.chmod(at, 0o444);
      }
      for (const entry of [...config.d.entries].reverse())
        FS.utime(config.d.root + "/" + entry.path, entry.mtime, entry.mtime);
      FS.utime(config.d.root, config.d.mtime, config.d.mtime);
      // The files that the page's file system cannot hold: as long as
      // the bench has them, and read as the zeros of a sparse file.
      for (const [where, size] of config.standIns) {
        const node = FS.lookupPath(where).node;
        node.usedBytes = size;
        node.contents = new Uint8Array(0);
        node.stream_ops = { ...node.stream_ops,
          read(stream, buffer, offset, length, position) {
            const count = Math.max(0, Math.min(size - position, length));
            buffer.fill(0, offset, offset + count);
            return count;
          } };
      }
    },
    event(number) {
      return ask({ type: "event", number });
    },
    screen(kind, number) {
      return ask({ type: "screen", kind, number, ...readDom() });
    },
    // For the harness: the next request, or the page's end.
    next() {
      return new Promise((resolve) => {
        const ended = document.getElementById("ended");
        const look = setInterval(() => {
          if (ended && !ended.hidden && pending.length === 0) {
            clearInterval(look);
            waiter = null;
            resolve({ type: "ended", text: ended.textContent });
          }
        }, 50);
        waiter = (request) => { clearInterval(look); resolve(request); };
        wake();
      });
    },
    done() {
      const resolve = taken.resolve;
      taken = null;
      resolve();
    },
    // D: as it is at the end: each file and directory, parents first,
    // with its time, whether it may be written, and a file's bytes.
    driveD() {
      const out = [];
      const walk = (directory, prefix) => {
        for (const name of fs.readdir(directory).sort()) {
          if (name === "." || name === "..")
            continue;
          const at = directory + "/" + name;
          const status = fs.stat(at);
          const path = prefix === "" ? name : prefix + "/" + name;
          const directory_ = fs.isDir(status.mode);
          const entry = { path, directory: directory_,
                          mtime: +status.mtime,
                          readonly: (status.mode & 0o200) === 0 };
          if (!directory_) {
            const bytes = fs.readFile(at);
            let text = "";
            for (let i = 0; i < bytes.length; i += 8192)
              text += String.fromCharCode(...bytes.subarray(i, i + 8192));
            entry.data = btoa(text);
          }
          out.push(entry);
          if (directory_)
            walk(at, path);
        }
      };
      walk(config.d.root, "");
      return { entries: out, mtime: +fs.stat(config.d.root).mtime };
    },
    records() {
      try {
        const bytes = fs.readFile("/test/screens");
        let text = "";
        for (let at = 0; at < bytes.length; at += 8192)
          text += String.fromCharCode(...bytes.subarray(at, at + 8192));
        return btoa(text);
      } catch {
        return "";
      }
    },
  };
})();`;
}

// A PNG of Chromium's: 8 bits a sample, RGB or RGBA, not interlaced.
function decodePng(buffer) {
  let at = 8, width = 0, height = 0, type = 0;
  const data = [];
  while (at < buffer.length) {
    const length = buffer.readUInt32BE(at);
    const kind = buffer.toString("ascii", at + 4, at + 8);
    const body = buffer.subarray(at + 8, at + 8 + length);
    if (kind === "IHDR") {
      width = body.readUInt32BE(0);
      height = body.readUInt32BE(4);
      if (body[8] !== 8 || body[12] !== 0)
        throw new Error("a PNG that is not of 8 bits or is interlaced");
      type = body[9];
    } else if (kind === "IDAT") {
      data.push(body);
    }
    at += 12 + length;
  }
  const channels = type === 6 ? 4 : type === 2 ? 3 : 0;
  if (channels === 0)
    throw new Error(`a PNG of the colour type ${type}`);
  const raw = inflateSync(Buffer.concat(data));
  const stride = width * channels;
  const out = new Uint8Array(width * height * 3);
  const line = new Uint8Array(stride), before = new Uint8Array(stride);
  for (let y = 0; y < height; y++) {
    const filter = raw[y * (stride + 1)];
    const row = raw.subarray(y * (stride + 1) + 1, (y + 1) * (stride + 1));
    for (let x = 0; x < stride; x++) {
      const a = x >= channels ? line[x - channels] : 0;
      const b = before[x];
      const c = x >= channels ? before[x - channels] : 0;
      let value = row[x];
      if (filter === 1) value += a;
      else if (filter === 2) value += b;
      else if (filter === 3) value += (a + b) >> 1;
      else if (filter === 4) {
        const p = a + b - c, pa = Math.abs(p - a), pb = Math.abs(p - b),
              pc = Math.abs(p - c);
        value += pa <= pb && pa <= pc ? a : pb <= pc ? b : c;
      }
      line[x] = value & 0xFF;
    }
    for (let x = 0; x < width; x++)
      for (let k = 0; k < 3; k++)
        out[(y * width + x) * 3 + k] = line[x * channels + k];
    before.set(line);
  }
  return { width, height, pixels: out };
}

async function shot(tab, clip) {
  const { data } = await tab.send("Page.captureScreenshot",
    { format: "png", clip: { ...clip, scale: 1 }, captureBeyondViewport: false });
  return Buffer.from(data, "base64");
}

// The glyphs of the page's font as the page draws them, white on black,
// each 8 by 16 pixels: how much of the character's colour each pixel has,
// from 0 to 1, 128 a glyph.
async function atlas(tab, shots, clip) {
  // Drawn in the page's own <pre>, in its spans of the class of white on
  // black, eight rows of 32 glyphs, so that the font is drawn as the page
  // draws it: the rows as they were put back after.
  await tab.eval(`(() => {
    const screen = document.getElementById("screen");
    const rows = Array.from(screen.children).filter((e) => e.tagName === "SPAN");
    window.vcTestSaved = rows.map((row) => row.innerHTML);
    rows.forEach((row, at) => {
      row.innerHTML = "";
      for (let column = 0; column < 80; column++) {
        const run = document.createElement("span");
        run.className = "vc-a0f";
        const byte = at * 32 + column;
        run.textContent = at < 8 && column < 32
          ? String.fromCodePoint(Module._CodePageGlyph(byte)) : " ";
        row.append(run);
      }
    });
  })()`);
  await sleep(50);
  const png = await shot(tab, clip);
  const image = decodePng(png);
  await tab.eval(`(() => {
    const screen = document.getElementById("screen");
    Array.from(screen.children).filter((e) => e.tagName === "SPAN")
      .forEach((row, at) => { row.innerHTML = window.vcTestSaved[at]; });
  })()`);
  if (shots) {
    mkdirSync(shots, { recursive: true });
    writeFileSync(path.join(shots, "atlas.png"), png);
  }
  const alpha = new Float32Array(256 * WIDTH * HEIGHT);
  for (let byte = 0; byte < 256; byte++) {
    const x0 = (byte % 32) * WIDTH, y0 = Math.floor(byte / 32) * HEIGHT;
    for (let y = 0; y < HEIGHT; y++)
      for (let x = 0; x < WIDTH; x++) {
        const at = ((y0 + y) * image.width + x0 + x) * 3;
        alpha[byte * 128 + y * WIDTH + x] = (image.pixels[at] + image.pixels[at + 1]
                                             + image.pixels[at + 2]) / 765;
      }
  }
  return alpha;
}

// The worst difference of the cell at `column`, `row` of `image` from
// the glyph `byte` in the colours `fg` and `bg`, where `cursor` is null
// or the lines from which the cursor's overlay is drawn over the cell in
// the difference with #aaa.
function cellOff(image, alpha, column, row, byte, fg, bg, cursor) {
  let worst = 0;
  const f = VGA[fg], b = VGA[bg];
  for (let y = 0; y < HEIGHT; y++)
    for (let x = 0; x < WIDTH; x++) {
      const a = alpha[byte * 128 + y * WIDTH + x];
      const at = ((row * HEIGHT + y) * image.width + column * WIDTH + x) * 3;
      for (let k = 0; k < 3; k++) {
        let want = b[k] + (f[k] - b[k]) * a;
        if (cursor !== null && y >= cursor)
          want = Math.abs(want - 0xaa);
        const off = Math.abs(image.pixels[at + k] - want);
        if (off > worst)
          worst = off;
      }
    }
  return worst;
}

// The nearest of the VGA's colours.
function nearest(r, g, b) {
  let best = 0, distance = Infinity;
  VGA.forEach(([vr, vg, vb], index) => {
    const d = (r - vr) ** 2 + (g - vg) ** 2 + (b - vb) ** 2;
    if (d < distance) {
      distance = d;
      best = index;
    }
  });
  return best;
}

// The cell as its pixels show it, where they show no cell that it was
// to be: its two colours, the one of most pixels the background, and the
// glyph nearest to its pixels in them.
function decodeCell(image, alpha, column, row) {
  const counts = new Map();
  for (let y = 0; y < HEIGHT; y++)
    for (let x = 0; x < WIDTH; x++) {
      const at = ((row * HEIGHT + y) * image.width + column * WIDTH + x) * 3;
      const c = nearest(image.pixels[at], image.pixels[at + 1], image.pixels[at + 2]);
      counts.set(c, (counts.get(c) || 0) + 1);
    }
  const sorted = [...counts.entries()].sort((p, q) => q[1] - p[1]);
  const bg = sorted[0][0], fg = sorted.length > 1 ? sorted[1][0] : bg;
  let best = 0x20, off = Infinity;
  for (let byte = 0; byte < 256; byte++) {
    const now = cellOff(image, alpha, column, row, byte, fg, bg, null);
    if (now < off) {
      off = now;
      best = byte;
    }
  }
  return { cell: ((bg & 7) << 4 | fg) << 8 | best, off };
}

// The cursor's first line of the overlay in a cell, as render.js draws
// it: a block the whole cell, a line its lowest two.
function cursorLine(cursor, column, row) {
  if (cursor === null || cursor.row !== row || cursor.column !== column)
    return null;
  return cursor.block ? 0 : HEIGHT - 2;
}

// The cells as the pixels show them, held to `expected`, the cells that
// the program drew: each cell whose pixels are its expected glyph in its
// colours (or, blinking, its background alone; under the cursor, with
// the overlay or without) is that cell; any other is what its pixels
// show, and is counted.
function pixelCells(image, alpha, expected, cursor) {
  const cells = [], off = [];
  for (let row = 0; row < ROWS; row++)
    for (let column = 0; column < COLUMNS; column++) {
      const want = expected[row * COLUMNS + column];
      const byte = want & 0xFF, attribute = want >> 8;
      const fg = attribute & 0x0F, bg = attribute >> 4 & 0x07;
      const line = cursorLine(cursor, column, row);
      const tries = [[byte, fg]];
      if (attribute & 0x80)
        tries.push([0x20, bg]);
      let least = Infinity;
      for (const [b, f] of tries) {
        least = Math.min(least, cellOff(image, alpha, column, row, b, f, bg, null));
        if (line !== null)
          least = Math.min(least, cellOff(image, alpha, column, row, b, f, bg, line));
      }
      if (least <= TOLERANCE) {
        cells.push(want);
        continue;
      }
      const found = decodeCell(image, alpha, column, row);
      cells.push(found.cell);
      off.push([row, column, want, found.cell, Math.round(least)]);
    }
  return { cells, off };
}

// On a phone the grid is at a scale that is no whole multiple of the
// font's, in device pixels that are no whole multiple of a cell, and no
// atlas of glyphs is exact there. What is held instead: that the cells
// lie on a lattice, the grid being 80 by 25 cells of one size, and that
// every pixel well inside a cell is of the cell's colours, its
// background's or its character's or a blend of the two (blinking, its
// background's alone), the most of them one of those two but in a shade.
function latticeCells(image, expected, cursor) {
  const off = [];
  const wide = image.width / COLUMNS, high = image.height / ROWS;
  for (let row = 0; row < ROWS; row++)
    for (let column = 0; column < COLUMNS; column++) {
      if (cursor !== null && cursor.row === row && cursor.column === column)
        continue;
      const want = expected[row * COLUMNS + column];
      const attribute = want >> 8;
      const f = VGA[attribute & 0x0F], b = VGA[attribute >> 4 & 0x07];
      let worst = 0, ofB = 0, ofF = 0, all = 0;
      for (let y = Math.ceil(row * high) + 1; y < Math.floor((row + 1) * high) - 1; y++)
        for (let x = Math.ceil(column * wide) + 1;
             x < Math.floor((column + 1) * wide) - 1; x++) {
          const at = (y * image.width + x) * 3;
          const p = [image.pixels[at], image.pixels[at + 1], image.pixels[at + 2]];
          // The distance from the segment between the two colours.
          const d = [0, 1, 2].map((k) => f[k] - b[k]);
          const length = d[0] ** 2 + d[1] ** 2 + d[2] ** 2;
          let t = length === 0 ? 0
            : ((p[0] - b[0]) * d[0] + (p[1] - b[1]) * d[1] + (p[2] - b[2]) * d[2]) / length;
          t = Math.max(0, Math.min(1, t));
          const away = Math.max(...[0, 1, 2].map((k) => Math.abs(p[k] - (b[k] + t * d[k]))));
          worst = Math.max(worst, away);
          all++;
          if (Math.max(...[0, 1, 2].map((k) => Math.abs(p[k] - b[k]))) <= TOLERANCE)
            ofB++;
          else if (Math.max(...[0, 1, 2].map((k) => Math.abs(p[k] - f[k]))) <= TOLERANCE)
            ofF++;
        }
      // A glyph of a pattern, the shades of B0h to B2h, is a blend of
      // the two colours wherever it is scaled down, so that the most of
      // its pixels may be neither.
      if (worst > TOLERANCE || (all > 0 && ofB + ofF < all / 2
                                && !SHADES.has(want & 0xFF)))
        off.push([row, column, want, want, Math.round(worst)]);
    }
  return off;
}

const [jobFile, outFile] = process.argv.slice(2);
const job = JSON.parse(readFileSync(jobFile, "utf8"));
// The program's time zone is the browser's: the bench's is UTC.
process.env.TZ = "UTC";

const out = { screens: [], said: [], ended: null, records: "", error: null };
const server = makeServer({ c: job.drives.c }, job.page);
await new Promise((done) => server.listen(0, "127.0.0.1", done));
const sockets = new Set();
server.on("connection", (socket) => {
  sockets.add(socket);
  socket.on("close", () => sockets.delete(socket));
});
let browser = null, tab = null, last = "nothing yet";
const deadline = Date.now() + job.limit * 1000;
try {
  // Text drawn in grey levels, not in the colours of a screen's
  // subpixels, which Chromium chooses by what is composited where, so
  // that a glyph is the same pixels wherever it is drawn.
  browser = await Browser.start(job.chromium, job.profile,
                                ["--disable-lcd-text"]);
  const view = VIEWPORTS[job.viewport || "desktop"];
  const desktop = !view.mobile;
  tab = await browser.page(view);
  await tab.send("Page.addScriptToEvaluateOnNewDocument", { source: hookSource({
    environment: job.environment, keys: job.keys, standIns: job.standIns,
    d: job.d, cHasVc: job.cHasVc, cTime: job.cTime,
    cwd: job.cwd }) });
  await tab.go(`http://127.0.0.1:${server.address().port}/`);
  for (;;) {
    if (Date.now() > deadline)
      throw new Error("the page did not load");
    try {
      if (await tab.eval("location.protocol === 'http:' && " +
                         "typeof window.vcTestHook === 'object'"))
        break;
    } catch {
      // the document is being replaced
    }
    await sleep(50);
  }
  let alpha = null, clip = null, buttons = 0, mouseAt = null;
  const mods = { bits: 0 };
  for (;;) {
    if (Date.now() > deadline)
      throw new Error(`the run took more than ${job.limit} s`);
    const request = await tab.eval("vcTestHook.next()");
    last = request.type === "event" ? `the event ${request.number}`
      : `the screen ${request.kind}${request.number}`;
    if (request.type === "ended") {
      out.ended = request.text;
      break;
    }
    if (alpha === null) {
      clip = await tab.rect("#screen");
      clip = { x: clip.left, y: clip.top, width: clip.width, height: clip.height };
      if (desktop) {
        alpha = await atlas(tab, job.shots, clip);
        if (clip.width !== COLUMNS * WIDTH || clip.height !== ROWS * HEIGHT)
          throw new Error(`the grid is ${clip.width} by ${clip.height} pixels`);
      } else {
        alpha = true;
        // A cell of a whole number of the page's pixels, twice as high as
        // wide (page.js, fit).
        if (clip.height % ROWS !== 0 || clip.width * 2 / COLUMNS !== clip.height / ROWS)
          throw new Error(`the grid is ${clip.width} by ${clip.height} pixels`);
      }
    }
    if (request.type === "event") {
      const action = job.actions[request.number];
      if (action === null || action === undefined)
        throw new Error(`the program asked for the event ${request.number}, ` +
                        "which the browser has nothing for");
      if (action.key) {
        const key = action.key;
        const params = { modifiers: action.mods, code: key.code, key: key.key,
                         windowsVirtualKeyCode: key.virtual,
                         nativeVirtualKeyCode: key.virtual };
        if (key.location)
          params.location = key.location;
        await tab.send("Input.dispatchKeyEvent", key.text
          ? { ...params, type: "keyDown", text: key.text, unmodifiedText: key.text }
          : { ...params, type: "rawKeyDown" });
        await tab.send("Input.dispatchKeyEvent", { ...params, type: "keyUp" });
      } else if (action.modifier) {
        const m = action.modifier;
        await tab.send("Input.dispatchKeyEvent", {
          type: action.down ? "rawKeyDown" : "keyUp", modifiers: action.mods,
          code: m.code, key: m.key, windowsVirtualKeyCode: m.virtual,
          nativeVirtualKeyCode: m.virtual, location: m.location });
      } else if (action.mouse) {
        const [column, row, want] = action.mouse;
        const x = clip.x + (column + 0.5) * clip.width / COLUMNS;
        const y = clip.y + (row + 0.5) * clip.height / ROWS;
        const names = [[1, "left"], [2, "right"], [4, "middle"]];
        if (mouseAt === null || mouseAt[0] !== x || mouseAt[1] !== y)
          await tab.send("Input.dispatchMouseEvent", { type: "mouseMoved", x, y,
            buttons, modifiers: action.mods,
            button: names.find(([bit]) => buttons & bit)?.[1] || "none" });
        mouseAt = [x, y];
        for (const [bit, name] of names) {
          if ((want & bit) && !(buttons & bit)) {
            buttons |= bit;
            await tab.send("Input.dispatchMouseEvent", { type: "mousePressed",
              x, y, button: name, buttons, clickCount: 1, modifiers: action.mods });
          } else if (!(want & bit) && (buttons & bit)) {
            buttons &= ~bit;
            await tab.send("Input.dispatchMouseEvent", { type: "mouseReleased",
              x, y, button: name, buttons, clickCount: 1, modifiers: action.mods });
          }
        }
      }
      await tab.eval("vcTestHook.done()");
      continue;
    }
    // A screen: what the <pre> has was read when it was told; now its
    // pixels.
    const png = await shot(tab, clip);
    const image = decodePng(png);
    const dom = request.cells;
    const pixels = desktop
      ? pixelCells(image, alpha, dom.map((c) => (c < 0 ? 0 : c)), request.cursor)
      : { cells: dom, off: latticeCells(image, dom.map((c) => (c < 0 ? 0 : c)),
                                        request.cursor) };
    if (pixels.off.length > 0 && job.shots) {
      mkdirSync(job.shots, { recursive: true });
      writeFileSync(path.join(job.shots,
        `${request.kind.trim()}-${request.number}.png`), png);
    }
    out.screens.push({ kind: request.kind, number: request.number, dom,
                       wrong: request.wrong, cursor: request.cursor,
                       pixels: pixels.cells, off: pixels.off.slice(0, 50),
                       offCount: pixels.off.length });
    await tab.eval("vcTestHook.done()");
  }
  out.records = await tab.eval("vcTestHook.records()");
  out.d = await tab.eval("vcTestHook.driveD()");
} catch (caught) {
  out.error = `${caught && caught.message || caught}, at ${last}`;
} finally {
  if (tab !== null)
    out.said = tab.console.filter((m) => m.level === "error"
                                         || m.level === "exception")
      .map((m) => m.text);
  if (browser !== null)
    await browser.close();
  for (const socket of sockets)
    socket.destroy();
  server.close();
}
writeFileSync(outFile, JSON.stringify(out));
