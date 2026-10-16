// The pre-js of the web page, the piece W3b of step 7 (TODO.md, "Step 7,
// the web page"): hands the page (page.js) Emscripten's file system and
// environment, which are the program's own and not on Module, just before
// the program starts, so that the page loads the drives into the file
// system and names them in the environment (VC_DRIVE_C and VC_DRIVE_D,
// sys_web.c).
Module.preRun = (Module.preRun || []).concat(() => {
  if (Module.vcBegin) {
    Module.vcBegin({ FS, ENV, addRunDependency, removeRunDependency });
  }
});
