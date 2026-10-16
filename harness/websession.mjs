// The session of the README's animation, played on the web page in a real
// browser (harness/session.py, which runs this and draws what it writes):
// the page build/portable_c/web/vc.html as its server serves it, with its
// one drive, C:, labelled VC, which holds the directory vc alone, where
// the page starts; in the Chromium that the machine has, headless, driven
// as harness/browser.mjs drives it, with the keys as a keyboard sends
// them to the page.
//
//   node harness/websession.mjs --chromium chromium \
//       --page build/portable_c/web/vc.html --work build/session \
//       --steps STEPS.json --stamp MS --out FRAMES.json
//
// STEPS.json is a list of steps, each { press, ctrl, alt, shift } for a
// key or { type } for text typed a character at a time, and `seconds`,
// how long the frame behind it is shown; a step with neither is the page
// as it is. Behind each step the screen is taken once it has stayed the
// same for a while, from the page's own cells, the mouse's pointer moved to
// the last cell of the key bar first: each row's runs, which the
// page draws as spans of the class of their attribute (render.js), and
// its cursor. FRAMES.json is a list of { rows, cursor, seconds }, a row a
// list of 80 cells [character, attribute] and the cursor [column, row,
// block] or null.
//
// So that the same steps make the same frames, the page's clock is the
// time MS of --stamp (milliseconds since 1970, UTC) when the page loads,
// and goes on from there, in UTC: the file that the session makes, and
// its directory, are of that time, and so is the drive's vc, which the
// server's directory has at that time. Every process that it starts it
// ends: Chromium, the server.

import { spawn } from "node:child_process";
import { createServer } from "node:http";
import { mkdirSync, readFileSync, rmSync, utimesSync, writeFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { Browser, VIEWPORTS, sleep } from "./browser.mjs";

const here = path.dirname(fileURLToPath(import.meta.url));
const web = path.join(here, "..", "src", "portable_c", "web");

function options(argv) {
  const said = { chromium: "chromium" };
  const names = ["chromium", "page", "work", "steps", "stamp", "out"];
  for (let i = 0; i < argv.length; i += 2) {
    const name = argv[i].replace(/^--/, "");
    if (!names.includes(name) || i + 1 >= argv.length) {
      console.error(`websession.mjs: what is ${argv[i]}? See the top of the file.`);
      process.exit(2);
    }
    said[name] = name === "chromium" || name === "stamp" || path.isAbsolute(argv[i + 1])
      ? argv[i + 1] : path.resolve(argv[i + 1]);
  }
  for (const wanted of names)
    if (said[wanted] === undefined) {
      console.error(`websession.mjs: --${wanted} is wanted.`);
      process.exit(2);
    }
  said.stamp = Number(said.stamp);
  return said;
}

async function freePort() {
  const probe = createServer();
  await new Promise((done) => probe.listen(0, "127.0.0.1", done));
  const { port } = probe.address();
  await new Promise((done) => probe.close(done));
  return port;
}

// The page's server on a free port, with C: a directory of the work's,
// made afresh with its vc alone, both of the time of the stamp.
async function startServer(drive, page, stamp) {
  rmSync(drive, { recursive: true, force: true });
  mkdirSync(path.join(drive, "vc"), { recursive: true });
  for (const made of [path.join(drive, "vc"), drive])
    utimesSync(made, stamp / 1000, stamp / 1000);
  const port = await freePort();
  const child = spawn(process.execPath, [path.join(web, "server.mjs"),
    "--port", String(port), "--page", page, "--drive-c", drive],
    { stdio: ["ignore", "pipe", "pipe"] });
  let output = "";
  child.stdout.on("data", (chunk) => { output += chunk; });
  child.stderr.on("data", (chunk) => { output += chunk; });
  const url = `http://127.0.0.1:${port}/`;
  const stop = async () => {
    if (child.exitCode !== null)
      return;
    child.kill();
    for (let tries = 0; tries < 50 && child.exitCode === null; tries++)
      await sleep(100);
    if (child.exitCode === null)
      child.kill("SIGKILL");
  };
  for (let tries = 0; tries < 100; tries++) {
    if (child.exitCode !== null)
      throw new Error(`the server ended: ${output}`);
    try {
      if ((await fetch(`${url}api/server`)).ok)
        return { url, stop };
    } catch {
      // not listening yet
    }
    await sleep(100);
  }
  await stop();
  throw new Error(`the server did not answer: ${output}`);
}

// The page's clock, put in before any script of the page runs: Date as it
// is, moved by as much as makes now the stamp.
function clockAt(stamp) {
  return `(() => {
    const Real = Date, shift = ${stamp} - Real.now();
    class Moved extends Real {
      constructor(...given) {
        if (given.length === 0) super(Real.now() + shift); else super(...given);
      }
      static now() { return Real.now() + shift; }
    }
    globalThis.Date = Moved;
  })();`;
}

// The screen as the page has drawn it: the cells of each row, by its
// runs' classes, and the cursor by where its element is.
const CELLS = `(() => {
  const screen = document.getElementById("screen");
  const rows = Array.from(screen.children).filter((e) => e.tagName === "SPAN")
    .map((row) => {
      const cells = [];
      for (const run of row.children) {
        const named = /\\bvc-a([0-9a-f]{2})\\b/.exec(run.className);
        const attribute = named ? parseInt(named[1], 16) : 0x07;
        for (const character of Array.from(run.textContent))
          cells.push([character, attribute]);
      }
      return cells;
    });
  const box = screen.getBoundingClientRect();
  const width = box.width / 80, height = box.height / 25;
  const cursor = document.getElementById("cursor");
  let place = null;
  if (getComputedStyle(cursor).display !== "none") {
    const tall = parseFloat(cursor.style.height);
    place = [Math.round(parseFloat(cursor.style.left) / width),
             Math.round((parseFloat(cursor.style.top) + tall) / height) - 1,
             tall >= height - 0.5];
  }
  return { rows, cursor: place };
})()`;

async function frame(tab) {
  await tab.settle(500);
  const taken = await tab.eval(CELLS);
  if (taken.rows.length !== 25 || taken.rows.some((row) => row.length !== 80))
    throw new Error(`the screen is not 80 by 25: ${taken.rows.map((r) => r.length)}`);
  return taken;
}

async function main() {
  const said = options(process.argv.slice(2));
  const steps = JSON.parse(readFileSync(said.steps, "utf8"));
  mkdirSync(said.work, { recursive: true });
  rmSync(path.join(said.work, "profile"), { recursive: true, force: true });
  const served = await startServer(path.join(said.work, "c"), said.page, said.stamp);
  let browser = null;
  try {
    browser = await Browser.start(said.chromium, path.join(said.work, "profile"));
    const tab = await browser.page(VIEWPORTS.desktop);
    await tab.send("Emulation.setTimezoneOverride", { timezoneId: "UTC" });
    await tab.send("Page.addScriptToEvaluateOnNewDocument",
                   { source: clockAt(said.stamp) });
    await tab.go(served.url);
    await tab.until((r, s) => s.includes("1Help") && s.includes("10Quit")
                    && r[0].includes(" C:\\VC "), 30000, "the program in C:\\VC");
    // The mouse's pointer, which the program draws in the middle of the
    // screen where it starts, moved to the corner of the key bar, where
    // it covers nothing of what the session shows.
    const corner = await tab.cellAt(79, 24);
    await tab.send("Input.dispatchMouseEvent",
                   { type: "mouseMoved", x: corner.x, y: corner.y });
    await tab.until((r) => r[24].length === 80, 5000, "the key bar");
    const frames = [];
    for (const step of steps) {
      if (step.press !== undefined)
        await tab.press(step.press, { ctrl: !!step.ctrl, alt: !!step.alt,
                                      shift: !!step.shift });
      if (step.type !== undefined)
        for (const character of step.type) {
          // As a person types: the editor clears what is typed ahead of
          // it, as the original does with Clear keyboard buffer on.
          await tab.press(character);
          await tab.settle(150);
        }
      const taken = await frame(tab);
      frames.push({ ...taken, seconds: step.seconds });
    }
    const wrong = tab.errors();
    if (wrong.length > 0)
      throw new Error(`the console: ${wrong.map((e) => e.text).join("; ")}`);
    writeFileSync(said.out, JSON.stringify(frames));
  } finally {
    if (browser !== null)
      await browser.close();
    await served.stop();
  }
}

await main();
