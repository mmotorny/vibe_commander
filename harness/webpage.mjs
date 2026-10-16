// The test of the web page's program against its server, run by
// harness/webpage.py: see there for what it holds. Node's own modules
// alone. It prints a line for each step that fails, with the screen, and
// exits with 1, or says how many passed.

import { readFileSync, mkdtempSync, mkdirSync, readdirSync, existsSync,
         chmodSync, rmSync, statSync, writeFileSync } from "node:fs";
import path from "node:path";
import vm from "node:vm";
import { createServer } from "node:http";
import { execFileSync } from "node:child_process";

const [web, html, work] = process.argv.slice(2);
const { makeServer, prepare, treeOf } = await import(path.join(web, "server.mjs"));
const { FakeIndexedDB } = await import(new URL("./fakeidb.mjs", import.meta.url));
const { layFiles } = await import(new URL("./webfiles.mjs", import.meta.url));
const top = mkdtempSync(path.join(work, "webpage-"));
const roots = await prepare({ c: path.join(top, "c") });
// The test's own files on C:, beside the program's directory vc.
layFiles(roots.c);
const server = makeServer(roots, html);
await new Promise((done) => server.listen(0, "127.0.0.1", done));
const base = `http://127.0.0.1:${server.address().port}`;

let passed = 0, failed = 0;
const sleep = (ms) => new Promise((done) => setTimeout(done, ms));
const KEY = { F2: "\x1b[Q", F7: "\x1b[18~", F8: "\x1b[19~", F10: "\x1b[21~",
              shiftF4: "\x1b[1;2S", shiftF9: "\x1b[20;2~", home: "\x1b[H",
              end: "\x1b[F", altF2: "\x1b[1;3Q", down: "\x1b[B", esc: "\x1b[27u", enter: "\r" };

// The page loaded from `base`, its scripts run as the browser would, with a
// document of nothing but what the page asks of one, and `indexedDB` as
// the window has it (harness/fakeidb.mjs, or none).
function load(base, indexedDB) {
  class Element {
    constructor(id) {
      this.id = id;
      this.style = { setProperty(name, value) { this[name] = value; } };
      this.dataset = {};
      // Hidden as the shell has them at the start.
      this.hidden = ["keys", "kept", "reset", "notice", "ended"].includes(id);
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
                 screen: null };
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
    navigator: { userAgent: "node" }, confirm: () => true, indexedDB,
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

// 1. The page against its server, which every change is made on.
const served = new FakeIndexedDB();
const page0 = load(base, served);
const { elements, opened, keys, step, see, text } = page0;
const onServer = (...names) => path.join(roots.c, ...names);

// The program starts in C:\VC, where its own files are, on the page's
// one drive; then goes up to C:\, where the test's files are.
await see(" C:\\VC ");

// The licences: one link in the bar, which shows the texts of the
// repository's files that the page carries, the project's own and
// Volkov Commander's among them, and not DOS/32A's, which is in no page;
// Esc closes them, and neither Esc nor any key while they are up reaches
// the program.
{
  const repository = path.resolve(web, "../../..");
  const texts = ["LICENSE", "src/asm/LICENSE.TXT", "LICENSE.EMSCRIPTEN",
                 "LICENSE.MUSL", "LICENSE.FONT"]
    .map((name) => readFileSync(path.join(repository, name), "utf8")
      .replace(/\r\n/g, "\n"));
  const dos32a = readFileSync(path.join(repository, "LICENSE.DOS32A"), "utf8");
  const shell = readFileSync(html, "utf8");
  await step("the bar has one link to the licences, and no credit apart",
             () => /<button [^>]*\bid="?licences-open\b[^>]*>Licences<\/button>/.test(shell)
                   && !shell.includes("CC BY-SA 4.0</a>"));
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
                   && texts.every((text, at) => shown[at] === text)
                   && !shown.includes(dos32a));
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
await step("the drive box has C: alone", () => {
  const lines = text().split("\n");
  const at = lines.findIndex((line) => line.includes("Choose"));
  // The letters between the box's sides, under its question.
  const left = at >= 0 ? lines[at].indexOf(". Choose") : -1;
  const right = at >= 0 ? lines[at].indexOf(".", left + 1) : -1;
  return at >= 0 && lines[at + 1].slice(left + 1, right).trim() === "C";
});
keys(KEY.esc);
await see("8Delete");
keys(KEY.enter);              // on .., the one entry of C:\VC
await see("readme");
keys(KEY.F7);
await see("Create the directory");
keys("newdir", KEY.enter);
await step("a directory made on the server",
           () => existsSync(onServer("newdir")));
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
// The server refuses: nothing may be made in C:'s root.
await see("8Delete");
chmodSync(roots.c, 0o555);
keys(KEY.F7);
await see("Create the directory");
keys("locked", KEY.enter);
await step("a refusal of the server is DOS's error, nothing made",
           () => !existsSync(onServer("locked"))
                 && text().includes("t create"));
chmodSync(roots.c, 0o755);
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

await step("the page of the server keeps nothing in the browser",
           () => served.opened === 0
                 && (!elements.kept || elements.kept.textContent === ""));
// What the page takes for a static site where api/server is not answered
// as it should be: the server never serves it.
const drivesOfSite = (await fetch(`${base}/drives/c.json`)).status;
await step("the server serves no drives of a static site",
           () => drivesOfSite === 404);
server.close();

// 2. The page served statically, as static.mjs makes it, by a host that
// answers 404 for what it has not, as GitHub Pages: the drive starts from
// what is served, and the changes are kept in IndexedDB (the stand-in,
// one for the browser across the page's visits). The site ships C: with
// its directory vc alone; the test serves its own files on C: besides,
// in a tree of its own in place of the site's.
const site = path.join(top, "static");
execFileSync(process.execPath, [path.join(web, "static.mjs"), "--page", html,
                                "--out", site]);
const shipped = JSON.parse(readFileSync(path.join(site, "drives", "c.json"),
                                        "utf8"));
await step("the static site ships C: with its vc alone",
           () => readdirSync(path.join(site, "drives")).join() === "c.json"
                 && shipped.entries.length === 1
                 && shipped.entries[0].path === "vc"
                 && shipped.entries[0].directory === true);
{
  const own = path.join(top, "served");
  mkdirSync(path.join(own, "vc"), { recursive: true });
  layFiles(own);
  writeFileSync(path.join(site, "drives", "c.json"),
                JSON.stringify(await treeOf(own)));
}
// A host of the static site, which answers a name that it has not by
// `missing`, and anything but the page by `away` where that is set.
async function hostOf({ missing, away = null }) {
  const host = createServer((request, response) => {
    const name = new URL(request.url, "http://localhost").pathname;
    const file = path.join(site, name.endsWith("/") ? `${name}index.html` : name);
    if (away !== null && !name.endsWith("/")) {
      response.writeHead(away, { "Content-Type": "text/html" });
      response.end("<h1>away</h1>");
      return;
    }
    if (!file.startsWith(site) || !existsSync(file)
        || !statSync(file).isFile()) {
      if (missing === 200) {
        response.writeHead(200, { "Content-Type": "text/html" });
        response.end(readFileSync(path.join(site, "index.html")));
        return;
      }
      response.writeHead(missing, { "Content-Type": "text/html" });
      response.end(`<h1>${missing}</h1>`);
      return;
    }
    response.writeHead(200);
    response.end(readFileSync(file));
  });
  await new Promise((done) => host.listen(0, "127.0.0.1", done));
  return host;
}
const host = await hostOf({ missing: 404 });
const at = `http://127.0.0.1:${host.address().port}`;
const browser = new FakeIndexedDB();
const kept = () => browser.records("vibe-commander", "records");
const keptText = (key) => {
  const record = kept().get(key);
  return record ? Buffer.from(record.data).toString("latin1") : null;
};
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
{
  const page = load(at, browser);
  await page.see(" C:\\VC ");
  await page.step("the page with no server keeps its changes in the browser",
                  () => page.elements.kept.textContent.includes("this browser")
                        && page.elements.reset.hidden === false);
  page.keys(KEY.enter);         // on .., up to C:\
  await page.see("readme");
  page.keys(KEY.F7);
  await page.see("Create the directory");
  page.keys("newdir", KEY.enter);
  await page.step("a directory made is kept", () => kept().has("c:newdir"));
  await page.see("NEWDIR");
  page.keys(KEY.shiftF4);
  await page.see("Edit the file");
  page.keys("note.txt", KEY.enter);
  await page.see("New-file");
  page.keys(KEY.enter);
  await page.see("2Save");
  for (const [place, character] of [..."kept here"].entries()) {
    page.keys(character);
    await page.see(`Col ${place + 2} `);
  }
  page.keys(KEY.F2);
  await page.step("a file written in the editor is kept at its close",
                  () => (keptText("c:note.txt") || "").startsWith("kept here"));
  page.keys(KEY.esc);
  await page.see("8Delete");
  // The last of the panel, readme.txt: deleted.
  page.keys(KEY.end, KEY.F8);
  await page.see("Do you wish to delete");
  page.keys(KEY.enter);
  await page.step("a file deleted is gone from what is kept",
                  () => kept().has("c:notes") && !kept().has("c:readme.txt"));
  await page.see("8Delete");
  page.keys(KEY.shiftF9);
  await page.see("Save");
  page.keys(KEY.enter);
  await page.step("the setup saved on C: is kept",
                  () => kept().has("c:vc/vc.ini"));
  await page.see("8Delete");
  page.keys(KEY.F10);
  await page.see("Do you want to quit");
  page.keys(KEY.enter);
  await page.step("the program ends", () => page.elements.ended
                  && !page.elements.ended.hidden
                  && page.elements.ended.textContent.includes("Reload"));
}
{
  // The page loaded again: the drive as it was left.
  const page = load(at, browser);
  await page.see("8Delete");
  await page.step("the next visit has the changes",
                  () => (inPage(page, "/drive/c/note.txt") || "")
                          .startsWith("kept here")
                        && inPage(page, "/drive/c/readme.txt") === null
                        && isDirectory(page, "/drive/c/newdir")
                        && inPage(page, "/drive/c/vc/vc.ini") !== null);
  page.elements.reset.listeners.click();
  await page.step("Reset forgets them, and loads the page anew",
                  () => page.reloads === 1 && kept().size === 0);
}
{
  const page = load(at, browser);
  await page.see(" C:\\VC ");
  await page.step("after a reset, the drive as it is served",
                  () => inPage(page, "/drive/c/readme.txt") !== null
                        && inPage(page, "/drive/c/note.txt") === null
                        && kept().has("c:readme.txt"));
}
{
  // A browser that gives the page no IndexedDB: the page's memory, and
  // the page says so.
  const page = load(at, undefined);
  await page.see(" C:\\VC ");
  await page.step("no IndexedDB: the changes last as long as the page",
                  () => page.elements.kept.textContent.includes("no storage")
                        && page.elements.reset.hidden === true);
  page.keys(KEY.F7);
  await page.see("Create the directory");
  page.keys("mem", KEY.enter);
  await page.step("a directory made in the page's memory",
                  () => isDirectory(page, "/drive/c/vc/mem"));
}
host.close();
// Hosts that answer api/server otherwise than by 4xx: with a page of
// their own, 200, as a host of pages in a sandbox may, or by 500; the
// drive beside the page says that it is a static site. And a host that
// answers 502 for anything but the page, a proxy in front of a server
// that has stopped: the page does not start.
for (const missing of [200, 500]) {
  const other = await hostOf({ missing });
  const page = load(`http://127.0.0.1:${other.address().port}`, undefined);
  await page.see(" C:\\VC ");
  await page.step(`a host that answers ${missing} for a name it has not is static`,
                  () => page.elements.kept.textContent.includes("no storage"));
  other.close();
}
{
  const other = await hostOf({ missing: 404, away: 502 });
  const page = load(`http://127.0.0.1:${other.address().port}`, undefined);
  await page.step("a server that is away: the page says so and does not start",
                  () => page.elements.ended && !page.elements.ended.hidden
                        && page.elements.ended.textContent.includes("502")
                        && page.screen === null);
  other.close();
}
rmSync(top, { recursive: true, force: true });
if (failed > 0) {
  console.log(`${failed} of ${passed + failed} steps failed`);
  process.exit(1);
}
console.log(`${passed} steps passed`);
process.exit(0);
