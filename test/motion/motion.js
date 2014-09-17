var IOLIB = require('./device');

var io = new IOLIB.IO({
  log: true,
  quickInit: false
});

var handle = io.mug_motion_init();

io.mug_motion_on(handle, function() {
  console.log(JSON.stringify(arguments));
});

io.mug_motion_angle_on(handle, function(angleX, angleY, angleZ) {
  console.log("( " + angleX + ", " + angleY + ", " + angleZ + " )");
});

io.mug_set_motion_timer(handle, 500);

io.mug_run_motion_watcher(handle);
