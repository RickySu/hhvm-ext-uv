<?php
require __DIR__ . '/../../test-tools.php';
$loop = new UVLoop();
$tickonece = [];
$tick = [];

$time = microtime(true);
$timerOnece = new UVTimer($loop);
$timerOnece->start(function($timer)use(&$tickonece, $time){
    $tickonece[] = round((microtime(true)-$time)*10);
}, 500);

$timer = new UVTimer($loop);
$timer->start(function($timer)use(&$tick, $time){
    $tick[] = floor((microtime(true)-$time)*10);
    if(count($tick)>5){
        $timer->stop();
    }
}, 500, 500);

$loop->run();
$timerOnece->stop();
$timerOnece = null;
$timer = null;
Equal(array(5), $tickonece, "timer run onece");
Equal(array(5, 10, 15, 20, 25, 30), $tick, "timer run repeat");
