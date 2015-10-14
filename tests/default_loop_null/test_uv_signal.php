<?php
require __DIR__ . '/../../test-tools.php';
$pid = pcntl_fork();
if($pid){
    usleep(100000);
    posix_kill($pid, SIGUSR1);
    pcntl_waitpid($pid, $status);
    exit;
}
$loop = UVLoop::defaultLoop();
$signal = new UVSignal(null);
$signal->start(function($signal, $signo){
    True(true, "UVSignal");
    $signal->stop();
}, SIGUSR1);
$loop->run();