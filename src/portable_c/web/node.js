// The pre-js of the program under Node, the link web-node of the piece W2
// of step 7 (TODO.md, "Step 7, the web page"): what the program reads of
// the environment, which Emscripten's own does not take from Node's,
// copied there before main() runs. VC_DRIVE_C and VC_DRIVE_D name the
// directories that are C: and D:, VC_LABEL_C and VC_LABEL_D their labels,
// and VC_PROGRAM_DRIVE the drive whose directory VC has the program's own
// files (sys_web.c); VC_TEST, VC_TEST_CLOCK and VC_TRACE are the test
// channel's and the trace's, VC_MEMORY the cap of the memory for a test
// (machine_posix.c), and TZ the time zone of the files' times.
Module.preRun = (Module.preRun || []).concat(() => {
  for (const name of ['VC_DRIVE_C', 'VC_DRIVE_D', 'VC_LABEL_C',
                      'VC_LABEL_D', 'VC_PROGRAM_DRIVE', 'VC_TEST',
                      'VC_TEST_CLOCK', 'VC_TRACE', 'VC_MEMORY', 'TZ']) {
    if (process.env[name] !== undefined) {
      ENV[name] = process.env[name];
    }
  }
});
