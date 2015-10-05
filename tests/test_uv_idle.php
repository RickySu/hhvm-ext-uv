<?php
require __DIR__ . '/../test-tools.php';

$count = 0;
$loop = new UVLoop();
$idle = new UVIdle($loop);
$idle->start(function($idle) use(&$count){
    if($count++ > 10){
        $idle->stop();
    }   
});
$loop->run();
True($count > 10, "UVIdle");