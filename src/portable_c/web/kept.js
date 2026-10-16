// The page's drives with no server, the piece W3c of step 7 (CLAUDE.md,
// "The web page"; TODO.md, "Step 7, the web page"): the page served
// statically, as from GitHub Pages, keeps the user's changes in the
// browser. The two drives start from what is served with the page, each
// a tree of the shape of the server's answer, drives/c.json and
// drives/d.json beside the page (static.mjs makes them), and every change
// that the program makes is kept in a store of the browser's own,
// IndexedDB, before it is made in the page's file system, as the page
// served by the server has it made on the server first (drives.js). At
// the next visit the drives are loaded from the store, not from what is
// served, until the user resets them.
//
// KeptDrives is drives.js's Drives with the store where the server was:
// the same `load`, `change` and `upload`, its `put` and `order` made in
// the store. What the store cannot keep is DOS's error for that call, as
// what the server refuses is. A change that the file system is to refuse
// is refused here first, by the copy, so that the store never has what
// the copy has not.
//
// The store is IndexedDB where the browser has it (IdbStore), and the
// memory of the page where it has not or refuses it, as in a sandbox or a
// private window (MemoryStore): then nothing outlives the page, and the
// page says so. Both answer the same five methods; the tests run IdbStore
// on a stand-in of IndexedDB, harness/fakeidb.mjs. `all` answers the
// record BEGUN too, which has no drive.

import { Drives, ROOTS, CHANGE_ERRORS, bytesOfBase64 } from "./drives.js";

const FAILED = CHANGE_ERRORS.indexOf("EIO");
const FULL = CHANGE_ERRORS.indexOf("ENOSPC");
const errorOf = (name) => CHANGE_ERRORS.indexOf(name);

// The key of the record that says that the drives have been loaded from
// what is served and are the store's from then on. A path's key has a
// drive's letter and a colon in front of it, which this has not.
const BEGUN = "begun";
const keyOf = (drive, path) => `${drive}:${path}`;

// The database: one store of records by key, each a file or a directory
// of a drive, { drive, path, directory, data, mtime, readonly }.
export const DATABASE = "vibe-commander";
const RECORDS = "records";

// A store in the memory of the page.
export class MemoryStore {
  constructor() {
    this.records = new Map();
    this.kept = false;            // nothing outlives the page
  }
  async get(key) { return this.records.get(key); }
  async put(key, value) { this.records.set(key, value); }
  async delete(key) { this.records.delete(key); }
  async all() { return Array.from(this.records.values()); }
  async clear() { this.records.clear(); }
}

// A request of IndexedDB made a promise.
function asked(request) {
  return new Promise((done, failed) => {
    request.onsuccess = () => done(request.result);
    request.onerror = () => failed(request.error);
  });
}

// A store in IndexedDB, one transaction for each call.
export class IdbStore {
  constructor(database) {
    this.database = database;
    this.kept = true;
  }
  records(mode) {
    return this.database.transaction(RECORDS, mode).objectStore(RECORDS);
  }
  // A write, answered when its transaction has been committed.
  written(make) {
    return new Promise((done, failed) => {
      const transaction = this.database.transaction(RECORDS, "readwrite");
      make(transaction.objectStore(RECORDS));
      transaction.oncomplete = () => done();
      transaction.onerror = () => failed(transaction.error);
      transaction.onabort = () => failed(transaction.error);
    });
  }
  async get(key) { return asked(this.records("readonly").get(key)); }
  async put(key, value) { return this.written((s) => s.put(value, key)); }
  async delete(key) { return this.written((s) => s.delete(key)); }
  async all() { return asked(this.records("readonly").getAll()); }
  async clear() { return this.written((s) => s.clear()); }
}

// The store that the browser gives the page: IdbStore, or MemoryStore
// where there is no IndexedDB or it does not open in `seconds`.
export async function openStore(indexedDB, seconds = 5) {
  if (!indexedDB || typeof indexedDB.open !== "function")
    return new MemoryStore();
  try {
    const database = await new Promise((done, failed) => {
      const timer = setTimeout(() => failed(new Error("no answer")),
                               seconds * 1000);
      const request = indexedDB.open(DATABASE, 1);
      request.onupgradeneeded = () => {
        if (!request.result.objectStoreNames.contains(RECORDS))
          request.result.createObjectStore(RECORDS);
      };
      request.onsuccess = () => { clearTimeout(timer); done(request.result); };
      request.onerror = () => { clearTimeout(timer); failed(request.error); };
      request.onblocked = () => {
        clearTimeout(timer);
        failed(new Error("blocked"));
      };
    });
    return new IdbStore(database);
  } catch {
    return new MemoryStore();
  }
}

// Parents in front of what is in them, and in the order of the names.
function inOrder(records) {
  const depth = (record) => record.path.split("/").length;
  return records.slice().sort((a, b) => depth(a) - depth(b)
    || (a.path < b.path ? -1 : a.path > b.path ? 1 : 0));
}

export class KeptDrives extends Drives {
  // `fs` and `fetch` as Drives has them, `base` where the trees are served,
  // "drives/" beside the page, `store` one of the stores above.
  constructor({ fs, fetch, base = "drives/", store, roots = ROOTS }) {
    super({ fs, fetch, base, roots });
    this.store = store;
  }

  get kept() {
    return this.store.kept;
  }

  // The drives as the store has them, or, at the first visit and after a
  // reset, as they are served, which the store then keeps.
  async load() {
    let records;
    if (await this.store.get(BEGUN)) {
      records = (await this.store.all()).filter(
        (record) => record && typeof record.drive === "string");
    } else {
      records = [];
      for (const drive of Object.keys(this.roots)) {
        const answer = await this.fetch(`${this.base}${drive}.json`,
                                        { cache: "no-cache" });
        if (!answer.ok)
          throw new Error(`drive ${drive} was not served: ${answer.status}`);
        for (const entry of (await answer.json()).entries) {
          const record = { drive, path: entry.path,
                           directory: !!entry.directory,
                           mtime: entry.mtime, readonly: !!entry.readonly };
          if (!record.directory)
            record.data = bytesOfBase64(entry.data);
          records.push(record);
        }
      }
      for (const record of records)
        await this.store.put(keyOf(record.drive, record.path), record);
      await this.store.put(BEGUN, { mtime: Date.now() });
    }
    for (const root of Object.values(this.roots))
      this.fs.mkdirTree(root);
    for (const record of inOrder(records)) {
      const root = this.roots[record.drive];
      if (root === undefined)
        continue;
      const at = `${root}/${record.path}`;
      if (record.directory) {
        this.fs.mkdirTree(at);
      } else {
        this.fs.writeFile(at, record.data);
        if (record.readonly)
          this.fs.chmod(at, 0o444);
      }
      this.fs.utime(at, record.mtime, record.mtime);
    }
  }

  // Everything that the store has, made what is served again at the next
  // load.
  async reset() {
    await this.store.clear();
  }

  // What the copy has at a drive's path: null, "file" or "directory".
  kind(place) {
    try {
      const status = this.fs.stat(`${this.roots[place.drive]}/${place.path}`);
      return this.fs.isDir(status.mode) ? "directory" : "file";
    } catch {
      return null;
    }
  }

  // A write of the store, 0 or the number of its error.
  async keep(make) {
    try {
      await make();
      return 0;
    } catch (caught) {
      return caught && caught.name === "QuotaExceededError" ? FULL : FAILED;
    }
  }

  // A file written whole, as the server's PUT.
  async put(place, bytes, mtime, readonly) {
    if (this.kind(place) === "directory")
      return errorOf("EACCES");
    return this.keep(() => this.store.put(keyOf(place.drive, place.path), {
      drive: place.drive, path: place.path, directory: false,
      data: new Uint8Array(bytes), mtime: Math.round(mtime), readonly }));
  }

  // A change, as the server's POST /api/change, refused where the server
  // would refuse it.
  async order(body) {
    const place = { drive: body.drive, path: body.path };
    const key = keyOf(body.drive, body.path);
    const kind = this.kind(place);
    switch (body.op) {
      case "delete":
        if (kind === null)
          return errorOf("ENOENT");
        if (kind !== "file")
          return errorOf("EACCES");
        return this.keep(() => this.store.delete(key));
      case "rename": {
        if (kind === null)
          return errorOf("ENOENT");
        const to = { drive: body.drive, path: body.to };
        if (this.kind(to) !== null)
          return errorOf("EEXIST");
        if (body.to === body.path || body.to.startsWith(`${body.path}/`))
          return errorOf("EACCES");    // a directory into itself
        return this.keep(async () => {
          const under = `${body.path}/`;
          for (const record of await this.store.all()) {
            if (record.drive !== body.drive
                || (record.path !== body.path
                    && !record.path.startsWith(under)))
              continue;
            const path = body.to + record.path.slice(body.path.length);
            await this.store.put(keyOf(body.drive, path),
                                 { ...record, path });
            await this.store.delete(keyOf(body.drive, record.path));
          }
        });
      }
      case "mkdir":
        if (kind !== null)
          return errorOf("EEXIST");
        return this.keep(() => this.store.put(key, {
          drive: body.drive, path: body.path, directory: true,
          mtime: Date.now(), readonly: false }));
      case "rmdir": {
        if (kind !== "directory")
          return errorOf("ENOENT");
        const at = `${this.roots[body.drive]}/${body.path}`;
        if (this.fs.readdir(at).some((name) => name !== "."
                                               && name !== ".."))
          return errorOf("ENOTEMPTY");
        return this.keep(() => this.store.delete(key));
      }
      case "mode": {
        if (kind !== "file")
          return kind === null ? errorOf("ENOENT") : 0;
        return this.keep(async () => {
          const record = await this.store.get(key);
          if (record)
            await this.store.put(key, { ...record,
                                        readonly: !!body.readonly });
        });
      }
      default:
        return errorOf("EACCES");
    }
  }
}
