<?php
require __DIR__ . '/../../test-tools.php';

$count = 0;
$idle = new UVIdle();
$idle->start(function($idle) use(&$count){
    if($count++ > 10){
        $idle->stop();
    }   
});
UVLoop::defaultLoop()->run();
True($count > 10, "UVIdle");