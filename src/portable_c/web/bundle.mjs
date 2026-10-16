// The shell of the web page made whole for Emscripten, the piece W3b of
// step 7 (TODO.md, "Step 7, the web page"): shell.html with the font put
// in as base64, the licences of what the page carries put in as data,
// and the page's modules put in as one classic script, which
// emcc then takes with --shell-file and fills with the program, one file
// by SINGLE_FILE. Node's own modules alone.
//
//   node bundle.mjs FONT.woff OUT.html
//
// The licences are the repository's own files, read as the page is made,
// so that what the page shows cannot drift from them: LICENCES below.
//
// The modules are plain ES modules that the tests import as they are;
// here each becomes a function of its own that answers what it exports,
// in the order of MODULES, which is the order in which they name one
// another, and its `import` lines take what it names from the answers of
// those before it. That is all of ES modules that the five use: one-line
// imports of names, and `export` in front of a declaration.

import { readFileSync, writeFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const here = path.dirname(fileURLToPath(import.meta.url));
export const MODULES = ["render.js", "keys.js", "mouse.js", "drives.js",
                        "page.js"];

// What the page carries of every work, and the file of the repository
// that has its licence: the page's "Licences" shows these and nothing
// else, each in the same form, the work, its copyright as the work has
// it, where it is from, and the text (CLAUDE.md, "The web page"). The
// project's own first, then Volkov Commander's, whose code it is made
// of, and the three that the build puts in: Emscripten's runtime in the
// script, musl's code in the wasm, the font. DOS/32A is in no page.
export const LICENCES = [
  { work: "Vibe Commander 0.1", owner: "(c) 2026 Maksym Motorny",
    home: null, file: "LICENSE" },
  { work: "Volkov Commander 4.05, which Vibe Commander is based on",
    owner: "(c) 1991-2000 Vsevolod V. Volkov", home: null,
    file: "src/asm/LICENSE.TXT" },
  { work: "Emscripten 6.0.12, its runtime in the page's script",
    owner: "(c) 2010-2014 Emscripten authors",
    home: "https://emscripten.org/", file: "LICENSE.EMSCRIPTEN" },
  { work: "musl, the C library in the page's program",
    owner: "(c) 2005-2020 Rich Felker, et al.",
    home: "https://musl.libc.org/", file: "LICENSE.MUSL" },
  { work: "Web437 IBM VGA 8x16, of the Ultimate Oldschool PC Font Pack 2.2",
    owner: "(c) 2016-2020 VileR",
    home: "https://int10h.org/oldschool-pc-fonts/", file: "LICENSE.FONT" },
];

// The licences as the page has them: each with its text, read from the
// top of the repository, its line ends made LF.
export function licences(top = path.resolve(here, "../../..")) {
  return LICENCES.map((one) => ({
    work: one.work, owner: one.owner, home: one.home,
    text: readFileSync(path.join(top, one.file), "utf8")
      .replace(/\r\n/g, "\n"),
  }));
}

const NAME = "[A-Za-z_$][\\w$]*";

// The names that a module exports, by its declarations.
function exported(text) {
  const names = [];
  const single = new RegExp(`^export\\s+(?:async\\s+)?(?:function\\*?|class)\\s+(${NAME})`, "gm");
  for (const match of text.matchAll(single))
    names.push(match[1]);
  const declared = /^export\s+(?:const|let|var)\s+([^;]*);/gms;
  for (const match of text.matchAll(declared)) {
    // Each name at the top of the list, in front of its `=`: what the
    // values hold is in brackets or quotes, and has no name so placed.
    let depth = 0, part = "";
    const parts = [];
    for (const character of match[1]) {
      if ("([{".includes(character)) depth++;
      if (")]}".includes(character)) depth--;
      if (character === "," && depth === 0) {
        parts.push(part);
        part = "";
      } else {
        part += character;
      }
    }
    parts.push(part);
    for (const one of parts) {
      const name = one.match(new RegExp(`^\\s*(${NAME})\\s*=`));
      if (name)
        names.push(name[1]);
    }
  }
  return names;
}

const variableOf = (name) => `module_${name.replace(/\W/g, "_")}`;

// The modules as one classic script.
export function script(directory = here) {
  const exports = {};
  const parts = MODULES.map((name) => {
    const text = readFileSync(path.join(directory, name), "utf8");
    exports[name] = exported(text);
    const lines = text.split("\n").map((line) => {
      const taken = line.match(/^import\s*\{([^}]*)\}\s*from\s*"\.\/([^"]+)";\s*$/);
      if (taken) {
        for (const one of taken[1].split(",").map((n) => n.trim())) {
          if (!(exports[taken[2]] || []).includes(one))
            throw new Error(`${name} takes ${one}, which ${taken[2]} ` +
                            "does not export ahead of it");
        }
        return `const {${taken[1]}} = ${variableOf(taken[2])};`;
      }
      if (/^import\s/.test(line))
        throw new Error(`${name}: an import that the bundle does not take`);
      return line.replace(/^export\s+/, "");
    });
    return `// ${name}\nconst ${variableOf(name)} = (function () {\n` +
      `${lines.join("\n")}\nreturn { ${exports[name].join(", ")} };\n})();`;
  });
  return `(function () {\n"use strict";\n${parts.join("\n")}\n})();\n`;
}

export function shell(font, directory = here) {
  const template = readFileSync(path.join(directory, "shell.html"), "utf8");
  const page = script(directory);
  if (page.includes("{{{") || page.includes("</script"))
    throw new Error("the page's script has what the shell cannot hold");
  // JSON in a script element of its own, whose `<` is escaped, so that
  // no text can end the element.
  const data = JSON.stringify(licences()).replace(/</g, "\\u003c");
  if (data.includes("{{{"))
    throw new Error("a licence has what the shell cannot hold");
  return template.replace("@@FONT@@", () => font.toString("base64"))
    .replace("@@LICENCES@@", () => data)
    .replace("@@PAGE@@", () => page);
}

if (process.argv[1] && path.resolve(process.argv[1])
    === fileURLToPath(import.meta.url)) {
  const [font, out] = process.argv.slice(2);
  writeFileSync(out, shell(readFileSync(font)));
}
