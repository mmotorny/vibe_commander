// A stand-in for the browser's IndexedDB, of as much of it as the web
// page's store uses (src/portable_c/web/kept.js, IdbStore and openStore),
// for the tests that run the page in Node: harness/webserver.mjs and
// harness/webpage.mjs. No browser and nothing downloaded. Its databases
// are kept in the object, so that a second page given the same object is
// a page loaded again in the same browser.
//
// What it does as IndexedDB does: requests answer later, by onsuccess or
// onerror; a database that is opened for the first time has
// onupgradeneeded first; a transaction answers oncomplete when its last
// request has; values are kept as structuredClone makes them. `refuse`
// makes every open fail, as a browser that gives a page no storage, and
// `full` every write, as QuotaExceededError.

const later = (work) => setTimeout(work, 0);

class Request {
  constructor() {
    this.onsuccess = null;
    this.onerror = null;
    this.result = undefined;
    this.error = null;
  }
  answer(result, error = null) {
    later(() => {
      this.result = result;
      this.error = error;
      if (error !== null) {
        if (this.onerror)
          this.onerror({ target: this });
      } else if (this.onsuccess) {
        this.onsuccess({ target: this });
      }
    });
  }
}

class Transaction {
  constructor(idb, records, mode) {
    this.idb = idb;
    this.records = records;
    this.mode = mode;
    this.pending = 0;
    this.failed = null;
    this.oncomplete = null;
    this.onerror = null;
    this.onabort = null;
    this.ended = false;
    later(() => this.end());       // a transaction with nothing asked
  }
  end() {
    if (this.pending > 0 || this.ended)
      return;
    this.ended = true;
    later(() => {
      if (this.failed !== null) {
        this.error = this.failed;
        if (this.onerror)
          this.onerror({ target: this });
        if (this.onabort)
          this.onabort({ target: this });
      } else if (this.oncomplete) {
        this.oncomplete({ target: this });
      }
    });
  }
  objectStore() {
    const ask = (work, writes) => {
      const request = new Request();
      this.pending++;
      later(() => {
        let result, error = null;
        if (writes && this.mode !== "readwrite") {
          error = Object.assign(new Error("read-only"), { name: "ReadOnlyError" });
        } else if (writes && this.idb.full) {
          error = Object.assign(new Error("full"),
                                { name: "QuotaExceededError" });
        } else {
          result = work();
        }
        if (error !== null)
          this.failed = error;
        request.answer(result, error);
        this.pending--;
        later(() => this.end());
      });
      return request;
    };
    const records = this.records;
    return {
      get: (key) => ask(() => structuredClone(records.get(key)), false),
      getAll: () => ask(() => Array.from(records.values(),
                                         (value) => structuredClone(value)),
                        false),
      put: (value, key) => ask(() => {
        records.set(key, structuredClone(value));
        return key;
      }, true),
      delete: (key) => ask(() => { records.delete(key); }, true),
      clear: () => ask(() => { records.clear(); }, true),
    };
  }
}

class Database {
  constructor(idb, stores) {
    this.idb = idb;
    this.stores = stores;
    this.objectStoreNames = { contains: (name) => stores.has(name) };
  }
  createObjectStore(name) {
    this.stores.set(name, new Map());
  }
  transaction(name, mode = "readonly") {
    if (!this.stores.has(name))
      throw Object.assign(new Error(name), { name: "NotFoundError" });
    return new Transaction(this.idb, this.stores.get(name), mode);
  }
}

export class FakeIndexedDB {
  constructor() {
    this.databases = new Map();
    this.opened = 0;
    this.refuse = false;
    this.full = false;
  }
  open(name) {
    this.opened++;
    const request = new Request();
    request.onupgradeneeded = null;
    request.onblocked = null;
    later(() => {
      if (this.refuse) {
        request.answer(undefined, Object.assign(new Error("refused"),
                                                { name: "SecurityError" }));
        return;
      }
      const fresh = !this.databases.has(name);
      if (fresh)
        this.databases.set(name, new Map());
      const database = new Database(this, this.databases.get(name));
      request.result = database;
      if (fresh && request.onupgradeneeded)
        request.onupgradeneeded({ target: request });
      request.answer(database);
    });
    return request;
  }
  // The records of a store of a database, for a test to look at.
  records(name, store) {
    const database = this.databases.get(name);
    return database && database.get(store) || new Map();
  }
}
