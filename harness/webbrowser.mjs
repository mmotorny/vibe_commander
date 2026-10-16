// The web page in a real browser: the Chromium that the machine has,
// headless, driven over its DevTools protocol (harness/browser.mjs), with
// Node's own modules alone. The test portable_c-web-browser runs it by
// harness/webbrowser.py; a person runs it with --shots to have its
// screenshots to look at:
//
//   node harness/webbrowser.mjs --chromium chromium \
//       --site build/portable_c/web/static --work build/browser \
//       [--shots build/browser/shots]
//
// The page is opened as the static site, served by src/portable_c/web/
// serve.mjs, which answers 404 for what it has not, as GitHub Pages does.
// The site ships C: with C:\VC's files and C:\'s (TODO.md, "C:\VC as it is
// shipped"): the test lays its own files on C: beside an empty vc
// (harness/webfiles.mjs), in a copy of the site whose tree of C: has them. On a desktop, 1280 by 800, and as an
// iPhone, 390 by 844 at three device pixels to one, with touch and
// Safari's agent, then turned, 844 by 390. What it holds, step by step,
// is in `desktop` and `phone` below; a step that fails says what the
// screen was, saves a screenshot of the moment into the work directory,
// and the run goes on to the next view. The work directory has to be
// one that Chromium may read and write: one of a snap reads no hidden
// directory and not the real /tmp, so it is in build/. Every process
// that it starts it ends: Chromium, the host.

import { readFileSync, rmSync, mkdirSync, cpSync, writeFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { Browser, VIEWPORTS, sleep } from "./browser.mjs";
import { layFiles } from "./webfiles.mjs";

const here = path.dirname(fileURLToPath(import.meta.url));
const web = path.join(here, "..", "src", "portable_c", "web");
const { treeOf } = await import(path.join(web, "static.mjs"));
const { serve } = await import(path.join(web, "serve.mjs"));

function options(argv) {
  const said = { chromium: "chromium", shots: null };
  for (let i = 0; i < argv.length; i += 2) {
    const name = argv[i].replace(/^--/, "");
    if (!["chromium", "site", "work", "shots"].includes(name)
        || i + 1 >= argv.length) {
      console.error(`webbrowser.mjs: what is ${argv[i]}? See the top of the file.`);
      process.exit(2);
    }
    said[name] = path.isAbsolute(argv[i + 1]) || name === "chromium"
      ? argv[i + 1] : path.resolve(argv[i + 1]);
  }
  for (const wanted of ["site", "work"])
    if (!said[wanted]) {
      console.error(`webbrowser.mjs: --${wanted} is wanted.`);
      process.exit(2);
    }
  return said;
}

const said = options(process.argv.slice(2));
let passed = 0;
const failures = [];

// A host of the static site that answers 404 for what it has not: a copy
// of the site in `top`, its tree of C: with the test's files besides vc.
async function startSite(top) {
  const site = path.join(top, "site");
  rmSync(top, { recursive: true, force: true });
  cpSync(said.site, site, { recursive: true });
  const shipped = JSON.parse(readFileSync(path.join(site, "drives", "c.json"),
                                          "utf8"));
  const paths = shipped.entries.map((entry) => entry.path);
  must(["vc", "vc/vc.ini", "vc/vc.hlp", "autoexec.bat"]
         .every((one) => paths.includes(one)),
       `the site's C: has ${paths.join(", ")}`);
  const own = path.join(top, "c");
  mkdirSync(path.join(own, "vc"), { recursive: true });
  layFiles(own);
  writeFileSync(path.join(site, "drives", "c.json"),
                JSON.stringify(await treeOf(own)));
  const host = serve(site);
  await new Promise((done) => host.listen(0, "127.0.0.1", done));
  return { url: `http://127.0.0.1:${host.address().port}/`,
           stop: () => new Promise((done) => {
             host.close(done);
             host.closeAllConnections();
           }) };
}

// One check: `what` is said, and passes when `test` returns without
// throwing.
async function check(view, what, test, tab) {
  try {
    await test();
    passed++;
  } catch (caught) {
    failures.push(`${view}: ${what}: ${caught.message}`);
    console.log(`FAIL ${view}: ${what}: ${caught.message}`);
    if (tab) {
      try {
        await tab.shot(path.join(said.work, `failed-${view}-${failures.length}.png`));
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
// stopped, and the bar's line on how long changes last.
async function started(tab) {
  const rows = await tab.until((r, s) => s.includes("1Help") && s.includes("10Quit"),
                               30000, "the program's screen");
  must((await tab.text("ended")) === null, "the page says that it ended");
  const kept = await tab.text("kept");
  must(kept === "Changes last until the page is reloaded or closed.",
       `the bar says ${JSON.stringify(kept)}`);
  return rows;
}

// The right panel at C:\, where the test's files are, from the program's
// directory C:\VC, where the page starts, by .. where it is there.
async function upToRoot(tab) {
  const rows = await tab.until((r) => r[0].slice(40).includes(" C:\\"), 10000,
                               "C: in the right panel");
  if (rows[0].slice(40).includes(" C:\\VC ")) {
    await tab.press("Home");
    await tab.press("Enter");
  }
  return tab.until((r) => r[0].slice(40).includes(" C:\\ ")
                   && find(r, "readme", 40) !== null, 10000, "C:\\ in the right panel");
}

// The right panel at C:\, the cursor on CODE.
async function toFiles(tab) {
  await upToRoot(tab);
  await tab.press("Home");
  return tab.until((r) => rightStatus(r).includes("CODE"), 5000, "the cursor on CODE");
}

async function desktop(browser, served) {
  const tab = await browser.page(VIEWPORTS.desktop);
  const view = "desktop";
  const step = (what, test) => check(view, what, test, tab);
  await tab.go(served.url);
  await step("the page starts", () => started(tab));
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
  await step("the strip of the keys that the browser keeps, and its hint",
             async () => {
    const strip = await tab.eval("Array.from(document.querySelectorAll(" +
      "'#bar > #kept-keys button')).map((b) => b.textContent).join()");
    must(strip === "Ctrl-PgUp,Ctrl-PgDn,Ctrl-T,Ctrl-W,Ctrl-F4,Alt-F4",
         `the bar's strip is ${strip}`);
    const hint = await tab.text("kept-hint");
    must(hint && hint.includes("Full screen") && hint.includes("Chrome and Edge"),
         `the hint is ${JSON.stringify(hint)}`);
    const r = await tab.rect("#kept-keys");
    must(r.height <= 40, `the strip is ${r.height} high`);
  });
  // The browser's pointer, as the element that a place of the page hits
  // has it: none at cells of the grid, its corners among them, and in the
  // black around it, which in full screen is a wide band; its own over
  // the bar's buttons and the link to the licences.
  const pointers = `(() => {
    const at = (x, y) => { const e = document.elementFromPoint(x, y);
      return e ? \`\${e.id || e.tagName}:\${getComputedStyle(e).cursor}\` : "nothing"; };
    const s = document.getElementById("screen").getBoundingClientRect();
    const t = document.getElementById("stage").getBoundingClientRect();
    const hidden = [];
    for (const [fx, fy] of [[0.001, 0.001], [0.5, 0.5], [0.999, 0.999],
                            [0.05, 0.95], [0.95, 0.05], [0.5, 0.01]])
      hidden.push(at(s.left + fx * s.width, s.top + fy * s.height));
    hidden.push(at(t.left + 2, t.top + 2), at(t.right - 2, t.bottom - 2),
                at((t.left + s.left) / 2, (s.top + s.bottom) / 2));
    const shown = ["#full", "#upload", "#kept-keys button", "#licences-open"]
      .map((selector) => { const r = document.querySelector(selector)
        .getBoundingClientRect(); return at(r.left + r.width / 2, r.top + r.height / 2); });
    return JSON.stringify({ hidden, shown, full: !!document.fullscreenElement });
  })()`;
  const noPointer = async (full) => {
    const said = JSON.parse(await tab.eval(pointers));
    must(said.full === full, `the full screen is ${said.full}`);
    must(said.hidden.every((one) => one.endsWith(":none")),
         `over the grid and the black around it: ${said.hidden.join(", ")}`);
    must(said.shown.every((one) => !one.endsWith(":none")),
         `over the bar: ${said.shown.join(", ")}`);
  };
  await step("no pointer of the browser's over the grid, one over the bar",
             () => noPointer(false));
  await step("none over the grid in full screen either", async () => {
    const full = async (wanted) => {
      await tab.hit("#full");
      for (let tries = 0; tries < 50; tries++) {
        if (await tab.eval("!!document.fullscreenElement") === wanted)
          return;
        await sleep(100);
      }
    };
    await full(true);
    await noPointer(true);
    await full(false);
  });
  await shot(tab, `desktop-start`);
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
  await step("the page starts in C:\\VC, and C:\\ has the test's files", async () => {
    await tab.until((r) => r[0].slice(40).includes(" C:\\VC "), 5000, "C:\\VC");
    const rows = await toFiles(tab);
    for (const name of ["CODE", "DATA", "NOTES", "VC", "readme   txt"])
      must(find(rows, name, 40) !== null, `${name} is not listed`);
  });
  await shot(tab, `desktop-panels`);
  await step("Down, Down, Up", async () => {
    await tab.press("Down");
    await tab.press("Down");
    await tab.until((r) => rightStatus(r).includes("NOTES"), 5000, "NOTES");
    await tab.press("Up");
    await tab.until((r) => rightStatus(r).includes("DATA"), 5000, "DATA");
  });
  await step("Tab, and Tab again", async () => {
    await tab.press("Tab");
    await tab.until((r) => r[23].startsWith("C:\\VC>"), 5000, "C:\\VC's prompt");
    await tab.press("Tab");
    await tab.until((r) => r[23].startsWith("C:\\>"), 5000, "C:\\'s prompt");
  });
  // The mouse, each click followed by a key that has to take effect: the
  // program waits for a button to be let go by asking the mouse alone,
  // and the page froze there while that asking did not yield.
  await step("a click on a file, then Up", async () => {
    const at = find(await tab.settle(), "NOTES", 40);
    must(at !== null, "NOTES is not listed");
    await tab.hitCell(at.column + 1, at.row);
    await tab.until((r) => rightStatus(r).includes("NOTES"), 5000, "the cursor on NOTES");
    await tab.press("Up");
    await tab.until((r) => rightStatus(r).includes("DATA"), 5000, "the cursor on DATA");
  });
  await step("a double click on CODE: into it, then Enter on ..: out", async () => {
    const at = find(await tab.settle(), "CODE", 40);
    must(at !== null, "CODE is not listed");
    await tab.doubleClickCell(at.column + 1, at.row);
    await tab.until((r) => r[0].slice(40).includes("C:\\CODE"), 5000, "C:\\CODE");
    await tab.press("Enter");
    await tab.until((r) => !r[0].slice(40).includes("CODE")
                    && rightStatus(r).includes("CODE"), 5000, "C:\\ with the cursor on CODE");
  });
  await step("a click on the menu bar: the menu, then Esc", async () => {
    await tab.hitCell(45, 0);
    await tab.until((r) => r[0].includes("Files") && r[0].includes("Commands"), 5000,
                    "the menu bar");
    await tab.press("Escape");
    await tab.until((r) => !r[0].includes("Commands") && r[0].slice(40).includes(" C:\\ "),
                    5000, "the menu gone");
  });
  await step("a click on the key bar, then Down", async () => {
    await tab.hitCell(1, 24);
    await tab.settle();
    await tab.press("Down");
    await tab.until((r) => rightStatus(r).includes("DATA"), 5000, "the cursor on DATA");
    await tab.press("Up");
    await tab.until((r) => rightStatus(r).includes("CODE"), 5000, "the cursor on CODE");
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
    await shot(tab, `desktop-licences`);
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
  await step("a reload: the drive as it is served, the file gone", async () => {
    await tab.reload();
    await started(tab);
    const rows = await toFiles(tab);
    must(find(rows, "note     txt", 40) === null, "note.txt is still there");
    must(find(rows, "readme   txt", 40) !== null, "readme.txt is gone");
  });
  await step("the page keeps nothing in the browser", async () => {
    const kept = await tab.eval("(async () => {" +
      " const databases = indexedDB.databases ? await indexedDB.databases() : [];" +
      " return databases.length + localStorage.length + sessionStorage.length; })()");
    must(kept === 0, `the browser has ${kept} things of the page's`);
  });
  await step("the console has no errors", () => {
    const wrong = tab.errors();
    must(wrong.length === 0, wrong.map((e) => e.text).join("; "));
  });
  await tab.close();
}

async function phone(browser, served) {
  const tab = await browser.page(VIEWPORTS.portrait);
  let view = "portrait";
  const step = (what, test) => check(view, what, test, tab);
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
  await step("the page starts", () => started(tab));
  await step("the row of keys shows", async () => {
    must(!(await tab.eval("document.getElementById('keys').hidden")), "it is hidden");
    const keys = await tab.eval("document.querySelectorAll('#keys button').length");
    must(keys === 27, `it has ${keys} keys`);
    const strip = await tab.eval("Array.from(document.querySelectorAll(" +
      "'#keys #kept-keys button')).map((b) => b.textContent).join()");
    must(strip === "Ctrl-PgUp,Ctrl-PgDn,Ctrl-T,Ctrl-W,Ctrl-F4,Alt-F4",
         `the row's strip is ${strip}`);
    const last = await tab.eval("document.querySelector('#keys button:last-child').textContent");
    must(last === "Enter", `its last key is ${last}`);
  });
  await step("the grid fits the width, nothing cut off", () => fits(390, 844));
  // Ctrl-F1 of the row takes the left panel away and brings it back, or
  // the other way round.
  const panels = (r) => (r[0].match(/╔/g) || []).length;
  await step("Ctrl and F1 of the row, twice: the left panel off and on", async () => {
    const before = panels(await tab.settle());
    for (const now of [3 - before, before]) {
      await tab.hit('[data-latch="ctrl"]');
      await tab.hit("#keys button:nth-child(3)");
      await tab.until((r) => panels(r) === now, 5000, `${now} panels`);
    }
  });
  await step("Alt and F2 of the row, then c: C:, and up to C:\\", async () => {
    await tab.hit('[data-latch="alt"]');
    await tab.hit("#keys button:nth-child(4)");
    await tab.settle();
    await tab.press("c");
    await upToRoot(tab);
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
    await tab.until((r) => r[0].slice(40).includes("C:\\CODE"), 5000, "C:\\CODE");
  });
  await step("a tap on the key bar, then Enter of the row on ..: out", async () => {
    await tab.hitCell(1, 24);
    await tab.settle();
    await tab.hit("#keys button:last-child");
    await tab.until((r) => !r[0].slice(40).includes("CODE")
                    && rightStatus(r).includes("CODE"), 5000, "C:\\ with the cursor on CODE");
  });
  await step("the row of keys whole, each key tappable", () => whole(390, 844));
  await shot(tab, "portrait");
  view = "landscape";
  await step("turned: the grid refits", async () => {
    await tab.view(VIEWPORTS.landscape);
    await sleep(800);
    await fits(844, 390);
    await tab.settle();
  });
  await step("turned: the row of keys whole, each key tappable", () => whole(844, 390));
  await shot(tab, "landscape");
  await step("the console has no errors", () => {
    const wrong = tab.errors();
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
    const served = await startSite(path.join(said.work, "site"));
    try {
      for (const run of [desktop, phone]) {
        try {
          await run(browser, served);
        } catch {
          // said already; on to the next
        }
      }
    } finally {
      await served.stop();
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
