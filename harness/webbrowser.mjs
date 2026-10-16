// The web page in a real browser: the Chromium that the machine has,
// headless, driven over its DevTools protocol (harness/browser.mjs), with
// Node's own modules alone. The test portable_c-web-browser runs it by
// harness/webbrowser.py; a person runs it with --shots to have its
// screenshots to look at:
//
//   node harness/webbrowser.mjs --chromium chromium \
//       --page build/portable_c/web/vc.html \
//       --site build/portable_c/web/static --work build/browser \
//       [--shots build/browser/shots] [--only server|static]
//
// The page is opened both ways that it is served: by its server,
// src/portable_c/web/server.mjs, run on a free port with its two drives in
// the work directory, and as the static site, behind a host that answers
// 404 for what it has not, as GitHub Pages does. Each way, on a desktop,
// 1280 by 800, and as an iPhone, 390 by 844 at three device pixels to
// one, with touch and Safari's agent, then turned, 844 by 390. What it
// holds, step by step, is in `desktop` and `phone` below; a step that
// fails says what the screen was, saves a screenshot of the moment into
// the work directory, and the run goes on to the next way of serving.
// The work directory has to be one that Chromium may read and write: one
// of a snap reads no hidden directory and not the real /tmp, so it is in
// build/. Every process that it starts it ends: Chromium, the server.

import { spawn } from "node:child_process";
import { createServer } from "node:http";
import { existsSync, readFileSync, rmSync, statSync, mkdirSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { Browser, VIEWPORTS, sleep } from "./browser.mjs";

const here = path.dirname(fileURLToPath(import.meta.url));
const web = path.join(here, "..", "src", "portable_c", "web");

function options(argv) {
  const said = { chromium: "chromium", shots: null, only: null };
  for (let i = 0; i < argv.length; i += 2) {
    const name = argv[i].replace(/^--/, "");
    if (!["chromium", "page", "site", "work", "shots", "only"].includes(name)
        || i + 1 >= argv.length) {
      console.error(`webbrowser.mjs: what is ${argv[i]}? See the top of the file.`);
      process.exit(2);
    }
    said[name] = path.isAbsolute(argv[i + 1]) || name === "chromium"
      || name === "only" ? argv[i + 1] : path.resolve(argv[i + 1]);
  }
  for (const wanted of ["page", "site", "work"])
    if (!said[wanted]) {
      console.error(`webbrowser.mjs: --${wanted} is wanted.`);
      process.exit(2);
    }
  return said;
}

const said = options(process.argv.slice(2));
let passed = 0;
const failures = [];

// A free port of the loopback's.
async function freePort() {
  const probe = createServer();
  await new Promise((done) => probe.listen(0, "127.0.0.1", done));
  const { port } = probe.address();
  await new Promise((done) => probe.close(done));
  return port;
}

// The page's server, a process of its own, on a free port, with its
// drives in `top`, made afresh.
async function startServer(top) {
  rmSync(top, { recursive: true, force: true });
  const port = await freePort();
  const child = spawn(process.execPath, [path.join(web, "server.mjs"),
    "--port", String(port), "--page", said.page,
    "--drive-c", path.join(top, "c"), "--drive-d", path.join(top, "d"),
    "--demo", path.join(web, "demo")], { stdio: ["ignore", "pipe", "pipe"] });
  let output = "";
  child.stdout.on("data", (chunk) => { output += chunk; });
  child.stderr.on("data", (chunk) => { output += chunk; });
  const url = `http://127.0.0.1:${port}/`;
  for (let tries = 0; tries < 100; tries++) {
    if (child.exitCode !== null)
      throw new Error(`the server ended: ${output}`);
    try {
      const answer = await fetch(`${url}api/server`);
      if (answer.ok)
        return { url, stop: () => stop(child), drives: top };
    } catch {
      // not listening yet
    }
    await sleep(100);
  }
  stop(child);
  throw new Error(`the server did not answer: ${output}`);
}

async function stop(child) {
  if (child.exitCode !== null)
    return;
  child.kill();
  for (let tries = 0; tries < 50 && child.exitCode === null; tries++)
    await sleep(100);
  if (child.exitCode === null)
    child.kill("SIGKILL");
}

// A host of the static site that answers 404 for what it has not.
async function startSite() {
  const site = said.site;
  const host = createServer((request, response) => {
    const name = decodeURIComponent(new URL(request.url, "http://x").pathname);
    const file = path.join(site, name.endsWith("/") ? `${name}index.html` : name);
    if (!file.startsWith(site) || !existsSync(file) || !statSync(file).isFile()) {
      response.writeHead(404, { "Content-Type": "text/html" });
      response.end("<h1>404</h1>");
      return;
    }
    const type = file.endsWith(".html") ? "text/html"
      : file.endsWith(".json") ? "application/json" : "application/octet-stream";
    response.writeHead(200, { "Content-Type": type });
    response.end(readFileSync(file));
  });
  await new Promise((done) => host.listen(0, "127.0.0.1", done));
  return { url: `http://127.0.0.1:${host.address().port}/`,
           stop: () => new Promise((done) => {
             host.close(done);
             host.closeAllConnections();
           }) };
}

// What the console has said that is an error, but for the one that a
// static page is meant to cause: it asks for api/server to know whether it
// has a server (askMode of page.js), and Chromium reports the host's 404
// for it as an error of the console.
function errors(tab, mode) {
  return tab.errors().filter((e) => !(mode === "static"
    && /status of 404 .*\/api\/server\)$/.test(e.text)));
}

// One check: `what` is said, and passes when `test` returns without
// throwing.
async function check(mode, view, what, test, tab) {
  try {
    await test();
    passed++;
  } catch (caught) {
    failures.push(`${mode}, ${view}: ${what}: ${caught.message}`);
    console.log(`FAIL ${mode}, ${view}: ${what}: ${caught.message}`);
    if (tab) {
      try {
        await tab.shot(path.join(said.work, `failed-${mode}-${view}-${failures.length}.png`));
      } catch {
        // the tab is gone
      }
    }
    throw caught;
  }
}

function must(condition, text) {
  if (!condition)
    throw new Error(text);
}

async function shot(tab, name) {
  if (said.shots !== null)
    await tab.shot(path.join(said.shots, `${name}.png`));
}

// Where a text is on the screen, from `column` on: { column, row }.
function find(rows, text, from = 0) {
  for (let row = 0; row < rows.length; row++) {
    const at = rows[row].indexOf(text, from);
    if (at >= 0)
      return { column: at, row };
  }
  return null;
}

// The mini status of the right panel: the current file's name.
const rightStatus = (rows) => rows[21].slice(40);

// The page started: the key bar on the screen, nothing that says that it
// stopped, and the bar's line on where changes are kept.
async function started(tab, mode) {
  const rows = await tab.until((r, s) => s.includes("1Help") && s.includes("10Quit"),
                               30000, "the program's screen");
  must((await tab.text("ended")) === null, "the page says that it ended");
  const kept = await tab.text("kept");
  if (mode === "server")
    must(kept === null, `the bar says "${kept}" for a page of the server`);
  else
    must(kept === "Changes are kept in this browser.",
         `the bar says ${JSON.stringify(kept)} for a static page`);
  return rows;
}

// The right panel on D:, both panels shown, the cursor on CODE.
async function toD(tab) {
  await tab.press("F2", { alt: true });
  await tab.settle();
  await tab.press("d");
  return tab.until((r) => r[0].slice(40).includes("D:\\")
                   && rightStatus(r).includes("CODE"), 10000, "D: in the right panel");
}

async function desktop(browser, mode, served) {
  const tab = await browser.page(VIEWPORTS.desktop);
  const view = "desktop";
  const step = (what, test) => check(mode, view, what, test, tab);
  await tab.go(served.url);
  await step("the page starts", () => started(tab, mode));
  await step("the font is the VGA's, and loaded", async () => {
    const font = await tab.eval("getComputedStyle(document.getElementById('screen')).fontFamily");
    must(font.includes("Web437 IBM VGA 8x16"), `the screen's font is ${font}`);
    must(await tab.eval("document.fonts.check('16px \"Web437 IBM VGA 8x16\"')"),
         "the font is not loaded");
  });
  await step("the grid is 640 by 400, a whole multiple of the font", async () => {
    const r = await tab.rect("#screen");
    must(r.width === 640 && r.height === 400, `the grid is ${r.width} by ${r.height}`);
  });
  await shot(tab, `${mode}-desktop-start`);
  await step("Ctrl-F1: the left panel too", async () => {
    await tab.press("F1", { ctrl: true });
    await tab.until((r) => (r[0].match(/╔/g) || []).length === 2, 5000, "two panels");
  });
  await step("the colours: the panel's VGA blue, its frame cyan", async () => {
    const colours = await tab.eval("(() => { const row = document.querySelector('#screen > span');" +
      " const run = row.querySelector('span'); const s = getComputedStyle(run);" +
      " return [s.color, s.backgroundColor]; })()");
    must(colours[0] === "rgb(85, 255, 255)" && colours[1] === "rgb(0, 0, 170)",
         `the frame is ${colours.join(" on ")}`);
  });
  await step("Alt-F2, d: D:'s demo files in the right panel", async () => {
    const rows = await toD(tab);
    for (const name of ["CODE", "DATA", "NOTES", "readme   txt"])
      must(find(rows, name, 40) !== null, `${name} is not listed`);
  });
  await shot(tab, `${mode}-desktop-panels`);
  await step("Down, Down, Up", async () => {
    await tab.press("Down");
    await tab.press("Down");
    await tab.until((r) => rightStatus(r).includes("NOTES"), 5000, "NOTES");
    await tab.press("Up");
    await tab.until((r) => rightStatus(r).includes("DATA"), 5000, "DATA");
  });
  await step("Tab, and Tab again", async () => {
    await tab.press("Tab");
    await tab.until((r) => r[23].startsWith("C:\\>"), 5000, "C:'s prompt");
    await tab.press("Tab");
    await tab.until((r) => r[23].startsWith("D:\\>"), 5000, "D:'s prompt");
  });
  await step("F1, with no help file: nothing, and the keys still the program's", async () => {
    const before = (await tab.settle()).join("\n");
    await tab.press("F1");
    const after = (await tab.settle(600)).join("\n");
    must(after === before, "the screen changed");
  });
  await step("the editor: Shift-F4, a name, a line, F2, Esc", async () => {
    await tab.press("F4", { shift: true });
    await tab.settle();
    await tab.type("note.txt");
    await tab.press("Enter");
    await tab.until((r, all) => all.includes("New-file"), 5000, "the question of a new file");
    await tab.press("Enter");
    await tab.settle();
    // As a person types: the editor clears what is typed ahead of it, as
    // the original does with Clear keyboard buffer on, its default.
    for (const character of "Hello from Chromium.") {
      await tab.press(character);
      await tab.settle(150);
    }
    await tab.press("F2");
    await tab.settle();
    await tab.press("Escape");
    await tab.until((r) => find(r, "note     txt", 40) !== null, 10000,
                    "note.txt in the panel");
  });
  if (mode === "server")
    await step("the file is on the server", async () => {
      const file = path.join(served.drives, "d", "note.txt");
      must(existsSync(file), `${file} is not there`);
      const text = readFileSync(file, "latin1");
      must(text.startsWith("Hello from Chromium."), `it has ${JSON.stringify(text)}`);
    });
  await step("Enter on a text file: a new tab, or the link to one", async () => {
    await tab.press("End");
    await tab.until((r) => rightStatus(r).includes("readme"), 5000, "readme.txt");
    const before = browser.opened.length;
    await tab.press("Enter");
    const end = Date.now() + 5000;
    let link = null;
    while (Date.now() < end && browser.opened.length === before && link === null) {
      await sleep(100);
      link = await tab.text("notice");
    }
    must(browser.opened.length > before || (link && link.includes("readme")),
         "no tab was opened and no link given");
    if (browser.opened.length > before) {
      const { targetId } = browser.opened[browser.opened.length - 1];
      let url = "";
      for (let tries = 0; tries < 50 && !url; tries++) {
        const { targetInfo } = await browser.connection.send("Target.getTargetInfo",
                                                             { targetId });
        url = targetInfo.url;
        if (!url)
          await sleep(100);
      }
      must(url.startsWith("blob:"), `the tab is ${JSON.stringify(url)}`);
      await browser.connection.send("Target.closeTarget", { targetId });
    }
  });
  await step("the Licences open and close, no key reaching the program", async () => {
    const before = (await tab.settle()).join("\n");
    await tab.hit("#licences-open");
    await sleep(300);
    must(await tab.eval("document.getElementById('licences').open"), "the dialog is not open");
    const works = await tab.eval("document.querySelectorAll('#licences h2').length");
    must(works === 5, `the dialog has ${works} works`);
    await shot(tab, `${mode}-desktop-licences`);
    await tab.press("Escape");
    await sleep(300);
    must(!(await tab.eval("document.getElementById('licences').open")), "Esc left it open");
    await tab.hit("#licences-open");
    await sleep(300);
    await tab.hit("#licences-close");
    await sleep(300);
    must(!(await tab.eval("document.getElementById('licences').open")),
         "its button left it open");
    must((await tab.settle()).join("\n") === before, "the screen changed");
  });
  if (mode === "static")
    await step("a reload: the file kept in the browser", async () => {
      await tab.reload();
      await started(tab, mode);
      const rows = await toD(tab);
      must(find(rows, "note     txt", 40) !== null, "note.txt is gone");
    });
  await step("the console has no errors", () => {
    const wrong = errors(tab, mode);
    must(wrong.length === 0, wrong.map((e) => e.text).join("; "));
  });
  await tab.close();
}

async function phone(browser, mode, served) {
  const tab = await browser.page(VIEWPORTS.portrait);
  let view = "portrait";
  const step = (what, test) => check(mode, view, what, test, tab);
  // What fits: the grid within the viewport's width, above the bar, and
  // the bar's bottom within its height.
  const fits = async (width, height) => {
    const grid = await tab.rect("#screen");
    const bar = await tab.rect("#bar");
    must(grid.left >= 0 && grid.right <= width + 0.5,
         `the grid is from ${grid.left} to ${grid.right} of ${width}`);
    must(grid.top >= 0 && grid.bottom <= bar.top + 0.5,
         `the grid ends at ${grid.bottom}, the bar begins at ${bar.top}`);
    must(bar.bottom <= height + 0.5, `the bar ends at ${bar.bottom} of ${height}`);
    must(grid.width > width * 0.8 || grid.height > (height - bar.height) * 0.8,
         `the grid is ${grid.width} by ${grid.height}: it does not fill the screen`);
    return grid;
  };
  // The row of keys: every key within the viewport, and tall enough to tap.
  const whole = async (width, height) => {
    const all = await tab.eval("Array.from(document.querySelectorAll('#keys button'))" +
      ".map((b) => { const r = b.getBoundingClientRect();" +
      " return [b.textContent, r.left, r.right, r.top, r.bottom, r.height]; })");
    for (const [label, left, right, top, bottom, high] of all) {
      must(left >= 0 && right <= width + 0.5 && top >= 0 && bottom <= height + 0.5,
           `${label} is from ${left},${top} to ${right},${bottom}`);
      must(high >= 28, `${label} is ${high} high`);
    }
  };
  await tab.go(served.url);
  await step("the page starts", () => started(tab, mode));
  await step("the row of keys shows", async () => {
    must(!(await tab.eval("document.getElementById('keys').hidden")), "it is hidden");
    const keys = await tab.eval("document.querySelectorAll('#keys button').length");
    must(keys === 21, `it has ${keys} keys`);
    const last = await tab.eval("document.querySelector('#keys button:last-child').textContent");
    must(last === "Enter", `its last key is ${last}`);
  });
  await step("the grid fits the width, nothing cut off", () => fits(390, 844));
  // The page starts from the setup that the desktop's run saved, two
  // panels: Ctrl-F1 of the row takes the left one away and brings it back.
  const panels = (r) => (r[0].match(/╔/g) || []).length;
  await step("Ctrl and F1 of the row, twice: the left panel off and on", async () => {
    const before = panels(await tab.settle());
    for (const now of [3 - before, before]) {
      await tab.hit('[data-latch="ctrl"]');
      await tab.hit("#keys button:nth-child(3)");
      await tab.until((r) => panels(r) === now, 5000, `${now} panels`);
    }
  });
  await step("Alt and F2 of the row, then d: D:", async () => {
    await tab.hit('[data-latch="alt"]');
    await tab.hit("#keys button:nth-child(4)");
    await tab.settle();
    await tab.press("d");
    await tab.until((r) => r[0].slice(40).includes("D:\\"), 10000, "D:");
  });
  await step("a tap on a file moves the cursor there", async () => {
    const rows = await tab.settle();
    const at = find(rows, "readme", 40);
    must(at !== null, "readme.txt is not listed");
    await tab.hitCell(at.column + 1, at.row);
    await tab.until((r) => rightStatus(r).includes("readme"), 5000, "the cursor on readme.txt");
  });
  await step("a tap on CODE and Enter of the row: into it", async () => {
    const rows = await tab.settle();
    const at = find(rows, "CODE", 40);
    must(at !== null, "CODE is not listed");
    await tab.hitCell(at.column + 1, at.row);
    await tab.until((r) => rightStatus(r).includes("CODE"), 5000, "the cursor on CODE");
    await tab.hit("#keys button:last-child");
    await tab.until((r) => r[0].slice(40).includes("D:\\CODE"), 5000, "D:\\CODE");
  });
  await step("the row of keys whole, each key tappable", () => whole(390, 844));
  await shot(tab, `${mode}-portrait`);
  view = "landscape";
  await step("turned: the grid refits", async () => {
    await tab.view(VIEWPORTS.landscape);
    await sleep(800);
    await fits(844, 390);
    await tab.settle();
  });
  await step("turned: the row of keys whole, each key tappable", () => whole(844, 390));
  await shot(tab, `${mode}-landscape`);
  await step("the console has no errors", () => {
    const wrong = errors(tab, mode);
    must(wrong.length === 0, wrong.map((e) => e.text).join("; "));
  });
  await tab.close();
}

async function main() {
  mkdirSync(said.work, { recursive: true });
  rmSync(path.join(said.work, "profile"), { recursive: true, force: true });
  if (said.shots !== null)
    mkdirSync(said.shots, { recursive: true });
  const browser = await Browser.start(said.chromium, path.join(said.work, "profile"));
  console.log(`${browser.version}`);
  try {
    for (const mode of ["server", "static"]) {
      if (said.only !== null && said.only !== mode)
        continue;
      const served = mode === "server"
        ? await startServer(path.join(said.work, "drives"))
        : await startSite();
      try {
        for (const run of [desktop, phone]) {
          try {
            await run(browser, mode, served);
          } catch {
            // said already; on to the next
          }
        }
      } finally {
        await served.stop();
      }
    }
  } finally {
    await browser.close();
  }
  console.log(`${passed} passed, ${failures.length} failed`);
  if (said.shots !== null)
    console.log(`screenshots in ${said.shots}`);
  return failures.length === 0 ? 0 : 1;
}

process.exitCode = await main();
