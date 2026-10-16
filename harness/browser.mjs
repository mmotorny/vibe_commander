// A headless Chromium driven over its DevTools protocol, for the web page
// (CLAUDE.md, "The web page"; TODO.md, "W6"): Node's own modules alone,
// its WebSocket among them, nothing from npm. The Chromium is the one that
// the machine has, which harness/webbrowser.mjs finds on the path.
//
// `Browser.start` runs it with a profile of its own in a directory that it
// is given, which has to be one that it may read and write: Chromium of a
// snap reads no hidden directory and not the real /tmp, so the callers
// give it one in build/. `Browser.page` opens a tab with a viewport, and
// `Tab` has what a test does with one: go to a URL, read the <pre> of the
// screen and the page's other texts, wait for a text, type keys as a
// keyboard sends them, click and tap, take a screenshot, and keep what the
// console said and what tabs the page opened.

import { spawn } from "node:child_process";
import { existsSync, mkdirSync, readFileSync, writeFileSync } from "node:fs";
import path from "node:path";

export const sleep = (ms) => new Promise((done) => setTimeout(done, ms));

// The iPhone that the phone's viewports are: an iPhone 15's screen in CSS
// pixels, and Safari's agent of iOS 18.
export const IPHONE_AGENT = "Mozilla/5.0 (iPhone; CPU iPhone OS 18_0 like " +
  "Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.0 " +
  "Mobile/15E148 Safari/604.1";
export const VIEWPORTS = {
  desktop: { width: 1280, height: 800, scale: 1, mobile: false },
  portrait: { width: 390, height: 844, scale: 3, mobile: true,
              orientation: { type: "portraitPrimary", angle: 0 } },
  landscape: { width: 844, height: 390, scale: 3, mobile: true,
               orientation: { type: "landscapePrimary", angle: 90 } },
};

// CDP's bits of the modifiers.
const MODIFIERS = { alt: 1, ctrl: 2, meta: 4, shift: 8 };
const HELD = { alt: ["AltLeft", "Alt", 18], ctrl: ["ControlLeft", "Control", 17],
               shift: ["ShiftLeft", "Shift", 16] };

// A key by the name that a test writes, as a US keyboard has it: its
// `code`, its `key` and the virtual key code of Windows, which Chromium
// wants for a key that types nothing.
const NAMED = {
  Enter: ["Enter", "Enter", 13, "\r"], Tab: ["Tab", "Tab", 9],
  Escape: ["Escape", "Escape", 27], Esc: ["Escape", "Escape", 27],
  Backspace: ["Backspace", "Backspace", 8], Delete: ["Delete", "Delete", 46],
  Insert: ["Insert", "Insert", 45], Home: ["Home", "Home", 36],
  End: ["End", "End", 35], PageUp: ["PageUp", "PageUp", 33],
  PageDown: ["PageDown", "PageDown", 34], Up: ["ArrowUp", "ArrowUp", 38],
  Down: ["ArrowDown", "ArrowDown", 40], Left: ["ArrowLeft", "ArrowLeft", 37],
  Right: ["ArrowRight", "ArrowRight", 39],
};
for (let n = 1; n <= 12; n++)
  NAMED[`F${n}`] = [`F${n}`, `F${n}`, 111 + n];

// The `code` of a character typed on a US keyboard, and whether it wants
// Shift.
const SHIFTED = { "!": "1", "@": "2", "#": "3", "$": "4", "%": "5",
  "^": "6", "&": "7", "*": "8", "(": "9", ")": "0", "_": "-", "+": "=",
  "{": "[", "}": "]", "|": "\\", ":": ";", "\"": "'", "<": ",", ">": ".",
  "?": "/", "~": "`" };
const PUNCTUATION = { "-": "Minus", "=": "Equal", "[": "BracketLeft",
  "]": "BracketRight", "\\": "Backslash", ";": "Semicolon", "'": "Quote",
  ",": "Comma", ".": "Period", "/": "Slash", "`": "Backquote", " ": "Space" };

function characterKey(character) {
  let base = character, shift = false;
  if (SHIFTED[character]) {
    base = SHIFTED[character];
    shift = true;
  } else if (character >= "A" && character <= "Z") {
    base = character.toLowerCase();
    shift = true;
  }
  let code;
  if (base >= "a" && base <= "z")
    code = `Key${base.toUpperCase()}`;
  else if (base >= "0" && base <= "9")
    code = `Digit${base}`;
  else if (PUNCTUATION[base])
    code = PUNCTUATION[base];
  else
    throw new Error(`no key types ${JSON.stringify(character)}`);
  const virtual = base >= "a" && base <= "z" ? base.toUpperCase().charCodeAt(0)
    : base >= "0" && base <= "9" ? base.charCodeAt(0) : 0;
  return { code, key: character, virtual, text: character, shift };
}

// One connection to the browser, over which every tab is a session.
class Connection {
  constructor(socket) {
    this.socket = socket;
    this.next = 1;
    this.waiting = new Map();
    this.listeners = new Set();
    socket.addEventListener("message", (event) => {
      const message = JSON.parse(event.data);
      if (message.id !== undefined) {
        const waiter = this.waiting.get(message.id);
        this.waiting.delete(message.id);
        if (!waiter)
          return;
        if (message.error)
          waiter.fail(new Error(`${waiter.method}: ${message.error.message}`));
        else
          waiter.done(message.result);
        return;
      }
      for (const listener of this.listeners)
        listener(message);
    });
  }

  send(method, params = {}, sessionId = undefined) {
    const id = this.next++;
    const message = { id, method, params };
    if (sessionId)
      message.sessionId = sessionId;
    // A command that is not answered in a minute fails, so that a run
    // ends where Chromium has gone astray.
    return new Promise((done, fail) => {
      const timer = setTimeout(() => {
        this.waiting.delete(id);
        fail(new Error(`${method}: no answer from Chromium`));
      }, 60000);
      this.waiting.set(id, { method,
        done: (value) => { clearTimeout(timer); done(value); },
        fail: (error) => { clearTimeout(timer); fail(error); } });
      this.socket.send(JSON.stringify(message));
    });
  }
}

export class Browser {
  // Chromium, headless, with its profile in `profile`, and a connection
  // to it.
  static async start(chromium, profile) {
    mkdirSync(profile, { recursive: true });
    const portFile = path.join(profile, "DevToolsActivePort");
    if (existsSync(portFile))
      writeFileSync(portFile, "");
    const child = spawn(chromium, [
      "--headless=new", "--remote-debugging-port=0",
      `--user-data-dir=${profile}`, "--no-first-run",
      "--no-default-browser-check", "--disable-extensions",
      "--disable-background-networking", "--disable-sync",
      "--disable-component-update", "--disable-default-apps",
      "--hide-scrollbars", "--mute-audio", "--password-store=basic",
      "--use-mock-keychain",
      "about:blank"], { stdio: ["ignore", "ignore", "pipe"] });
    let said = "";
    child.stderr.on("data", (chunk) => { said += chunk; });
    let address = null;
    for (let tries = 0; tries < 300 && address === null; tries++) {
      if (child.exitCode !== null)
        throw new Error(`Chromium ended at once: ${said.slice(-2000)}`);
      try {
        const lines = readFileSync(portFile, "utf8").split("\n");
        if (lines.length >= 2 && lines[1].startsWith("/devtools/"))
          address = `ws://127.0.0.1:${lines[0]}${lines[1]}`;
      } catch {
        // not written yet
      }
      if (address === null)
        await sleep(100);
    }
    if (address === null) {
      child.kill();
      throw new Error(`Chromium gave no DevTools port: ${said.slice(-2000)}`);
    }
    const socket = new WebSocket(address);
    await new Promise((done, fail) => {
      socket.addEventListener("open", done, { once: true });
      socket.addEventListener("error", () => fail(new Error("no WebSocket")),
                              { once: true });
    });
    const browser = new Browser(child, new Connection(socket));
    browser.version = (await browser.connection.send("Browser.getVersion")).product;
    return browser;
  }

  constructor(child, connection) {
    this.child = child;
    this.connection = connection;
    this.opened = [];             // the tabs that a page opened, by URL
    connection.listeners.add((message) => {
      if (message.method === "Target.targetCreated") {
        const info = message.params.targetInfo;
        if (info.type === "page" && info.openerId)
          this.opened.push(info);
      }
    });
  }

  // A new tab with the viewport `view`, one of VIEWPORTS.
  async page(view) {
    await this.connection.send("Target.setDiscoverTargets", { discover: true });
    const { targetId } = await this.connection.send("Target.createTarget",
                                                    { url: "about:blank" });
    const { sessionId } = await this.connection.send("Target.attachToTarget",
                                                     { targetId, flatten: true });
    const tab = new Tab(this, targetId, sessionId);
    await tab.setUp(view);
    return tab;
  }

  async close() {
    // Chromium may close its socket before it answers: the answer is not
    // waited for long.
    await Promise.race([this.connection.send("Browser.close").catch(() => {}),
                        sleep(2000)]);
    this.connection.socket.close();
    for (let tries = 0; tries < 50 && this.child.exitCode === null; tries++)
      await sleep(100);
    if (this.child.exitCode === null)
      this.child.kill("SIGKILL");
  }
}

export class Tab {
  constructor(browser, targetId, sessionId) {
    this.browser = browser;
    this.targetId = targetId;
    this.sessionId = sessionId;
    this.console = [];            // { level, text }
    this.touch = false;
    browser.connection.listeners.add((message) => {
      if (message.sessionId !== sessionId)
        return;
      const p = message.params;
      if (message.method === "Runtime.consoleAPICalled")
        this.console.push({ level: p.type, text: p.args.map((a) =>
          a.value !== undefined ? String(a.value) : a.description || a.type).join(" ") });
      else if (message.method === "Runtime.exceptionThrown")
        this.console.push({ level: "exception", text:
          p.exceptionDetails.exception?.description || p.exceptionDetails.text });
      else if (message.method === "Log.entryAdded")
        this.console.push({ level: p.entry.level, text:
          `${p.entry.text}${p.entry.url ? ` (${p.entry.url})` : ""}` });
    });
  }

  send(method, params = {}) {
    return this.browser.connection.send(method, params, this.sessionId);
  }

  async setUp(view) {
    await this.send("Page.enable");
    await this.send("Runtime.enable");
    await this.send("Log.enable");
    await this.view(view);
  }

  // The viewport, and for a phone its touch and its agent.
  async view(view) {
    const metrics = { width: view.width, height: view.height,
                      deviceScaleFactor: view.scale, mobile: view.mobile };
    if (view.orientation)
      metrics.screenOrientation = view.orientation;
    await this.send("Emulation.setDeviceMetricsOverride", metrics);
    this.touch = view.mobile;
    await this.send("Emulation.setTouchEmulationEnabled",
                    { enabled: view.mobile, maxTouchPoints: view.mobile ? 5 : 1 });
    if (view.mobile)
      await this.send("Emulation.setUserAgentOverride",
                      { userAgent: IPHONE_AGENT, platform: "iPhone" });
  }

  async go(url) {
    await this.send("Page.navigate", { url });
  }

  async reload() {
    await this.send("Page.reload", { ignoreCache: true });
  }

  // What an expression of the page's comes to.
  async eval(expression) {
    const { result, exceptionDetails } = await this.send("Runtime.evaluate",
      { expression, returnByValue: true, awaitPromise: true });
    if (exceptionDetails)
      throw new Error(`${expression}: ${exceptionDetails.exception?.description
                                        || exceptionDetails.text}`);
    return result.value;
  }

  // The 25 rows of the screen, as the <pre> has them.
  async screen() {
    const text = await this.eval(
      "(() => { const s = document.getElementById('screen');" +
      " return s ? Array.from(s.children).filter((e) => e.tagName === 'SPAN')" +
      ".map((e) => e.textContent) : []; })()");
    return text;
  }

  async text(id) {
    return this.eval(`(() => { const e = document.getElementById(${JSON.stringify(id)});` +
                     " return e && !e.hidden ? e.textContent : null; })()");
  }

  // Waits until `test` of the screen's rows is true, and returns them; or
  // throws with the screen as it is after `ms`.
  async until(test, ms = 15000, what = "the screen") {
    const end = Date.now() + ms;
    let rows = [];
    while (Date.now() < end) {
      rows = await this.screen();
      if (test(rows, rows.join("\n")))
        return rows;
      const ended = await this.text("ended");
      if (ended)
        throw new Error(`the page says: ${ended}`);
      await sleep(100);
    }
    throw new Error(`${what} did not come in ${ms} ms; the screen:\n${rows.join("\n")}`);
  }

  // The screen once it has stayed the same for `quiet` ms.
  async settle(quiet = 400, ms = 10000) {
    const end = Date.now() + ms;
    let last = (await this.screen()).join("\n"), since = Date.now();
    while (Date.now() < end) {
      await sleep(100);
      const now = (await this.screen()).join("\n");
      if (now !== last) {
        last = now;
        since = Date.now();
      } else if (Date.now() - since >= quiet) {
        break;
      }
    }
    return last.split("\n");
  }

  async keyEvent(type, key, modifiers) {
    const params = { type, modifiers, code: key.code, key: key.key,
                     windowsVirtualKeyCode: key.virtual,
                     nativeVirtualKeyCode: key.virtual };
    if (type === "keyDown" && key.text) {
      params.type = "keyDown";
      params.text = key.text;
      params.unmodifiedText = key.text;
    }
    if (type === "keyDown" && key.location)
      params.location = key.location;
    await this.send("Input.dispatchKeyEvent", params);
  }

  // A key pressed and let go as a keyboard has it, the modifiers that it
  // is pressed with going down first and up last: `press("F2", { alt:
  // true })`, `press("a")`.
  async press(name, held = {}) {
    let key;
    if (NAMED[name]) {
      const [code, k, virtual, text] = NAMED[name];
      key = { code, key: k, virtual, text };
    } else {
      key = characterKey(name);
      if (key.shift)
        held = { ...held, shift: true };
    }
    if (held.ctrl || held.alt)
      key.text = undefined;
    let bits = 0;
    for (const which of ["ctrl", "alt", "shift"]) {
      if (!held[which])
        continue;
      bits |= MODIFIERS[which];
      const [code, k, virtual] = HELD[which];
      await this.keyEvent("rawKeyDown", { code, key: k, virtual, location: 1 }, bits);
    }
    await this.keyEvent(key.text ? "keyDown" : "rawKeyDown", key, bits);
    await this.keyEvent("keyUp", key, bits);
    for (const which of ["shift", "alt", "ctrl"]) {
      if (!held[which])
        continue;
      bits &= ~MODIFIERS[which];
      const [code, k, virtual] = HELD[which];
      await this.keyEvent("keyUp", { code, key: k, virtual, location: 1 }, bits);
    }
  }

  async type(text) {
    for (const character of text)
      await this.press(character);
  }

  // Where a cell of the screen is in the viewport, its middle.
  async cellAt(column, row) {
    return this.eval("(() => { const r = document.getElementById('screen')" +
      ".getBoundingClientRect(); return { x: r.left + (" + column + " + 0.5) * r.width / 80," +
      " y: r.top + (" + row + " + 0.5) * r.height / 25 }; })()");
  }

  async rect(selector) {
    return this.eval(`(() => { const e = document.querySelector(${JSON.stringify(selector)});` +
      " if (!e) return null; const r = e.getBoundingClientRect();" +
      " return { left: r.left, top: r.top, right: r.right, bottom: r.bottom," +
      " width: r.width, height: r.height }; })()");
  }

  // A tap of a finger, or a click of the left button, at a place of the
  // viewport.
  async tapAt(x, y) {
    await this.send("Input.dispatchTouchEvent",
                    { type: "touchStart", touchPoints: [{ x, y }] });
    await sleep(60);
    await this.send("Input.dispatchTouchEvent",
                    { type: "touchEnd", touchPoints: [] });
  }

  async clickAt(x, y, button = "left") {
    await this.send("Input.dispatchMouseEvent", { type: "mouseMoved", x, y });
    await this.send("Input.dispatchMouseEvent",
                    { type: "mousePressed", x, y, button, buttons: 1, clickCount: 1 });
    await sleep(40);
    await this.send("Input.dispatchMouseEvent",
                    { type: "mouseReleased", x, y, button, buttons: 0, clickCount: 1 });
  }

  // A cell of the screen tapped, or clicked where there is no touch.
  async hitCell(column, row) {
    const { x, y } = await this.cellAt(column, row);
    if (this.touch)
      await this.tapAt(x, y);
    else
      await this.clickAt(x, y);
  }

  // An element of the page tapped or clicked in its middle.
  async hit(selector) {
    const r = await this.rect(selector);
    if (r === null)
      throw new Error(`no ${selector} on the page`);
    const x = r.left + r.width / 2, y = r.top + r.height / 2;
    if (this.touch)
      await this.tapAt(x, y);
    else
      await this.clickAt(x, y);
  }

  // A screenshot into `file`, of the whole viewport or of `clip`, a
  // rectangle of it, { x, y, width, height }, magnified by `zoom`.
  async shot(file, clip = null, zoom = 1) {
    const params = { format: "png" };
    if (clip !== null)
      params.clip = { ...clip, scale: zoom };
    const { data } = await this.send("Page.captureScreenshot", params);
    mkdirSync(path.dirname(file), { recursive: true });
    writeFileSync(file, Buffer.from(data, "base64"));
  }

  // What the console has said that is an error.
  errors() {
    return this.console.filter((m) => m.level === "error" || m.level === "exception");
  }

  async close() {
    await this.browser.connection.send("Target.closeTarget", { targetId: this.targetId });
  }
}
