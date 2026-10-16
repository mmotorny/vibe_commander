// The test of the web page's program against its server, run by
// harness/webpage.py: see there for what it holds. Node's own modules
// alone. It prints a line for each step that fails, with the screen, and
// exits with 1, or says how many passed.

import { readFileSync, mkdtempSync, readdirSync, existsSync, chmodSync,
         rmSync } from "node:fs";
import path from "node:path";
import vm from "node:vm";

const [web, html, work] = process.argv.slice(2);
const { makeServer, prepare } = await import(path.join(web, "server.mjs"));
const top = mkdtempSync(path.join(work, "webpage-"));
const roots = await prepare({ c: path.join(top, "c"), d: path.join(top, "d"),
                              demo: path.join(web, "demo") });
const server = makeServer(roots, html);
await new Promise((done) => server.listen(0, "127.0.0.1", done));
const base = `http://127.0.0.1:${server.address().port}`;

// A document of nothing but what the page asks of one.
class Element {
  constructor(id) {
    this.id = id;
    this.style = {};
    this.dataset = {};
    this.hidden = false;
    this.textContent = "";
    this.innerHTML = "";
    this.classList = { toggle() {}, add() {}, remove() {} };
  }
  append() {}
  addEventListener() {}
  getBoundingClientRect() { return { left: 0, top: 0, width: 640, height: 400 }; }
  focus() {}
  click() {}
}
const elements = {};
const opened = [];
const window = {
  document: {
    readyState: "complete", head: new Element("head"),
    documentElement: new Element("html"),
    getElementById: (id) => (elements[id] ||= new Element(id)),
    createElement: (tag) => new Element(tag),
    querySelectorAll: () => [], addEventListener() {},
  },
  matchMedia: () => ({ matches: false }), addEventListener() {},
  location: { href: `${base}/` },
  open: (url) => { opened.push(url); return {}; },
  fetch: (url, init) => fetch(new URL(url, base), init),
  console, setTimeout, clearTimeout, setInterval, clearInterval, URL, Blob,
  URLSearchParams, atob, btoa, TextDecoder, TextEncoder, WebAssembly, performance, crypto,
  navigator: { userAgent: "node" }, confirm: () => true,
};
window.window = window.self = window;
// The tab that is opened has the file as a blob: the name of DOS's that
// the page was handed, and the bytes, are what is held.
const createObjectURL = URL.createObjectURL;
window.URL = class extends URL {
  static createObjectURL(blob) {
    opened.blob = blob;
    return createObjectURL(blob);
  }
};
const context = vm.createContext(window);
for (const [, script] of readFileSync(html, "utf8")
       .matchAll(/<script[^>]*>([\s\S]*?)<\/script>/g))
  vm.runInContext(script, context);
const Module = window.Module;
let screen = null;
Module.vcDraw = (cells) => { screen = Array.from(cells); };

function text() {
  if (screen === null)
    return "(nothing drawn)";
  let out = "";
  for (let row = 0; row < 25; row++)
    out += String.fromCharCode(...screen.slice(row * 80, row * 80 + 80)
      .map((cell) => ((cell & 0xFF) >= 32 && (cell & 0xFF) < 127
                      ? cell & 0xFF : 46))) + "\n";
  return out;
}
const sleep = (ms) => new Promise((done) => setTimeout(done, ms));
const keys = (...all) => { for (const one of all) Module.vcInput.push(one); };
const KEY = { F2: "\x1b[Q", F7: "\x1b[18~", F8: "\x1b[19~", F10: "\x1b[21~",
              shiftF4: "\x1b[1;2S", shiftF9: "\x1b[20;2~", home: "\x1b[H",
              end: "\x1b[F", altF2: "\x1b[1;3Q", down: "\x1b[B", esc: "\x1b[27u", enter: "\r" };

let passed = 0, failed = 0;
async function step(name, condition, seconds = 20) {
  const until = Date.now() + seconds * 1000;
  while (Date.now() < until) {
    if (condition()) {
      passed++;
      return true;
    }
    await sleep(50);
  }
  failed++;
  console.log(`FAIL ${name}\n${text()}`);
  return false;
}
const onServer = (...names) => path.join(roots.d, ...names);
// What the screen is to show before the next keys are given: the
// program clears what was typed ahead where a box opens.
const see = (what, seconds) => step(`the screen shows ${what}`,
                                    () => text().includes(what), seconds);

// The program starts at the root of the first drive, C:, where its own
// directory is; then D:, by the drive box of the panel.
await see(" C:\\ ");
await see("VC");
keys(KEY.altF2);
await see("Drive");
keys("d");
await see("readme");
keys(KEY.F7);
await see("Create the directory");
keys("newdir", KEY.enter);
await step("a directory made on the server",
           () => existsSync(onServer("newdir")));
await see("NEWDIR");
keys(KEY.end, KEY.enter);
await step("Enter on a file opens it in a tab", () => opened.length === 1
           && opened.blob && opened.blob.size
              === readFileSync(onServer("readme.txt")).length);
keys(KEY.shiftF4);
await see("Edit the file");
keys("note.txt", KEY.enter);
await see("New-file");
keys(KEY.enter);
await see("2Save");
// One key at a time, as a hand types: the editor clears what is typed
// ahead of it.
for (const [at, character] of [..."hello from the page"].entries()) {
  keys(character);
  await see(`Col ${at + 2} `);
}
keys(KEY.F2);
await step("a file written in the editor reaches the server at its close",
           () => existsSync(onServer("note.txt"))
                 && readFileSync(onServer("note.txt"), "latin1")
                      .startsWith("hello from the page"));
keys(KEY.esc);
await see("8Delete");
keys(KEY.home, KEY.down, KEY.down, KEY.F8);
await see("Do you wish to delete");
keys(KEY.enter);
await step("a directory removed on the server",
           () => !existsSync(onServer("newdir")));
// The server refuses: nothing may be made in D:'s root.
await see("8Delete");
chmodSync(roots.d, 0o555);
keys(KEY.F7);
await see("Create the directory");
keys("locked", KEY.enter);
await step("a refusal of the server is DOS's error, nothing made",
           () => !existsSync(onServer("locked"))
                 && text().includes("t create"));
chmodSync(roots.d, 0o755);
keys(KEY.esc);
await see("8Delete");
keys(KEY.shiftF9);
await see("Save");
keys(KEY.enter);
await step("the setup saved on C: of the server",
           () => existsSync(path.join(roots.c, "vc", "vc.ini")));
await see("8Delete");
keys(KEY.F10);
await see("Do you want to quit");
keys(KEY.enter);
await step("the program ends, and the page says so",
           () => elements.ended && !elements.ended.hidden
                 && elements.ended.textContent.includes("Reload"));

server.close();
rmSync(top, { recursive: true, force: true });
if (failed > 0) {
  console.log(`${failed} of ${passed + failed} steps failed`);
  process.exit(1);
}
console.log(`${passed} steps passed`);
process.exit(0);
