// The files that the web page's tests work on, made by each test in the
// page's one drive, C:, beside the program's own directory vc, where the
// page itself starts with nothing but that directory (CLAUDE.md, "The
// web page"): harness/webserver.mjs, harness/webpage.mjs and
// harness/webbrowser.mjs. Nothing of them is shipped with the page.
// Node's own modules alone.

import { mkdirSync, writeFileSync } from "node:fs";
import path from "node:path";

// Each file by its path in the drive, and what is in it, with the line
// ends of DOS.
export const FILES = {
  "readme.txt": "Files of the web page's tests, made by the test.\r\n",
  "code/build.bat": "@echo off\r\necho Commands cannot be run here.\r\n",
  "code/hello.c": "#include <stdio.h>\r\n\r\nint main(void)\r\n{\r\n" +
                  "    puts(\"Hello\");\r\n    return 0;\r\n}\r\n",
  "data/cities.csv": "city,country\r\nKyiv,Ukraine\r\nLviv,Ukraine\r\n",
  "data/dos.txt": "DOS ends a line with CR LF.\r\n",
  "notes/ideas.md": "# Ideas\r\n\r\n- one drive\r\n",
  "notes/todo.txt": "Things to do:\r\n  - test the page\r\n",
};

// The files laid out in the directory `root`.
export function layFiles(root) {
  for (const [name, text] of Object.entries(FILES)) {
    const at = path.join(root, ...name.split("/"));
    mkdirSync(path.dirname(at), { recursive: true });
    writeFileSync(at, text);
  }
}
