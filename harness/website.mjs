// The test of the web page's static site, its server on this machine and
// the page's drive, run by harness/website.py: see there for what it
// holds. Node's own modules alone; it prints a line for each check that
// fails and exits with 1, or says how many passed.

import assert from "node:assert/strict";
import { mkdtempSync, mkdirSync, writeFileSync, readFileSync, symlinkSync,
         chmodSync, utimesSync, readdirSync, rmSync } from "node:fs";
import { request } from "node:http";
import { tmpdir } from "node:os";
import path from "node:path";
import vm from "node:vm";

const web = process.argv[2];
const work = process.argv[3] || tmpdir();
const { makeSite, treeOf } = await import(path.join(web, "static.mjs"));
const { serve } = await import(path.join(web, "serve.mjs"));
const { Drives } = await import(path.join(web, "drives.js"));
const { FILES, layFiles } = await import(new URL("./webfiles.mjs", import.meta.url));
const bundle = await import(path.join(web, "bundle.mjs"));

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

const top = mkdtempSync(path.join(work, "website-"));
const outside = path.join(top, "outside");
mkdirSync(outside);
writeFileSync(path.join(outside, "secret.txt"), "not yours");
const page = path.join(top, "vc.html");
writeFileSync(page, "<!doctype html><title>page</title>");
const when = Date.UTC(2026, 9, 16, 9, 30, 0);

// 1. The site.
const shipped = path.join(top, "shipped");
await check("the site as it is shipped: the page and C: with vc alone",
            async () => {
  await makeSite({ page, out: shipped });
  assert.deepEqual(readdirSync(shipped).sort(),
                   [".nojekyll", "drives", "index.html"]);
  assert.equal(readFileSync(path.join(shipped, "index.html"), "utf8"),
               readFileSync(page, "utf8"));
  assert.deepEqual(readdirSync(path.join(shipped, "drives")), ["c.json"]);
  const c = JSON.parse(readFileSync(path.join(shipped, "drives", "c.json")));
  assert.equal(c.entries.length, 1);
  assert.equal(c.entries[0].path, "vc");
  assert.equal(c.entries[0].directory, true);
});
const drive = path.join(top, "c");
mkdirSync(path.join(drive, "vc"), { recursive: true });
layFiles(drive);
chmodSync(path.join(drive, "data", "dos.txt"), 0o444);
utimesSync(path.join(drive, "notes", "todo.txt"), when / 1000, when / 1000);
// Links of the host's inside the drive that lead out of it.
symlinkSync(path.join(outside, "secret.txt"), path.join(drive, "link.txt"));
symlinkSync(outside, path.join(drive, "out"));
utimesSync(drive, when / 1000, when / 1000);
const site = path.join(top, "site");
let tree = null;
await check("a site with C: of a directory: its tree, parents first",
            async () => {
  await makeSite({ page, out: site, drive });
  tree = JSON.parse(readFileSync(path.join(site, "drives", "c.json")));
  assert.deepEqual(tree, await treeOf(drive));
  assert.equal(tree.mtime, when);
  const paths = tree.entries.map((entry) => entry.path);
  for (const name of Object.keys(FILES)) {
    assert.ok(paths.includes(name), `${name} is not in the tree`);
    const parent = name.split("/").slice(0, -1).join("/");
    if (parent !== "")
      assert.ok(paths.indexOf(parent) < paths.indexOf(name),
                `${name} comes before its directory`);
  }
  assert.ok(!paths.some((p) => p.startsWith("link") || p.startsWith("out")),
            "a link is in the tree");
  const todo = tree.entries.find((entry) => entry.path === "notes/todo.txt");
  assert.equal(Buffer.from(todo.data, "base64").toString(),
               FILES["notes/todo.txt"]);
  assert.equal(todo.mtime, when);
  assert.equal(tree.entries.find((e) => e.path === "data/dos.txt").readonly,
               true);
  assert.equal(todo.readonly, false);
});
// A link of the host's in the site itself, which is never served.
symlinkSync(path.join(outside, "secret.txt"), path.join(site, "secret.txt"));
symlinkSync(outside, path.join(site, "out"));

// 2. The server.
const server = serve(site);
await new Promise((done) => server.listen(0, "127.0.0.1", done));
const base = `http://127.0.0.1:${server.address().port}`;
// A request with its path as it is written, which fetch would make
// plain: `..` and the like reach the server as they are.
function raw(method, pathname) {
  return new Promise((done, failed) => {
    const asked = request({ host: "127.0.0.1", port: server.address().port,
                            method, path: pathname }, (answer) => {
      const pieces = [];
      answer.on("data", (piece) => pieces.push(piece));
      answer.on("end", () => done({ status: answer.statusCode,
                                    headers: answer.headers,
                                    body: Buffer.concat(pieces).toString() }));
    });
    asked.on("error", failed);
    asked.end();
  });
}
await check("the page at the root, and the site's files", async () => {
  const root = await raw("GET", "/");
  assert.equal(root.status, 200);
  assert.match(root.headers["content-type"], /^text\/html/);
  assert.equal(root.body, readFileSync(page, "utf8"));
  const index = await raw("GET", "/index.html");
  assert.equal(index.body, root.body);
  const c = await raw("GET", "/drives/c.json");
  assert.equal(c.status, 200);
  assert.equal(c.headers["content-type"], "application/json");
  assert.deepEqual(JSON.parse(c.body), tree);
  const head = await raw("HEAD", "/drives/c.json");
  assert.equal(head.status, 200);
  assert.equal(head.body, "");
});
await check("nothing outside the site, and nothing but GET and HEAD",
            async () => {
  for (const pathname of ["/missing.txt", "/drives/d.json", "/../outside/secret.txt",
                          "/drives/../../outside/secret.txt",
                          "/%2e%2e/outside/secret.txt", "/drives/%2e%2e/%2e%2e/outside",
                          "/drives%5cc.json", "/index.html%00",
                          "/secret.txt", "/out/secret.txt", "/%E0%A4%A",
                          "/etc/passwd"]) {
    const answer = await raw("GET", pathname);
    assert.equal(answer.status, 404, `${pathname} answered ${answer.status}`);
    assert.ok(!answer.body.includes("not yours"), `${pathname} was served`);
  }
  for (const method of ["PUT", "POST", "DELETE"]) {
    const answer = await raw(method, "/drives/c.json");
    assert.equal(answer.status, 404, `${method} answered ${answer.status}`);
  }
  assert.deepEqual(JSON.parse(readFileSync(path.join(site, "drives", "c.json"))),
                   tree);
});

// 3. The page's drive, drives.js, loaded from the site as served, with a
// file system of the shape of Emscripten's.
class Files {
  constructor() {
    this.nodes = new Map([["/", { directory: true, mode: 0o40777, mtime: 0 }]]);
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
    const parent = at.slice(0, at.lastIndexOf("/")) || "/";
    if (!this.nodes.has(parent))
      throw new Error("ENOENT");
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
}
const files = new Files();
const drives = new Drives({ fs: files, base: `${base}/drives/`, fetch });
await check("the drive loaded as it is served", async () => {
  await drives.load();
  assert.equal(new TextDecoder().decode(files.readFile("/drive/c/notes/todo.txt")),
               FILES["notes/todo.txt"]);
  assert.ok(files.stat("/drive/c/vc"));
  assert.equal(files.stat("/drive/c/notes/todo.txt").mtime.getTime(), when);
  assert.equal(files.stat("/drive/c").mtime.getTime(), when);
  assert.equal(files.stat("/drive/c/data/dos.txt").mode & 0o222, 0);
  assert.notEqual(files.stat("/drive/c/notes/todo.txt").mode & 0o200, 0);
  const notes = tree.entries.find((entry) => entry.path === "notes");
  assert.equal(files.stat("/drive/c/notes").mtime.getTime(), notes.mtime);
  assert.throws(() => files.stat("/drive/c/link.txt"));
});
await check("a file brought in, and a name that no file has refused",
            async () => {
  const bytes = new TextEncoder().encode("brought in");
  assert.ok(!drives.exists("/drive/c/notes", "new.txt"));
  assert.equal(drives.upload("/drive/c/notes", "new.txt", bytes), null);
  assert.ok(drives.exists("/drive/c/notes", "new.txt"));
  assert.equal(new TextDecoder().decode(files.readFile("/drive/c/notes/new.txt")),
               "brought in");
  for (const name of ["", ".", "..", "../x.txt", "a\\b", "a\0b"])
    assert.equal(typeof drives.upload("/drive/c/notes", name, bytes), "string",
                 `${JSON.stringify(name)} was taken`);
  assert.equal(typeof drives.upload("/drive/nowhere", "x.txt", bytes), "string");
  // Nothing of it reaches the site.
  assert.deepEqual(readdirSync(path.join(site, "drives")), ["c.json"]);
});
await check("a site that serves no drive, or no tree, is refused", async () => {
  const none = new Drives({ fs: new Files(), base: `${base}/nothing/`, fetch });
  await assert.rejects(none.load());
  const notree = new Drives({
    fs: new Files(), base: "",
    fetch: async () => new Response("{\"no\":1}", { status: 200 }) });
  await assert.rejects(notree.load());
});
server.close();

// 4. The page's script as the build makes it: one script, which compiles.
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
  assert.equal(window.Module.vcChange, undefined);
  assert.equal(window.Module.vcSync, undefined);
});

rmSync(top, { recursive: true, force: true });
if (failed > 0) {
  console.log(`${failed} of ${passed + failed} checks failed`);
  process.exit(1);
}
console.log(`${passed} checks passed`);
