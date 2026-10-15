// The page's side of sys_web.c, the piece W2 of step 7 (TODO.md, "Step
// 7, the web page"): an Emscripten library, linked with --js-library, of
// the functions that the program calls when it yields. Each goes by what
// the page has put on Module, and does nothing where the page has put
// nothing, as under Node, where the test channel drives the program:
//
//   Module.vcInput  an array of strings of bytes, the escape codes that
//                   keys.js and mouse.js make of the browser's events,
//                   queued while the program runs and taken here;
//   Module.vcDraw   (cells, place, shape): the 2,000 cells of the grid as
//                   a Uint16Array over the program's memory, valid for the
//                   call alone, the cursor's place and its shape, as
//                   render.js's draw takes them;
//   Module.vcOpen   (host, name): a file of the host to open in a new tab,
//                   with DOS's name of it;
//   Module.vcSync   () => a promise: the store written, which the program
//                   waits for before it ends.
addToLibrary({
  web_input: (to, room) => {
    const queue = Module.vcInput;
    let count = 0;
    while (queue && queue.length > 0 && count < room) {
      const bytes = queue[0];
      const taken = Math.min(bytes.length, room - count);
      for (let i = 0; i < taken; i++) {
        HEAPU8[to + count + i] = bytes.charCodeAt(i) & 0xFF;
      }
      count += taken;
      if (taken === bytes.length) {
        queue.shift();
      } else {
        queue[0] = bytes.slice(taken);
      }
    }
    return count;
  },

  web_draw: (cells, place, shape) => {
    if (Module.vcDraw) {
      Module.vcDraw(HEAPU16.subarray(cells >> 1, (cells >> 1) + 2000),
                    place, shape);
    }
  },

  web_open_file__deps: ['$UTF8ToString'],
  web_open_file: (host, name) => {
    if (Module.vcOpen) {
      Module.vcOpen(UTF8ToString(host), UTF8ToString(name));
    }
  },

  web_sync__async: true,
  web_sync: () => Asyncify.handleAsync(async () => {
    if (Module.vcSync) {
      await Module.vcSync();
    }
  }),
});
