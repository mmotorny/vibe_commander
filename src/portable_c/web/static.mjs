// The web page made a static site, the piece W3c of step 7 (CLAUDE.md,
// "The web page"; TODO.md, "Step 7, the web page"): what a host with no
// server, as GitHub Pages, serves, a directory of
//
//   index.html       the page, vc.html;
//   drives/c.json    C:, its directory vc and nothing in it;
//   drives/d.json    D:, the demo;
//   .nojekyll        so that GitHub Pages serves the files as they are;
//
// the trees of the shape that the server answers for GET /api/tree
// (server.mjs, treeOf). The page finds no server there and keeps what the
// user changes in the browser (page.js, askMode; kept.js). Node's own
// modules alone.
//
//   node static.mjs --page vc.html --demo DIR --out DIR

import { promises as fs } from "node:fs";
import path from "node:path";
import { treeOf } from "./server.mjs";

const told = {};
const argv = process.argv.slice(2);
for (let i = 0; i < argv.length; i += 2) {
  const name = { "--page": "page", "--demo": "demo", "--out": "out" }[argv[i]];
  if (name === undefined || i + 1 >= argv.length) {
    console.error(`static.mjs: what is ${argv[i]}? See the top of the file.`);
    process.exit(2);
  }
  told[name] = path.resolve(argv[i + 1]);
}
if (!told.page || !told.demo || !told.out) {
  console.error("static.mjs: --page, --demo and --out are wanted.");
  process.exit(2);
}

const page = await fs.stat(told.page);
const c = { entries: [{ path: "vc", directory: true,
                        mtime: Math.floor(page.mtimeMs), readonly: false }] };
const d = await treeOf(told.demo);
await fs.rm(told.out, { recursive: true, force: true });
await fs.mkdir(path.join(told.out, "drives"), { recursive: true });
await fs.copyFile(told.page, path.join(told.out, "index.html"));
await fs.writeFile(path.join(told.out, "drives", "c.json"), JSON.stringify(c));
await fs.writeFile(path.join(told.out, "drives", "d.json"), JSON.stringify(d));
await fs.writeFile(path.join(told.out, ".nojekyll"), "");
