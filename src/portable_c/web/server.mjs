// The web page's server, the piece W3b of step 7 (CLAUDE.md, "The web
// page"; TODO.md, "Step 7, the web page"): a script for Node, of Node's
// own modules and nothing else, that serves the page and the two drives'
// directories, C: where the program lives and D: of the demo, and makes
// on them every change that the page asks for, as it happens. The page
// mirrors the drives in its own file system (drives.js) and asks this
// server before it changes anything there: what the server refuses or
// cannot do is DOS's error for that call.
//
// It listens on 127.0.0.1 alone, on the port that it is told; Tailscale
// Serve hands it to the user's tailnet over HTTPS, and anyone on the
// tailnet may write, the tailnet being the access. It never reaches
// outside the two directories: a path is names of a drive's directory,
// with no `.`, `..`, absolute path or backslash, and none of the names on
// the way may be a link of the host's.
//
//   node src/portable_c/web/server.mjs [--port N] [--page FILE]
//        [--drive-c DIR] [--drive-d DIR] [--demo DIR]
//
// The page is build/portable_c/web/vc.html, and the drives are
// build/portable_c/web/drives/c and d, where nothing else is said: C: is
// made with its directory vc where it is not there, and D: is made as a
// copy of the demo, src/portable_c/web/demo, where it is not there. The
// target `portable_c-web-serve` of the build runs it so.
//
// What it answers:
//   GET  /               the page;
//   GET  /api/tree?drive=c
//                        the drive's files and directories, parents first:
//                        { entries: [{ path, directory, mtime, readonly,
//                        data }] }, `data` the contents in base64;
//   PUT  /api/file?drive=c&path=P&mtime=MS&readonly=0
//                        the file P written whole with the request's body,
//                        made where it is not there, its time and its
//                        read-only bit set;
//   POST /api/change     { op, drive, path, to }: `delete` a file,
//                        `rename` P to `to`, `mkdir`, `rmdir`, or `mode`
//                        with { readonly };
// each change { ok: true }, or an HTTP error with { error }, one of
// ERRORS below, which the page makes DOS's error.

import { createServer as createHttpServer } from "node:http";
import { promises as fs } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

export const ERRORS = ["EACCES", "ENOENT", "EEXIST", "ENOTEMPTY", "ENOSPC",
                       "EIO"];
export const DRIVES = ["c", "d"];
const BODY_MOST = 256 * 1024 * 1024;     // bytes of a file, at the most
const PATH_MOST = 4096;

const here = path.dirname(fileURLToPath(import.meta.url));
const top = path.resolve(here, "..", "..", "..");
export const DEFAULTS = {
  port: 8437,
  page: path.join(top, "build", "portable_c", "web", "vc.html"),
  c: path.join(top, "build", "portable_c", "web", "drives", "c"),
  d: path.join(top, "build", "portable_c", "web", "drives", "d"),
  demo: path.join(here, "demo"),
};

// A refusal with its error and HTTP's status.
class Refused extends Error {
  constructor(error, status = 403) {
    super(error);
    this.error = error;
    this.status = status;
  }
}

// The error that the page is told of what Node's file system said.
function errorOf(caught) {
  if (caught instanceof Refused)
    return caught;
  switch (caught && caught.code) {
    case "EACCES": case "EPERM": case "EROFS":
      return new Refused("EACCES");
    case "ENOENT": case "ENOTDIR":
      return new Refused("ENOENT", 404);
    case "EEXIST":
      return new Refused("EEXIST", 409);
    case "ENOTEMPTY":
      return new Refused("ENOTEMPTY", 409);
    case "ENOSPC": case "EDQUOT": case "EFBIG":
      return new Refused("ENOSPC", 507);
    default:
      return new Refused("EIO", 500);
  }
}

// The names of a path of a drive, or a refusal: names divided by `/`,
// none empty, `.` or `..`, and no backslash or NUL in them. The root
// itself is no path here: nothing asks to change it.
export function namesOf(text) {
  if (typeof text !== "string" || text.length === 0 || text.length > PATH_MOST)
    throw new Refused("EACCES");
  const names = text.split("/");
  for (const name of names) {
    if (name === "" || name === "." || name === ".."
        || /[\\\0]/.test(name))
      throw new Refused("EACCES");
  }
  return names;
}

// Copy a directory into one that is not there yet, for the demo.
async function copyTree(from, to) {
  await fs.mkdir(to, { recursive: true });
  for (const entry of await fs.readdir(from, { withFileTypes: true })) {
    const source = path.join(from, entry.name);
    const target = path.join(to, entry.name);
    if (entry.isDirectory())
      await copyTree(source, target);
    else if (entry.isFile())
      await fs.copyFile(source, target);
  }
}

// The two drives' directories, made where they are not there, with their
// paths from the root and their links followed.
export async function prepare(options) {
  const roots = {};
  const c = options.c, d = options.d;
  await fs.mkdir(path.join(c, "vc"), { recursive: true });
  try {
    await fs.access(d);
  } catch {
    if (options.demo)
      await copyTree(options.demo, d);
    else
      await fs.mkdir(d, { recursive: true });
  }
  roots.c = await fs.realpath(c);
  roots.d = await fs.realpath(d);
  return roots;
}

// The server, not yet listening: `roots` the drives' directories as
// prepare answers them, `page` the file of the page.
export function makeServer(roots, page) {
  // The path of the host of a drive's path, every name on the way that the
  // host has looked at: a link refused wherever it leads, the last name
  // too.
  async function hostOf(drive, text) {
    if (!DRIVES.includes(drive))
      throw new Refused("EACCES");
    const names = namesOf(text);
    let at = roots[drive];
    for (const name of names) {
      at = path.join(at, name);
      let status;
      try {
        status = await fs.lstat(at);
      } catch (caught) {
        if (caught.code === "ENOENT")
          continue;                     // what follows is not there either
        throw caught;
      }
      if (status.isSymbolicLink())
        throw new Refused("EACCES");
    }
    // Made of names that are no `..`, it cannot be outside; said all the
    // same.
    const relative = path.relative(roots[drive], at);
    if (relative.startsWith("..") || path.isAbsolute(relative))
      throw new Refused("EACCES");
    return at;
  }

  async function tree(drive) {
    if (!DRIVES.includes(drive))
      throw new Refused("EACCES");
    const entries = [];
    async function walk(directory, prefix) {
      const listed = await fs.readdir(directory, { withFileTypes: true });
      listed.sort((a, b) => (a.name < b.name ? -1 : a.name > b.name ? 1 : 0));
      for (const entry of listed) {
        const at = path.join(directory, entry.name);
        const named = prefix === "" ? entry.name : `${prefix}/${entry.name}`;
        if (entry.isSymbolicLink() || /[\\\0]/.test(entry.name))
          continue;                     // never followed, never shown
        const status = await fs.lstat(at);
        const readonly = (status.mode & 0o200) === 0;
        if (entry.isDirectory()) {
          entries.push({ path: named, directory: true,
                         mtime: Math.floor(status.mtimeMs), readonly: false });
          await walk(at, named);
        } else if (entry.isFile()) {
          const data = await fs.readFile(at);
          entries.push({ path: named, directory: false,
                         mtime: Math.floor(status.mtimeMs), readonly,
                         data: data.toString("base64") });
        }
      }
    }
    await walk(roots[drive], "");
    return { entries };
  }

  async function write(drive, text, body, mtime, readonly) {
    const at = await hostOf(drive, text);
    const parent = await fs.stat(path.dirname(at));
    if (!parent.isDirectory())
      throw new Refused("ENOENT", 404);
    let status = null;
    try {
      status = await fs.lstat(at);
    } catch (caught) {
      if (caught.code !== "ENOENT")
        throw caught;
    }
    if (status !== null && !status.isFile())
      throw new Refused("EACCES");
    if (status !== null && (status.mode & 0o200) === 0)
      await fs.chmod(at, status.mode | 0o200);
    await fs.writeFile(at, body);
    await fs.chmod(at, readonly ? 0o444 : 0o644);
    if (Number.isFinite(mtime)) {
      const when = new Date(mtime);
      await fs.utimes(at, when, when);
    }
  }

  async function change(order) {
    const drive = order.drive;
    const at = await hostOf(drive, order.path);
    switch (order.op) {
      case "delete": {
        const status = await fs.lstat(at);
        if (!status.isFile())
          throw new Refused("EACCES");
        await fs.unlink(at);
        return;
      }
      case "rename": {
        const to = await hostOf(drive, order.to);
        await fs.lstat(at);
        try {
          await fs.lstat(to);
          throw new Refused("EEXIST", 409);
        } catch (caught) {
          if (caught instanceof Refused || caught.code !== "ENOENT")
            throw caught;
        }
        const resolved = path.resolve(to);
        if (resolved === at || resolved.startsWith(at + path.sep))
          throw new Refused("EACCES");  // a directory into itself
        await fs.rename(at, to);
        return;
      }
      case "mkdir":
        await fs.mkdir(at);
        return;
      case "rmdir": {
        const status = await fs.lstat(at);
        if (!status.isDirectory())
          throw new Refused("ENOENT", 404);
        await fs.rmdir(at);
        return;
      }
      case "mode": {
        const status = await fs.lstat(at);
        if (status.isFile())
          await fs.chmod(at, order.readonly ? 0o444 : 0o644);
        return;
      }
      default:
        throw new Refused("EACCES", 400);
    }
  }

  function send(response, status, value, type = "application/json") {
    const body = type === "application/json"
      ? Buffer.from(JSON.stringify(value)) : value;
    response.writeHead(status, {
      "Content-Type": type,
      "Content-Length": body.length,
      "Cache-Control": "no-store",
      "X-Content-Type-Options": "nosniff",
    });
    response.end(body);
  }

  async function bodyOf(request) {
    const pieces = [];
    let length = 0;
    for await (const piece of request) {
      length += piece.length;
      if (length > BODY_MOST)
        throw new Refused("ENOSPC", 413);
      pieces.push(piece);
    }
    return Buffer.concat(pieces);
  }

  async function handle(request, response) {
    const url = new URL(request.url, "http://localhost");
    const query = url.searchParams;
    if (request.method === "GET"
        && (url.pathname === "/" || url.pathname === "/vc.html")) {
      send(response, 200, await fs.readFile(page), "text/html; charset=utf-8");
      return;
    }
    if (request.method === "GET" && url.pathname === "/api/tree") {
      send(response, 200, await tree(query.get("drive")));
      return;
    }
    if (request.method === "PUT" && url.pathname === "/api/file") {
      const body = await bodyOf(request);
      await write(query.get("drive"), query.get("path"), body,
                  Number(query.get("mtime")), query.get("readonly") === "1");
      send(response, 200, { ok: true });
      return;
    }
    if (request.method === "POST" && url.pathname === "/api/change") {
      let order;
      try {
        order = JSON.parse((await bodyOf(request)).toString("utf8"));
      } catch {
        throw new Refused("EACCES", 400);
      }
      if (order === null || typeof order !== "object")
        throw new Refused("EACCES", 400);
      await change(order);
      send(response, 200, { ok: true });
      return;
    }
    send(response, 404, { error: "ENOENT" });
  }

  return createHttpServer((request, response) => {
    handle(request, response).catch((caught) => {
      const refused = errorOf(caught);
      if (!response.headersSent)
        send(response, refused.status, { error: refused.error });
      else
        response.destroy();
    });
  });
}

// The command line: what it is told, the rest by DEFAULTS.
function told(argv) {
  const options = { ...DEFAULTS };
  const names = { "--port": "port", "--page": "page", "--drive-c": "c",
                  "--drive-d": "d", "--demo": "demo" };
  for (let i = 0; i < argv.length; i += 2) {
    const name = names[argv[i]];
    if (name === undefined || i + 1 >= argv.length) {
      console.error(`server.mjs: what is ${argv[i]}? See the top of the ` +
                    "file for what it takes.");
      process.exit(2);
    }
    options[name] = name === "port" ? Number(argv[i + 1])
                                    : path.resolve(argv[i + 1]);
  }
  return options;
}

if (process.argv[1] && path.resolve(process.argv[1])
    === fileURLToPath(import.meta.url)) {
  const options = told(process.argv.slice(2));
  const roots = await prepare(options);
  const server = makeServer(roots, options.page);
  server.listen(options.port, "127.0.0.1", () => {
    console.log(`Vibe Commander's page on http://127.0.0.1:` +
                `${server.address().port}/, C: ${roots.c}, D: ${roots.d}`);
  });
}
