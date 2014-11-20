#!/usr/bin/node

var child_process = require('child_process');
var spawn = child_process.spawn;
var exec  = child_process.exec;

var argv = process.argv;
console.log(JSON.stringify(argv));

var isSIGINT = false;

var cmdRun = "gst-launch-0.10 filesrc location=\"" + argv[2] + "\" ! flump3dec ! alsasink device=plughw:1,0";

var cmdKill = "kill -USR1 " + argv[3];

console.log(cmdRun);

var player = spawn("gst-launch-0.10", ["filesrc", "location=\"" + argv[2] + "\"", "!", "flump3dec", "!", "alsasink", "device=plughw:1,0"]);

player.stdout.on('data', function(data) {
  console.log('[stdout] ' + data);
});

player.stderr.on('data', function(data) {
  console.log('[stderr] ' + data);
});

player.on('close', function(code, sig) {
  console.log('player exit code ' + code);
  console.log('player exit signal ' + sig);
  console.log(cmdKill);

  if(!isSIGINT) {
    exec(cmdKill, function() {
    });
  }
});

process.on('SIGINT', function(code) {
  isSIGINT = true;
  console.log('run_player.js captured ctrl c');
  player.kill('SIGINT');
});
