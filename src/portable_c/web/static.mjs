// The web page made a static site, the piece W3c of step 7 (CLAUDE.md,
// "The web page"; TODO.md, "Step 7, the web page"): what the page is
// served as, from GitHub Pages or any host of files, a directory of
//
//   index.html       the page, vc.html;
//   drives/c.json    C:, the page's one drive, as every load of the page
//                    starts it: its directory vc and nothing in it, or the
//                    tree of the directory that --drive-c names, for a
//                    test;
//   .nojekyll        so that GitHub Pages serves the files as they are.
//
// The page loads the tree into its memory and keeps every change there
// until it is closed or loaded again (drives.js). Node's own modules
// alone; serve.mjs serves such a directory on this machine.
//
//   node static.mjs --page vc.html --out DIR [--drive-c DIR]

import { promises as fs } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

// A directory's files and directories, parents first, as drives.js loads
// them: { entries: [{ path, directory, mtime, readonly, data }], mtime },
// `data` the contents in base64 and `mtime` the root's. Links of the
// host's, and names with a backslash or a NUL, are left out.
export async function treeOf(root) {
  const entries = [];
  async function walk(directory, prefix) {
    const listed = await fs.readdir(directory, { withFileTypes: true });
    listed.sort((a, b) => (a.name < b.name ? -1 : a.name > b.name ? 1 : 0));
    for (const entry of listed) {
      const at = path.join(directory, entry.name);
      const named = prefix === "" ? entry.name : `${prefix}/${entry.name}`;
      if (entry.isSymbolicLink() || /[\\\0]/.test(entry.name))
        continue;                       // never followed, never shown
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
  await walk(root, "");
  return { entries, mtime: Math.floor((await fs.stat(root)).mtimeMs) };
}

// The site made in `out` of the page `page`, its C: the tree of `drive`
// where that is said.
export async function makeSite({ page, out, drive = null }) {
  let c;
  if (drive === null) {
    const status = await fs.stat(page);
    c = { entries: [{ path: "vc", directory: true,
                      mtime: Math.floor(status.mtimeMs), readonly: false }] };
  } else {
    c = await treeOf(drive);
  }
  await fs.rm(out, { recursive: true, force: true });
  await fs.mkdir(path.join(out, "drives"), { recursive: true });
  await fs.copyFile(page, path.join(out, "index.html"));
  await fs.writeFile(path.join(out, "drives", "c.json"), JSON.stringify(c));
  await fs.writeFile(path.join(out, ".nojekyll"), "");
}

if (process.argv[1] && path.resolve(process.argv[1])
    === fileURLToPath(import.meta.url)) {
  const told = {};
  const argv = process.argv.slice(2);
  for (let i = 0; i < argv.length; i += 2) {
    const name = { "--page": "page", "--out": "out",
                   "--drive-c": "drive" }[argv[i]];
    if (name === undefined || i + 1 >= argv.length) {
      console.error(`static.mjs: what is ${argv[i]}? See the top of the file.`);
      process.exit(2);
    }
    told[name] = path.resolve(argv[i + 1]);
  }
  if (!told.page || !told.out) {
    console.error("static.mjs: --page and --out are wanted.");
    process.exit(2);
  }
  await makeSite(told);
}
