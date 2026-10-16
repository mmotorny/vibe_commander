// A server of the static site on this machine, for the tests and for
// trying the page out (docs/web.md): the files of one directory, as
// static.mjs makes it, served as any host of files serves them, as GitHub
// Pages does, and nothing else. It writes nothing: the page keeps its
// drive in its own memory (drives.js). Node's own modules alone.
//
//   node src/portable_c/web/serve.mjs [--port N] [DIR]
//
// DIR is build/portable_c/web/static where nothing is said, and the port
// 8437. It listens on 127.0.0.1 alone. It answers GET and HEAD of a file
// of DIR, index.html for a directory, and 404 for anything else: a name
// that is not there, `..`, a link of the host's, any other method.

import { createServer } from "node:http";
import { promises as fs } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const TYPES = {
  ".html": "text/html; charset=utf-8", ".json": "application/json",
  ".js": "text/javascript", ".css": "text/css", ".wasm": "application/wasm",
  ".png": "image/png", ".svg": "image/svg+xml", ".txt": "text/plain",
};

// The file of `root` that a request's path names, or null: names divided
// by `/`, none `..` or with a backslash or a NUL, no link on the way.
async function fileOf(root, pathname) {
  let names;
  try {
    names = decodeURIComponent(pathname).split("/").filter((n) => n !== "");
  } catch {
    return null;
  }
  if (names.some((n) => n === ".." || n === "." || /[\\\0]/.test(n)))
    return null;
  let at = root;
  for (const name of names) {
    at = path.join(at, name);
    try {
      if ((await fs.lstat(at)).isSymbolicLink())
        return null;
    } catch {
      return null;
    }
  }
  try {
    if ((await fs.stat(at)).isDirectory())
      at = path.join(at, "index.html");
    return (await fs.lstat(at)).isFile() ? at : null;
  } catch {
    return null;
  }
}

// The server of the directory `root`, not yet listening.
export function serve(root) {
  const top = path.resolve(root);
  return createServer((request, response) => {
    const answer = async () => {
      const file = request.method === "GET" || request.method === "HEAD"
        ? await fileOf(top, new URL(request.url, "http://localhost").pathname)
        : null;
      if (file === null) {
        response.writeHead(404, { "Content-Type": "text/html" });
        response.end("<h1>404</h1>");
        return;
      }
      const body = await fs.readFile(file);
      response.writeHead(200, {
        "Content-Type": TYPES[path.extname(file)] || "application/octet-stream",
        "Content-Length": body.length,
        "Cache-Control": "no-cache",
        "X-Content-Type-Options": "nosniff",
      });
      response.end(request.method === "HEAD" ? undefined : body);
    };
    answer().catch(() => {
      if (!response.headersSent)
        response.writeHead(500);
      response.end();
    });
  });
}

if (process.argv[1] && path.resolve(process.argv[1])
    === fileURLToPath(import.meta.url)) {
  const here = path.dirname(fileURLToPath(import.meta.url));
  let port = 8437;
  let root = path.join(here, "..", "..", "..", "build", "portable_c", "web",
                       "static");
  const argv = process.argv.slice(2);
  for (let i = 0; i < argv.length; i++) {
    if (argv[i] === "--port" && i + 1 < argv.length)
      port = Number(argv[++i]);
    else if (!argv[i].startsWith("--"))
      root = argv[i];
    else {
      console.error(`serve.mjs: what is ${argv[i]}? See the top of the file.`);
      process.exit(2);
    }
  }
  const server = serve(root);
  server.listen(port, "127.0.0.1", () => {
    console.log(`Vibe Commander's page on http://127.0.0.1:` +
                `${server.address().port}/, from ${path.resolve(root)}`);
  });
}
