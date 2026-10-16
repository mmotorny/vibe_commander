// The page's drive, the pieces W3b and W3c of step 7 (CLAUDE.md, "The web
// page"; TODO.md, "Step 7, the web page"). The page's one drive, C:, is a
// directory of Emscripten's file system, MEMFS, in the page's memory,
// which the program reads and writes as the program for Linux does the
// host's files (files_posix.c). At the start it is loaded from what is
// served beside the page, drives/c.json, the tree that static.mjs makes;
// from then on it is the page's alone. Nothing of it is written anywhere
// else and nothing is kept in the browser: what the program and the user
// change lasts until the page is closed or loaded again, and every load
// starts from what is served.
//
// A plain module of ECMAScript that is handed the file system, of the
// shape of Emscripten's FS, and `fetch`, so that the test runs it in Node
// with a file system of its own.

// Where the drives are in the page's file system, by letter: C:, the one
// drive of the page, the program's own files in its directory vc. A test
// may hand Drives others.
export const ROOTS = { c: "/drive/c" };

// A:, which is not served: the page makes it, empty, a diskette that the
// user may change disks to and copy onto (page.js).
export const DISKETTE = "/drive/a";

export function bytesOfBase64(text) {
  if (typeof Uint8Array.fromBase64 === "function")
    return Uint8Array.fromBase64(text);
  const binary = atob(text);
  const bytes = new Uint8Array(binary.length);
  for (let i = 0; i < binary.length; i++)
    bytes[i] = binary.charCodeAt(i);
  return bytes;
}

export class Drives {
  // `fs` of the shape of Emscripten's FS (mkdirTree, writeFile, stat,
  // utime, chmod), `fetch` of the browser's shape, `base` where the trees
  // are served, relative to the page: a drive's tree is `${base}c.json`.
  constructor({ fs, fetch, base = "drives/", roots = ROOTS }) {
    this.fs = fs;
    this.fetch = fetch;
    this.base = base;
    this.roots = roots;
  }

  // Each drive's tree, as it is served, made in its directory: its files
  // and directories, parents first, { entries: [{ path, directory, mtime,
  // readonly, data }], mtime }, `data` the contents in base64 and `mtime`
  // that of the drive's root where the tree has it.
  async load() {
    for (const [drive, root] of Object.entries(this.roots)) {
      const answer = await this.fetch(`${this.base}${drive}.json`,
                                      { cache: "no-cache" });
      if (!answer.ok)
        throw new Error(`drive ${drive.toUpperCase()}: was not served: ` +
                        `${answer.status}`);
      const { entries, mtime } = await answer.json();
      if (!Array.isArray(entries))
        throw new Error(`drive ${drive.toUpperCase()}: is no tree`);
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

  // Whether `name` is in the directory `directory` of the file system.
  exists(directory, name) {
    try {
      this.fs.stat(`${directory}/${name}`);
      return true;
    } catch {
      return false;
    }
  }

  // A file that the user brings in, `name` in the directory `directory`
  // of the file system, written there with the time of now: null, or why
  // it was not.
  upload(directory, name, bytes) {
    if (name === "" || /[/\\\0]/.test(name) || name === "." || name === "..")
      return "no name that a file can have";
    const host = `${directory}/${name}`;
    const now = Date.now();
    try {
      this.fs.writeFile(host, bytes);
      this.fs.utime(host, now, now);
    } catch {
      return "the page could not write it";
    }
    return null;
  }
}
