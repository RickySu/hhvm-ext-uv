<?php
require __DIR__ . '/../../test-tools.php';
$loop = UVLoop::defaultLoop();
$count = 0;
$idle = new UVIdle(null);
$idle->start(function($idle) use(&$count){
    if($count++ > 10){
        $idle->stop();
    }   
});
$loop->run();
True($count > 10, "UVIdle");