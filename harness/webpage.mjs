// The test of the web page's program as its static site serves it, run
// by harness/webpage.py: see there for what it holds. Node's own modules
// alone. It prints a line for each step that fails, with the screen, and
// exits with 1, or says how many passed.

import { readFileSync, mkdtempSync, mkdirSync, readdirSync, rmSync,
         statSync } from "node:fs";
import path from "node:path";
import vm from "node:vm";
import { createServer } from "node:http";

const [web, html, work] = process.argv.slice(2);
const { makeSite } = await import(path.join(web, "static.mjs"));
const { serve } = await import(path.join(web, "serve.mjs"));
const { layFiles } = await import(new URL("./webfiles.mjs", import.meta.url));
const top = mkdtempSync(path.join(work, "webpage-"));

// The site as the build has made it to be shipped, beside the page, and
// one whose C: has the test's own files beside the program's directory
// vc, each served as a host of files serves it.
const shipped = path.join(path.dirname(html), "static");
const own = path.join(top, "c");
mkdirSync(path.join(own, "vc"), { recursive: true });
layFiles(own);
const site = path.join(top, "site");
await makeSite({ page: html, out: site, drive: own });
// Every file of a directory, with its bytes and its time: what the run
// is to leave as it was.
function listing(root) {
  const out = [];
  const walk = (at) => {
    for (const name of readdirSync(at).sort()) {
      const file = path.join(at, name);
      const status = statSync(file);
      out.push(`${path.relative(root, file)} ${status.mtimeMs} ${status.mode}`
               + (status.isFile() ? ` ${readFileSync(file).toString("base64")}` : ""));
      if (status.isDirectory())
        walk(file);
    }
  };
  walk(root);
  return out.join("\n");
}
const before = listing(top);
async function hostOf(root) {
  const host = serve(root);
  await new Promise((done) => host.listen(0, "127.0.0.1", done));
  return host;
}
const host = await hostOf(site);
const base = `http://127.0.0.1:${host.address().port}`;

let passed = 0, failed = 0;
const sleep = (ms) => new Promise((done) => setTimeout(done, ms));
const KEY = { F2: "\x1b[Q", F7: "\x1b[18~", F8: "\x1b[19~", F10: "\x1b[21~",
              shiftF4: "\x1b[1;2S", altF1: "\x1b[1;3P", shiftF9: "\x1b[20;2~", home: "\x1b[H",
              end: "\x1b[F", altF2: "\x1b[1;3Q", down: "\x1b[B", esc: "\x1b[27u", enter: "\r" };

// The page loaded from `base`, its scripts run as the browser would, with a
// document of nothing but what the page asks of one, and a window whose
// stores of the browser's say whether the page has asked for one.
function load(base) {
  class Element {
    constructor(id) {
      this.id = id;
      this.style = { setProperty(name, value) { this[name] = value; } };
      this.dataset = {};
      // Hidden as the shell has them at the start.
      this.hidden = ["keys", "notice", "ended"].includes(id);
      this.textContent = "";
      this.innerHTML = "";
      this.listeners = {};
      this.classList = { toggle() {}, add() {}, remove() {} };
      this.children = [];
      this.open = false;
    }
    append(...items) { this.children.push(...items); }
    addEventListener(type, listener) { this.listeners[type] = listener; }
    // A dialog, as the licences are shown in.
    showModal() { this.open = true; }
    close() {
      this.open = false;
      if (this.listeners.close)
        this.listeners.close({ target: this });
    }
    blur() {}
    getBoundingClientRect() { return { left: 0, top: 0, width: 640, height: 400 }; }
    focus() {}
    click() {}
  }
  const elements = {};
  const opened = [];
  const listeners = {};
  const page = { elements, opened, listeners, reloads: 0, fs: null,
                 screen: null, stored: [] };
  // The browser's stores, each use of which is noted.
  const store = (name) => new Proxy({}, {
    get: (_, key) => { page.stored.push(`${name}.${String(key)}`); return () => null; },
  });
  const window = {
    document: {
      readyState: "complete", head: new Element("head"),
      documentElement: new Element("html"),
      getElementById: (id) => (elements[id] ||= new Element(id)),
      createElement: (tag) => new Element(tag),
      querySelectorAll: () => [],
      addEventListener: (type, listener) => { listeners[type] = listener; },
    },
    matchMedia: () => ({ matches: false }), addEventListener() {},
    location: { href: `${base}/`, reload: () => { page.reloads++; } },
    open: (url) => { opened.push(url); return {}; },
    fetch: (url, init) => fetch(new URL(url, `${base}/`), init),
    console, setTimeout, clearTimeout, setInterval, clearInterval, URL, Blob,
    URLSearchParams, atob, btoa, TextDecoder, TextEncoder, WebAssembly, performance, crypto,
    navigator: { userAgent: "node", storage: store("navigator.storage") },
    confirm: () => true, indexedDB: store("indexedDB"),
    localStorage: store("localStorage"), sessionStorage: store("sessionStorage"),
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
  // The scripts run, and the data of the licences is the text of its
  // element, as a browser has them.
  for (const [, type, script] of readFileSync(html, "utf8")
         .matchAll(/<script([^>]*)>([\s\S]*?)<\/script>/g)) {
    // Emscripten minifies the shell: an attribute may have no quotes.
    if (/\bid="?licences-data\b/.test(type))
      window.document.getElementById("licences-data").textContent = script;
    else
      vm.runInContext(script, context);
  }
  const Module = window.Module;
  page.Module = Module;
  Module.vcDraw = (cells) => { page.screen = Array.from(cells); };
  // The page's file system, for the test to look into.
  const begin = Module.vcBegin;
  Module.vcBegin = (given) => { page.fs = given.FS; begin(given); };

  page.text = () => {
    if (page.screen === null)
      return "(nothing drawn)";
    let out = "";
    for (let row = 0; row < 25; row++)
      out += String.fromCharCode(...page.screen.slice(row * 80, row * 80 + 80)
        .map((cell) => ((cell & 0xFF) >= 32 && (cell & 0xFF) < 127
                        ? cell & 0xFF : 46))) + "\n";
    return out;
  };
  page.keys = (...all) => { for (const one of all) Module.vcInput.push(one); };
  page.step = async (name, condition, seconds = 20) => {
    const until = Date.now() + seconds * 1000;
    while (Date.now() < until) {
      if (condition()) {
        passed++;
        return true;
      }
      await sleep(50);
    }
    failed++;
    console.log(`FAIL ${name}\n${page.text()}`);
    return false;
  };
  // What the screen is to show before the next keys are given: the
  // program clears what was typed ahead where a box opens.
  page.see = (what, seconds) => page.step(`the screen shows ${what}`,
                                          () => page.text().includes(what),
                                          seconds);
  return page;
}

const isDirectory = (page, at) => {
  try {
    return page.fs.isDir(page.fs.stat(at).mode);
  } catch {
    return false;
  }
};
const inPage = (page, at) => {
  try {
    return Buffer.from(page.fs.readFile(at)).toString("latin1");
  } catch {
    return null;
  }
};

// 1. The page as its site serves it, its C: with the test's files.
const page0 = load(base);
const { elements, opened, keys, step, see, text } = page0;
const inPage0 = (...names) => inPage(page0, ["/drive/c", ...names].join("/"));

// The program starts in C:\VC, where its own files are, on the page's
// one drive; then goes up to C:\, where the test's files are.
await see(" C:\\VC ");

// The licences: one link in the bar, which shows the texts of the
// repository's files that the page carries, the project's own, Volkov
// Commander's and DOS/32A's, which the program for DOS in C:\VC has
// linked into it, among them; Esc closes them, and neither Esc nor any key while they are up reaches
// the program.
{
  const repository = path.resolve(web, "../../..");
  const texts = ["LICENSE", "src/asm/LICENSE.TXT", "LICENSE.DOS32A",
                 "LICENSE.EMSCRIPTEN", "LICENSE.MUSL", "LICENSE.FONT"]
    .map((name) => readFileSync(path.join(repository, name), "utf8")
      .replace(/\r\n/g, "\n"));
  const shell = readFileSync(html, "utf8");
  await step("the bar has one link to the licences, and no credit apart",
             () => /<button [^>]*\bid="?licences-open\b[^>]*>Licences<\/button>/.test(shell)
                   && !shell.includes("CC BY-SA 4.0</a>"));
  await step("the bar says that changes last until the page is reloaded or closed",
             () => /<span [^>]*\bid="?kept\b[^>]*>Changes last until the page is reloaded or closed\.<\/span>/
                     .test(shell));
  const sent = [];
  const push = page0.Module.vcInput.push;
  page0.Module.vcInput.push = (...bytes) => sent.push(...bytes);
  const key = (type, code, key) => ({
    type, code, key, repeat: false, shiftKey: false, altKey: false,
    ctrlKey: false, metaKey: false, getModifierState: () => false,
    preventDefault() {}, target: { closest: () => null },
  });
  elements["licences-open"].listeners.click();
  const shown = elements["licences-texts"].children
    .filter((child) => child.id === "pre").map((child) => child.textContent);
  await step("the licences show every text that the page carries",
             () => elements.licences.open && shown.length === texts.length
                   && texts.every((text, at) => shown[at] === text));
  await step("the licences say what DOS/32A's asks them to say",
             () => elements["licences-texts"].children.some((child) =>
               child.textContent.includes(
                 "This product uses DOS/32 Advanced DOS Extender technology.")
               && child.id !== "pre"));
  page0.listeners.keydown(key("keydown", "KeyA", "a"));
  page0.listeners.keyup(key("keyup", "KeyA", "a"));
  page0.listeners.keydown(key("keydown", "Escape", "Escape"));
  elements.licences.close();      // what the browser does with Esc
  page0.listeners.keyup(key("keyup", "Escape", "Escape"));
  await step("Esc closes the licences, and the program sees no key of it",
             () => !elements.licences.open && sent.length === 0);
  elements["licences-open"].listeners.click();
  elements["licences-close"].listeners.click();
  page0.listeners.keydown(key("keydown", "KeyA", "a"));
  page0.listeners.keyup(key("keyup", "KeyA", "a"));
  await step("the button closes them, and the keys are the program's again",
             () => !elements.licences.open && sent.length > 0
                   && elements["licences-texts"].children.length
                      === 3 * texts.length);
  page0.Module.vcInput.push = push;
}
keys(KEY.altF2);
await see("Drive");
await step("the drive box has A: and C:", () => {
  const lines = text().split("\n");
  const at = lines.findIndex((line) => line.includes("Choose"));
  // The letters between the box's sides, under its question.
  const left = at >= 0 ? lines[at].indexOf(". Choose") : -1;
  const right = at >= 0 ? lines[at].indexOf(".", left + 1) : -1;
  return at >= 0 && lines[at + 1].slice(left + 1, right).trim()
    .replace(/\s+/g, " ") === "A C";
});
keys(KEY.esc);
await see("8Delete");
keys(KEY.enter);              // on .., the one entry of C:\VC
await see("readme");
keys(KEY.F7);
await see("Create the directory");
keys("newdir", KEY.enter);
await step("a directory made in the page's memory",
           () => isDirectory(page0, "/drive/c/newdir"));
await see("NEWDIR");
// Enter of the row of keys that a phone lacks, its last key, which goes
// as the keyboard's Enter does.
const rowEnter = elements.keys.children.at(-1);
await step("the row of keys ends in Enter",
           () => rowEnter && rowEnter.textContent === "Enter");
keys(KEY.end);
rowEnter.listeners.click();
await step("Enter of the row on a file opens it in a tab", () => opened.length === 1
           && opened.blob && opened.blob.size
              === readFileSync(path.join(own, "readme.txt")).length);
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
await step("a file written in the editor is in the page's memory",
           () => (inPage0("note.txt") || "").startsWith("hello from the page"));
keys(KEY.esc);
await see("8Delete");
keys(KEY.home, KEY.down, KEY.down, KEY.F8);
await see("Do you wish to delete");
keys(KEY.enter);
await step("a directory removed from the page's memory",
           () => !isDirectory(page0, "/drive/c/newdir"));
await see("8Delete");
// The last of the panel, readme.txt: deleted.
keys(KEY.end, KEY.F8);
await see("Do you wish to delete");
keys(KEY.enter);
await step("a file deleted from the page's memory",
           () => inPage0("readme.txt") === null && inPage0("notes", "todo.txt") !== null);
await see("8Delete");
keys(KEY.shiftF9);
await see("Save");
keys(KEY.enter);
await step("the setup saved on C:, in the page's memory",
           () => inPage0("vc", "vc.ini") !== null);
await see("8Delete");
keys(KEY.F10);
await see("Do you want to quit");
keys(KEY.enter);
await step("the program ends, and the page says so",
           () => elements.ended && !elements.ended.hidden
                 && elements.ended.textContent.includes("Reload"));
await step("nothing is kept: no store of the browser's asked for, and " +
           "nothing served changed",
           () => page0.stored.length === 0 && listing(top) === before);

// 2. The page loaded again: its drive as it is served, whatever the
// visit before did.
{
  const page = load(base);
  await page.see(" C:\\VC ");
  await page.step("a page loaded again starts from what is served",
                  () => inPage(page, "/drive/c/readme.txt") !== null
                        && inPage(page, "/drive/c/note.txt") === null
                        && inPage(page, "/drive/c/vc/vc.ini") === null
                        && !isDirectory(page, "/drive/c/newdir")
                        && page.stored.length === 0);
}
host.close();

// 3. The site as the build has made it to be shipped: C:\VC with the
// program's files, C:\ with AUTOEXEC.BAT and CONFIG.SYS, the clock on by
// the VC.INI that it ships, the empty A:, and the strip of the keys that
// the browser keeps, with its hint.
{
  const other = await hostOf(shipped);
  const page = load(`http://127.0.0.1:${other.address().port}`);
  const names = (at) => page.fs.readdir(at).filter((n) => n !== "."
                                                  && n !== "..").sort().join();
  await page.see(" C:\\VC ");
  await page.step("the shipped site's C: has vc, AUTOEXEC.BAT and CONFIG.SYS",
                  () => names("/drive/c") === "autoexec.bat,config.sys,vc");
  await page.step("the shipped site's C:\\VC has the program's files",
                  () => ["license.txt", "vc.ext", "vc.hlp", "vc.ini", "vc.mnu",
                         "vcedit.ext", "vcview.ext"]
                    .every((name) => names("/drive/c/vc").split(",")
                      .includes(name)));
  await page.step("the shipped site's C:\\VC has its licences in one file",
                  () => !names("/drive/c/vc").includes("dos32a")
                    && Buffer.from(page.fs.readFile("/drive/c/vc/license.txt"))
                         .toString("latin1")
                       .includes("This product uses DOS/32 Advanced DOS " +
                                 "Extender technology."));
  await page.see("vc       ini");
  await page.see("vc       hlp");
  // The clock, in the corner of the top row, "12:00p" as the program
  // writes it, its colon blinking.
  await page.step("the clock is on, in the top row's corner",
                  () => /^[ \d]\d[: ]\d\d[ap]$/.test(
                    page.text().split("\n")[0].slice(74)));
  const left = () => page.text().split("\n")[0].slice(0, 40);
  await page.step("C:\\VC on the left, C:\\ on the right",
                  () => left().includes(" C:\\VC ")
                        && page.text().split("\n")[0].slice(40)
                          .includes(" C:\\ "));
  await page.step("A: is there and empty",
                  () => isDirectory(page, "/drive/a")
                        && names("/drive/a") === "");
  page.keys(KEY.altF1);
  await page.see("Drive");
  page.keys("a");
  await page.see(" A:\\ ");
  page.keys(KEY.altF1);
  await page.see("Drive");
  page.keys("c");
  await page.see(" C:\\VC ");
  const strip = page.elements["kept-keys"].children;
  const labels = strip.filter((child) => child.id !== "kept-hint")
    .map((child) => child.textContent);
  const hint = strip.find((child) => child.id === "kept-hint");
  await page.step("the strip has the keys that the browser keeps, and a hint",
                  () => labels.join() === "Ctrl-PgUp,Ctrl-PgDn,Ctrl-T," +
                          "Ctrl-W,Ctrl-F4,Alt-F4"
                        && hint && /Full screen/.test(hint.textContent)
                        && /Chrome and Edge/.test(hint.textContent));
  // Ctrl-PgUp of the strip, as the browser would not hand it on: C:\.
  strip[0].listeners.click();
  await page.step("Ctrl-PgUp of the strip takes the left panel to C:\\",
                  () => left().includes(" C:\\ "));
  other.close();
}

// 4. A host that serves the page and no drive: the page says so and does
// not start.
{
  const answer = createServer((request, response) => {
    if (new URL(request.url, "http://x").pathname === "/") {
      response.writeHead(200, { "Content-Type": "text/html" });
      response.end(readFileSync(html));
      return;
    }
    response.writeHead(404);
    response.end();
  });
  await new Promise((done) => answer.listen(0, "127.0.0.1", done));
  const page = load(`http://127.0.0.1:${answer.address().port}`);
  await page.step("a site with no drive: the page says so and does not start",
                  () => page.elements.ended && !page.elements.ended.hidden
                        && page.elements.ended.textContent.includes("could not be loaded")
                        && page.screen === null);
  answer.close();
}
rmSync(top, { recursive: true, force: true });
if (failed > 0) {
  console.log(`${failed} of ${passed + failed} steps failed`);
  process.exit(1);
}
console.log(`${passed} steps passed`);
process.exit(0);
