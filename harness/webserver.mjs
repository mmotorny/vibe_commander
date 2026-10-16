// The test of the web page's server and of the page's file system, run by
// harness/webserver.py: see there for what it holds. Node's own modules
// alone; it prints a line for each check that fails and exits with 1, or
// says how many passed.

import assert from "node:assert/strict";
import { mkdtempSync, mkdirSync, writeFileSync, readFileSync, symlinkSync,
         statSync, existsSync, rmSync, readdirSync } from "node:fs";
import { tmpdir } from "node:os";
import path from "node:path";
import vm from "node:vm";

const web = process.argv[2];
const work = process.argv[3] || tmpdir();
const { makeServer, prepare } = await import(path.join(web, "server.mjs"));
const drives = await import(path.join(web, "drives.js"));
const bundle = await import(path.join(web, "bundle.mjs"));
const { Drives, HOST_CONTENTS, HOST_DELETE, HOST_RENAME, HOST_DIR_MAKE,
        HOST_DIR_REMOVE, HOST_MODE, CHANGE_ERRORS } = drives;

let passed = 0, failed = 0;
async function check(name, body) {
  try {
    await body();
    passed++;
  } catch (caught) {
    failed++;
    console.log(`FAIL ${name}: ${caught.message}`);
  }
}

const top = mkdtempSync(path.join(work, "webserver-"));
const outside = path.join(top, "outside");
mkdirSync(outside);
writeFileSync(path.join(outside, "secret.txt"), "not yours");
const page = path.join(top, "vc.html");
writeFileSync(page, "<!doctype html><title>page</title>");
const options = { c: path.join(top, "c"), d: path.join(top, "d"),
                  demo: path.join(web, "demo"), page };
const roots = await prepare(options);
// A link of the host's inside a drive that leads out of it, and one to a
// directory outside.
symlinkSync(path.join(outside, "secret.txt"), path.join(roots.d, "link.txt"));
symlinkSync(outside, path.join(roots.d, "out"));
const server = makeServer(roots, page);
await new Promise((done) => server.listen(0, "127.0.0.1", done));
const base = `http://127.0.0.1:${server.address().port}`;

async function change(order) {
  const answer = await fetch(`${base}/api/change`, {
    method: "POST", headers: { "Content-Type": "application/json" },
    body: JSON.stringify(order) });
  return { status: answer.status, body: await answer.json() };
}
async function put(drive, file, bytes, query = "") {
  const answer = await fetch(
    `${base}/api/file?drive=${drive}&path=${encodeURIComponent(file)}${query}`,
    { method: "PUT", body: bytes });
  return { status: answer.status, body: await answer.json() };
}
async function tree(drive) {
  const answer = await fetch(`${base}/api/tree?drive=${drive}`);
  return { status: answer.status, body: await answer.json() };
}
const refused = (answer) => {
  assert.notEqual(answer.status, 200, "was done");
  assert.equal(answer.body.error, "EACCES");
};

// 1. The server.
await check("the drives made", () => {
  assert.ok(statSync(path.join(roots.c, "vc")).isDirectory());
  for (const name of readdirSync(options.demo))
    assert.ok(existsSync(path.join(roots.d, name)), name);
});
await check("the page", async () => {
  const answer = await fetch(`${base}/`);
  assert.equal(answer.status, 200);
  assert.match(await answer.text(), /<title>page/);
});
await check("the tree", async () => {
  const { status, body } = await tree("d");
  assert.equal(status, 200);
  const paths = body.entries.map((entry) => entry.path);
  assert.ok(paths.includes("readme.txt"));
  assert.ok(paths.includes("notes"));
  assert.ok(paths.indexOf("notes") < paths.indexOf("notes/todo.txt"));
  assert.ok(!paths.some((p) => p.startsWith("link.txt") || p.startsWith("out")),
            "a link listed");
  const readme = body.entries.find((entry) => entry.path === "readme.txt");
  assert.equal(Buffer.from(readme.data, "base64").toString(),
               readFileSync(path.join(roots.d, "readme.txt"), "utf8"));
});
await check("a file written, its time and read-only", async () => {
  const when = Date.UTC(1994, 4, 3, 12, 0, 0);
  const answer = await put("c", "vc/vc.ini", Buffer.from([1, 2, 3]),
                           `&mtime=${when}&readonly=1`);
  assert.equal(answer.status, 200);
  const at = path.join(roots.c, "vc", "vc.ini");
  assert.deepEqual([...readFileSync(at)], [1, 2, 3]);
  assert.equal(statSync(at).mtimeMs, when);
  assert.equal(statSync(at).mode & 0o222, 0);
  // Written again over its read-only bit, as the program closes a file
  // that it made read-only.
  assert.equal((await put("c", "vc/vc.ini", Buffer.from("x"),
                          "&readonly=0")).status, 200);
  assert.equal(readFileSync(at, "utf8"), "x");
  assert.notEqual(statSync(at).mode & 0o200, 0);
});
await check("a file into a directory that is not there", async () => {
  const answer = await put("c", "nowhere/x.txt", Buffer.from("x"));
  assert.equal(answer.body.error, "ENOENT");
});
await check("a directory made, renamed, removed", async () => {
  assert.equal((await change({ op: "mkdir", drive: "d", path: "new" })).status, 200);
  assert.equal((await change({ op: "mkdir", drive: "d", path: "new" })).body.error,
               "EEXIST");
  assert.equal((await put("d", "new/a.txt", Buffer.from("a"))).status, 200);
  assert.equal((await change({ op: "rmdir", drive: "d", path: "new" })).body.error,
               "ENOTEMPTY");
  assert.equal((await change({ op: "rename", drive: "d", path: "new",
                               to: "old" })).status, 200);
  assert.ok(existsSync(path.join(roots.d, "old", "a.txt")));
  assert.equal((await change({ op: "rename", drive: "d", path: "old",
                               to: "old/inner" })).body.error, "EACCES");
  assert.equal((await change({ op: "rename", drive: "d", path: "readme.txt",
                               to: "old/a.txt" })).body.error, "EEXIST");
  assert.equal((await change({ op: "delete", drive: "d", path: "old" })).body.error,
               "EACCES");
  assert.equal((await change({ op: "delete", drive: "d", path: "old/a.txt" })).status,
               200);
  assert.equal((await change({ op: "rmdir", drive: "d", path: "old" })).status, 200);
  assert.ok(!existsSync(path.join(roots.d, "old")));
  assert.equal((await change({ op: "delete", drive: "d", path: "old" })).body.error,
               "ENOENT");
});
await check("the mode", async () => {
  assert.equal((await change({ op: "mode", drive: "d", path: "readme.txt",
                               readonly: true })).status, 200);
  assert.equal(statSync(path.join(roots.d, "readme.txt")).mode & 0o222, 0);
  await change({ op: "mode", drive: "d", path: "readme.txt", readonly: false });
  assert.notEqual(statSync(path.join(roots.d, "readme.txt")).mode & 0o200, 0);
});
await check("paths outside the drives refused", async () => {
  for (const bad of ["..", "../outside/secret.txt", "notes/../../outside",
                     "/etc/passwd", "notes\\todo.txt", "", ".", "notes//x",
                     "notes/./todo.txt", `${outside}/secret.txt`, "a\0b"]) {
    refused(await put("d", bad, Buffer.from("x")));
    refused(await change({ op: "delete", drive: "d", path: bad }));
    refused(await change({ op: "mkdir", drive: "d", path: bad }));
    refused(await change({ op: "rename", drive: "d", path: "readme.txt",
                          to: bad }));
  }
  refused(await put("e", "x.txt", Buffer.from("x")));
  refused(await change({ op: "mkdir", drive: "../c", path: "x" }));
  refused(await change({ op: "mkdir", drive: "d" }));
  assert.equal((await tree("e")).body.error, "EACCES");
  assert.equal((await change({ op: "chown", drive: "d", path: "readme.txt" }))
               .body.error, "EACCES");
});
await check("links of the host refused", async () => {
  refused(await put("d", "link.txt", Buffer.from("x")));
  refused(await put("d", "out/new.txt", Buffer.from("x")));
  refused(await change({ op: "delete", drive: "d", path: "link.txt" }));
  refused(await change({ op: "delete", drive: "d", path: "out/secret.txt" }));
  refused(await change({ op: "mode", drive: "d", path: "out/secret.txt",
                        readonly: true }));
  refused(await change({ op: "rename", drive: "d", path: "out", to: "in" }));
  refused(await change({ op: "rename", drive: "d", path: "readme.txt",
                        to: "out/readme.txt" }));
  assert.equal(readFileSync(path.join(outside, "secret.txt"), "utf8"),
               "not yours");
  assert.deepEqual(readdirSync(outside), ["secret.txt"]);
});
await check("what is no request", async () => {
  const answer = await fetch(`${base}/api/change`, { method: "POST",
                                                     body: "not json" });
  assert.notEqual(answer.status, 200);
  assert.equal((await fetch(`${base}/etc/passwd`)).status, 404);
});

// 2. The page's file system, drives.js, against the server, with a file
// system of the shape of Emscripten's.
class Files {
  constructor() {
    this.nodes = new Map([["/", { directory: true, mode: 0o40777, mtime: 0 }]]);
    this.streams = new Map();
  }
  mkdirTree(at) {
    let so = "";
    for (const name of at.split("/").filter(Boolean)) {
      so += `/${name}`;
      if (!this.nodes.has(so))
        this.nodes.set(so, { directory: true, mode: 0o40777, mtime: 0 });
    }
  }
  writeFile(at, bytes) {
    const old = this.nodes.get(at);
    this.nodes.set(at, { directory: false, mode: old ? old.mode : 0o100666,
                         mtime: Date.now(), data: Uint8Array.from(bytes) });
  }
  readFile(at) {
    const node = this.nodes.get(at);
    if (!node || node.directory)
      throw new Error("ENOENT");
    return node.data;
  }
  stat(at) {
    const node = this.nodes.get(at);
    if (!node)
      throw new Error("ENOENT");
    return { mode: node.mode, mtime: new Date(node.mtime) };
  }
  utime(at, atime, mtime) { this.nodes.get(at).mtime = mtime; }
  chmod(at, mode) {
    const node = this.nodes.get(at);
    node.mode = (node.mode & ~0o7777) | mode;
  }
  open(at) {
    const fd = this.streams.size + 3;
    this.streams.set(fd, { path: at });
    return fd;
  }
  getStream(fd) { return this.streams.get(fd); }
}
const number = (name) => CHANGE_ERRORS.indexOf(name);

const files = new Files();
const mirror = new Drives({ fs: files, fetch, base });
await check("the drives loaded", async () => {
  await mirror.load();
  assert.equal(new TextDecoder().decode(files.readFile("/drive/d/notes/todo.txt")),
               readFileSync(path.join(roots.d, "notes", "todo.txt"), "utf8"));
  assert.ok(files.stat("/drive/c/vc"));
  assert.equal(files.stat("/drive/d/data/cities.csv").mtime.getTime(),
               Math.floor(statSync(path.join(roots.d, "data", "cities.csv"))
                          .mtimeMs));
  assert.throws(() => files.stat("/drive/d/link.txt"));
});
await check("where a path is", () => {
  assert.deepEqual(mirror.locate("/drive/d/notes/todo.txt"),
                   { drive: "d", path: "notes/todo.txt" });
  assert.equal(mirror.locate("/drive/d"), null);
  assert.equal(mirror.locate("/drive/dd/x"), null);
  assert.equal(mirror.locate("/tmp/x"), null);
});
await check("a file closed reaches the server", async () => {
  const at = "/drive/c/vc/vc.ini";
  files.writeFile(at, new TextEncoder().encode("[setup]"));
  const when = Date.UTC(2026, 9, 15, 10, 0, 0);
  files.utime(at, when, when);
  files.chmod(at, 0o444);
  const fd = files.open(at);
  assert.equal(await mirror.change(HOST_CONTENTS, null, null, fd, 0), 0);
  const host = path.join(roots.c, "vc", "vc.ini");
  assert.equal(readFileSync(host, "utf8"), "[setup]");
  assert.equal(statSync(host).mtimeMs, when);
  assert.equal(statSync(host).mode & 0o222, 0);
  // A file of no drive is no concern of the server's.
  assert.equal(await mirror.change(HOST_CONTENTS, null, null,
                                   files.open("/tmp/x"), 0), 0);
});
await check("the changes reach the server", async () => {
  assert.equal(await mirror.change(HOST_DIR_MAKE, "/drive/d/work", null, -1, 0), 0);
  assert.ok(statSync(path.join(roots.d, "work")).isDirectory());
  assert.equal(await mirror.change(HOST_DIR_MAKE, "/drive/d/work", null, -1, 0),
               number("EEXIST"));
  assert.equal(await mirror.change(HOST_RENAME, "/drive/d/work",
                                   "/drive/d/play", -1, 0), 0);
  assert.equal(await mirror.change(HOST_RENAME, "/drive/d/play",
                                   "/drive/c/play", -1, 0), number("EACCES"));
  assert.equal(await mirror.change(HOST_DIR_REMOVE, "/drive/d/play", null, -1, 0), 0);
  assert.ok(!existsSync(path.join(roots.d, "play")));
  assert.equal(await mirror.change(HOST_MODE, "/drive/d/readme.txt", null, -1,
                                   0o444), 0);
  assert.equal(statSync(path.join(roots.d, "readme.txt")).mode & 0o222, 0);
  assert.equal(await mirror.change(HOST_MODE, "/drive/d/readme.txt", null, -1,
                                   0o644), 0);
  assert.equal(await mirror.change(HOST_DELETE, "/drive/d/data/dos.txt", null,
                                   -1, 0), 0);
  assert.ok(!existsSync(path.join(roots.d, "data", "dos.txt")));
  assert.equal(await mirror.change(HOST_DELETE, "/drive/d/data/dos.txt", null,
                                   -1, 0), number("ENOENT"));
  assert.equal(await mirror.change(HOST_DELETE, "/drive/d/link.txt", null,
                                   -1, 0), number("EACCES"));
});
await check("an upload", async () => {
  const bytes = new TextEncoder().encode("brought in");
  assert.ok(!mirror.exists("/drive/d/notes", "new.txt"));
  assert.equal(await mirror.upload("/drive/d/notes", "new.txt", bytes), 0);
  assert.ok(mirror.exists("/drive/d/notes", "new.txt"));
  assert.equal(readFileSync(path.join(roots.d, "notes", "new.txt"), "utf8"),
               "brought in");
  assert.equal(await mirror.upload("/drive/d/notes", "../x.txt", bytes),
               number("EACCES"));
  assert.equal(await mirror.upload("/elsewhere", "x.txt", bytes),
               number("EACCES"));
});
await check("a server that is not there is DOS's failure", async () => {
  const gone = new Drives({ fs: files, fetch, base: "http://127.0.0.1:1" });
  assert.equal(await gone.change(HOST_DELETE, "/drive/d/readme.txt", null, -1, 0),
               number("EIO"));
  assert.ok(existsSync(path.join(roots.d, "readme.txt")));
});

// 3. The page's script as the build makes it: one script, which compiles.
await check("the page's script", () => {
  const script = bundle.script(web);
  new vm.Script(script, { filename: "page" });
  assert.ok(!/^\s*(import|export)\s/m.test(script));
  // Run with a window of nothing but what the page sets before the
  // document is there: it puts Module up, and the modules' answers hold
  // every name that another takes.
  const window = {};
  const context = vm.createContext({
    window, console,
    document: { readyState: "loading", addEventListener: () => {} },
  });
  vm.runInContext(script, context);
  assert.equal(typeof window.Module.vcBegin, "function");
  assert.equal(window.Module.vcInput.length, 0);
});

server.close();
rmSync(top, { recursive: true, force: true });
if (failed > 0) {
  console.log(`${failed} of ${passed + failed} checks failed`);
  process.exit(1);
}
console.log(`${passed} checks passed`);
