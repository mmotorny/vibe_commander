// The session of the README's animation, played on the web page in a real
// browser (harness/session.py, which runs this and draws what it writes):
// the site that the build made, build/portable_c/web/static, as it is
// published, with C:\VC and C:\ as they are shipped and A: empty, served
// by src/portable_c/web/serve.mjs; in the Chromium that the machine has,
// headless, driven as harness/browser.mjs drives it, with the keys and the
// mouse as a keyboard and a mouse send them to the page.
//
//   node harness/websession.mjs --chromium chromium \
//       --site build/portable_c/web/static --work build/session \
//       --steps STEPS.json --stamp MS --out FRAMES.json
//
// STEPS.json is a list of steps, each { press, ctrl, alt, shift } for a
// key, { type } for text typed a character at a time, or { mouse:
// [column, row] } for the pointer moved onto a cell, and `seconds`, how
// long the frame behind it is shown; a step with none of them is the page
// as it is. Behind each step the page's clock is let run for `run`
// milliseconds, 1000 where it says none, and the screen is taken from the
// page's own cells: each row's runs, which the page draws as spans of the
// class of their attribute (render.js), and its cursor. A step may say
// `quantum`, how far the clock goes at each look while it runs (below),
// so that a copy that the page makes in an instant is caught with its
// box. FRAMES.json is a list of { rows, cursor, seconds }, a row a list
// of 80 cells [character, attribute] and the cursor [column, row, block]
// or null.
//
// So that the same steps make the same frames, the page's clock is the
// time MS of --stamp (milliseconds since 1970, UTC) when the page loads,
// in UTC, and from the program's start a clock of the session's own
// (clockAt): every file of C: is of that time, and so is what the
// session makes, and the program goes round as often between two steps
// at every run, so its random numbers, the screen saver's stars, and the
// colon of its clock are the same. Every process that it starts it ends:
// Chromium, the host.

import { cpSync, mkdirSync, readFileSync, rmSync, writeFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { Browser, VIEWPORTS, sleep } from "./browser.mjs";

const here = path.dirname(fileURLToPath(import.meta.url));
const web = path.join(here, "..", "src", "portable_c", "web");

function options(argv) {
  const said = { chromium: "chromium" };
  const names = ["chromium", "site", "work", "steps", "stamp", "out"];
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

// The site as the build made it, build/portable_c/web/static (--site),
// copied into `work`, with every file and directory of its C: of the time
// of the stamp, served on a free port.
async function startSite(work, built, stamp) {
  const { serve } = await import(path.join(web, "serve.mjs"));
  const site = path.join(work, "site");
  rmSync(site, { recursive: true, force: true });
  cpSync(built, site, { recursive: true });
  const drive = path.join(site, "drives", "c.json");
  const c = JSON.parse(readFileSync(drive, "utf8"));
  for (const entry of c.entries)
    entry.mtime = stamp;
  c.mtime = stamp;
  writeFileSync(drive, JSON.stringify(c));
  const host = serve(site);
  await new Promise((done) => host.listen(0, "127.0.0.1", done));
  return { url: `http://127.0.0.1:${host.address().port}/`,
           stop: () => new Promise((done) => {
             host.close(done);
             host.closeAllConnections();
           }) };
}

// The page's clock, put in before any script of the page runs: the time
// of the stamp, and from the program's first look at the clock on a clock
// of the session's own, as the bench's machine counts its time in what it
// executes. That clock goes on by QUANTUM at every look, and to the time of
// a timer of the page when the timer is due; a timer is due only when the
// session lets the clock run (__session.run), never by itself. So the
// program goes round the same number of times between two keys at every
// run, its random numbers, the stars, are the same, and so is the colon
// of the clock, which shows in the second half of a second.
const QUANTUM = 0.05;                   // milliseconds
const CLAMP = 4;                        // the least wait of a timer, as
                                        // a browser has it for timers
                                        // that set timers
function clockAt(stamp) {
  return `(() => {
    const Real = Date, realNow = performance.now.bind(performance);
    const realSet = setTimeout, realClear = clearTimeout;
    const shift = ${stamp} - Real.now();
    let own = false, now = 0, made = 0, serial = 1e9, quantum = ${QUANTUM};
    const due = [], channel = new MessageChannel();
    const look = () => { now += quantum; return now; };
    class Moved extends Real {
      constructor(...given) {
        if (given.length === 0) super(Moved.now()); else super(...given);
      }
      static now() { return own ? ${stamp} + Math.floor(look()) : Real.now() + shift; }
    }
    globalThis.Date = Moved;
    performance.now = () => {
      if (!own)
        own = true;
      return look();
    };
    globalThis.setTimeout = (call, wait = 0, ...given) => {
      if (!own)
        return realSet(call, wait, ...given);
      const id = ++serial;
      due.push({ at: now + Math.max(${CLAMP}, Number(wait) || 0), made: made++, id,
                 call, given });
      return id;
    };
    globalThis.clearTimeout = (id) => {
      const at = due.findIndex((timer) => timer.id === id);
      if (at >= 0) due.splice(at, 1); else realClear(id);
    };
    globalThis.__session = {
      started: () => own,
      // The clock let run for ms: every timer that is due by then called,
      // in the order of its time, and of its making, each once what the
      // one before has set going has run, which Emscripten's Asyncify
      // goes on with in a promise of its own.
      async run(ms, step = ${QUANTUM}) {
        quantum = step;
        const until = now + ms;
        for (let count = 0; ; count++) {
          if (count > 1000000)
            throw new Error("the page's timers do not end");
          let first = -1;
          for (let i = 0; i < due.length; i++)
            if (due[i].at <= until && (first < 0 || due[i].at < due[first].at
                || due[i].at === due[first].at && due[i].made < due[first].made))
              first = i;
          if (first < 0)
            break;
          const [timer] = due.splice(first, 1);
          if (timer.at > now)
            now = timer.at;
          timer.call(...timer.given);
          await new Promise((done) => {
            channel.port1.onmessage = done;
            channel.port2.postMessage(0);
          });
        }
        quantum = ${QUANTUM};
        if (now < until)
          now = until;
        return now;
      },
    };
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
  const taken = await tab.eval(CELLS);
  if (taken.rows.length !== 25 || taken.rows.some((row) => row.length !== 80))
    throw new Error(`the screen is not 80 by 25: ${taken.rows.map((r) => r.length)}`);
  return taken;
}

// The page's clock let run for ms, in pieces that keep the browser
// answering.
async function run(tab, ms, quantum = QUANTUM) {
  for (let left = ms; left > 0; left -= 1000)
    await tab.eval(`__session.run(${Math.min(left, 1000)}, ${quantum})`);
}

async function main() {
  const said = options(process.argv.slice(2));
  const steps = JSON.parse(readFileSync(said.steps, "utf8"));
  mkdirSync(said.work, { recursive: true });
  rmSync(path.join(said.work, "profile"), { recursive: true, force: true });
  const served = await startSite(said.work, said.site, said.stamp);
  let browser = null;
  try {
    browser = await Browser.start(said.chromium, path.join(said.work, "profile"));
    const tab = await browser.page(VIEWPORTS.desktop);
    await tab.send("Emulation.setTimezoneOverride", { timezoneId: "UTC" });
    await tab.send("Page.addScriptToEvaluateOnNewDocument",
                   { source: clockAt(said.stamp) });
    await tab.go(served.url);
    const end = Date.now() + 30000;
    while (!await tab.eval("!!globalThis.__session && __session.started()")) {
      if (Date.now() > end)
        throw new Error("the program did not start in 30 s");
      await sleep(100);
    }
    await run(tab, 1000);
    await tab.until((r, s) => s.includes("1Help") && s.includes("10Quit")
                    && r[0].includes(" C:\\VC "), 1000, "the program in C:\\VC");
    const frames = [];
    for (const step of steps) {
      if (step.press !== undefined)
        await tab.press(step.press, { ctrl: !!step.ctrl, alt: !!step.alt,
                                      shift: !!step.shift });
      if (step.mouse !== undefined) {
        const [column, row] = step.mouse;
        const at = await tab.cellAt(column, row);
        await tab.send("Input.dispatchMouseEvent",
                       { type: "mouseMoved", x: at.x, y: at.y });
      }
      if (step.type !== undefined)
        for (const character of step.type) {
          await tab.press(character);
          await run(tab, 150);
        }
      await run(tab, step.run ?? 1000, step.quantum ?? QUANTUM);
      frames.push({ ...(await frame(tab)), seconds: step.seconds });
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
