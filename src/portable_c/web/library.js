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
//   Module.vcWhere  (host): the directory of the file system that is DOS's
//                   current one, told at every draw;
//   Module.vcTestEvent, Module.vcTestScreen: a test's, which page.js puts
//                   there where the page is tested (its test hook).
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

  web_where__deps: ['$UTF8ToString'],
  web_where: (host) => {
    if (Module.vcWhere) {
      Module.vcWhere(UTF8ToString(host));
    }
  },

  // A test of the page (harness/webpagerun.mjs): the event of the test
  // channel's keys of that number given by the page, as a user's keys
  // and mouse come, waited for; and a screen that the channel has
  // recorded, which the page has just drawn, told by its kind and number
  // and waited for, while the test reads it from the page.
  // Where the page has no test, nothing is asked or told.
  web_test_event__async: true,
  web_test_event: (event) => Asyncify.handleAsync(async () => {
    if (Module.vcTestEvent) {
      await Module.vcTestEvent(event);
    }
  }),

  web_test_screen__deps: ['$UTF8ToString'],
  web_test_screen__async: true,
  web_test_screen: (kind, number) => Asyncify.handleAsync(async () => {
    if (Module.vcTestScreen) {
      await Module.vcTestScreen(UTF8ToString(kind), number >>> 0);
    }
  }),
});
