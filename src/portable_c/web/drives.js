// The page's file system against its server, the piece W3b of step 7
// (CLAUDE.md, "The web page"; TODO.md, "Step 7, the web page"). The
// page's one drive, C:, is a directory of the server's (server.mjs); the
// page has a copy of it in Emscripten's file system, MEMFS, which the
// program reads and writes as the program for Linux does the host's files
// (files_posix.c). At the start its tree is loaded from the server;
// then every change that the program makes is asked of the server first,
// as files_posix.c tells it (enum HostChange of emulate_posix.h, by the
// page's library, web_change), and made in the copy only where the server
// has made it: what the server refuses or cannot do is DOS's error for
// that call, CHANGE_ERRORS by number, the table of sys_web.c.
//
// A file's contents go whole when the program closes it, before the close
// answers, with its time and whether it is read-only.
//
// A plain module of ECMAScript that is handed the file system, of the
// shape of Emscripten's FS, and `fetch`, so that the test runs it in Node
// against the server with a file system of its own.

// What the program changes (enum HostChange of emulate_posix.h).
export const HOST_CONTENTS = 0, HOST_DELETE = 1, HOST_RENAME = 2,
             HOST_DIR_MAKE = 3, HOST_DIR_REMOVE = 4, HOST_MODE = 5;

// The answers, by number, as sys_web.c's table has them: 0 for done.
export const CHANGE_ERRORS = ["", "EACCES", "ENOENT", "EEXIST", "ENOTEMPTY",
                              "ENOSPC", "EIO"];
const FAILED = CHANGE_ERRORS.indexOf("EIO");

// Where the drives are in the page's file system, by letter: C:, the one
// drive of the page, the program's own files in its directory vc. A test
// may hand Drives others.
export const ROOTS = { c: "/drive/c" };

export function bytesOfBase64(text) {
  if (typeof Uint8Array.fromBase64 === "function")
    return Uint8Array.fromBase64(text);
  const binary = atob(text);
  const bytes = new Uint8Array(binary.length);
  for (let i = 0; i < binary.length; i++)
    bytes[i] = binary.charCodeAt(i);
  return bytes;
}

// The number of an error that the server answered, by its name.
function numberOf(name) {
  const number = CHANGE_ERRORS.indexOf(name);
  return number > 0 ? number : FAILED;
}

export class Drives {
  // `fs` of the shape of Emscripten's FS (mkdirTree, writeFile, readFile,
  // stat, utime, chmod, getStream), `fetch` of the browser's shape, `base`
  // the server's URL that the API's paths are put behind.
  constructor({ fs, fetch, base = "", roots = ROOTS }) {
    this.fs = fs;
    this.fetch = fetch;
    this.base = base;
    this.roots = roots;
  }

  // Each drive's tree, loaded from the server into its directory.
  async load() {
    for (const [drive, root] of Object.entries(this.roots)) {
      const answer = await this.fetch(
        `${this.base}/api/tree?drive=${encodeURIComponent(drive)}`,
        { cache: "no-store" });
      if (!answer.ok)
        throw new Error(`the server did not give drive ${drive}: ` +
                        `${answer.status}`);
      const { entries, mtime } = await answer.json();
      this.fs.mkdirTree(root);
      for (const entry of entries) {
        const at = `${root}/${entry.path}`;
        if (entry.directory) {
          this.fs.mkdirTree(at);
        } else {
          this.fs.writeFile(at, bytesOfBase64(entry.data));
          if (entry.readonly)
            this.fs.chmod(at, 0o444);
        }
        this.fs.utime(at, entry.mtime, entry.mtime);
      }
      // A directory's time once what is in it has been made, which moved
      // it; and the root's, which the tree has apart.
      for (const entry of entries)
        if (entry.directory)
          this.fs.utime(`${root}/${entry.path}`, entry.mtime, entry.mtime);
      if (typeof mtime === "number")
        this.fs.utime(root, mtime, mtime);
    }
  }

  // The drive and the path in it of a path of the file system, or null
  // where it is in no drive or is a drive's root.
  locate(host) {
    if (typeof host !== "string")
      return null;
    for (const [drive, root] of Object.entries(this.roots)) {
      if (host.startsWith(`${root}/`) && host.length > root.length + 1)
        return { drive, path: host.slice(root.length + 1) };
    }
    return null;
  }

  // A request to the server: 0, or the number of its error, that of a
  // failure on the way where it answered nothing.
  async request(url, init) {
    let answer;
    try {
      answer = await this.fetch(`${this.base}${url}`, init);
    } catch {
      return FAILED;
    }
    if (answer.ok)
      return 0;
    try {
      return numberOf((await answer.json()).error);
    } catch {
      return FAILED;
    }
  }

  async order(body) {
    return this.request("/api/change", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body),
    });
  }

  async put(place, bytes, mtime, readonly) {
    const query = new URLSearchParams({
      drive: place.drive, path: place.path, mtime: String(Math.round(mtime)),
      readonly: readonly ? "1" : "0" });
    return this.request(`/api/file?${query}`, {
      method: "PUT",
      headers: { "Content-Type": "application/octet-stream" },
      body: bytes,
    });
  }

  // A change that the program is about to make, made on the server: the
  // answer for files_posix.c, 0 or the number of an error. A path that is
  // in no drive is no concern of the server's.
  async change(what, host, other, fd, mode) {
    if (what === HOST_CONTENTS) {
      const stream = this.fs.getStream(fd);
      const place = stream ? this.locate(stream.path) : null;
      if (place === null)
        return 0;
      const status = this.fs.stat(stream.path);
      return this.put(place, this.fs.readFile(stream.path),
                      Number(status.mtime), (status.mode & 0o200) === 0);
    }
    const place = this.locate(host);
    if (place === null)
      return 0;
    switch (what) {
      case HOST_DELETE:
        return this.order({ op: "delete", ...place });
      case HOST_RENAME: {
        const to = this.locate(other);
        if (to === null || to.drive !== place.drive)
          return numberOf("EACCES");
        return this.order({ op: "rename", ...place, to: to.path });
      }
      case HOST_DIR_MAKE:
        return this.order({ op: "mkdir", ...place });
      case HOST_DIR_REMOVE:
        return this.order({ op: "rmdir", ...place });
      case HOST_MODE:
        return this.order({ op: "mode", ...place,
                            readonly: (mode & 0o200) === 0 });
      default:
        return numberOf("EACCES");
    }
  }

  // A file that the user brings in, `name` in the directory `directory` of
  // the file system, written on the server and then in the copy: 0, or the
  // number of an error. Whether it is there already is `exists`.
  exists(directory, name) {
    try {
      this.fs.stat(`${directory}/${name}`);
      return true;
    } catch {
      return false;
    }
  }

  async upload(directory, name, bytes) {
    if (name === "" || /[/\\\0]/.test(name) || name === "." || name === "..")
      return numberOf("EACCES");
    const host = `${directory}/${name}`;
    const place = this.locate(host);
    if (place === null)
      return numberOf("EACCES");
    const now = Date.now();
    const answer = await this.put(place, bytes, now, false);
    if (answer !== 0)
      return answer;
    try {
      this.fs.writeFile(host, bytes);
      this.fs.utime(host, now, now);
    } catch {
      return FAILED;
    }
    return 0;
  }
}
