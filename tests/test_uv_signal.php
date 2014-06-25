<?php
require __DIR__ . '/../test-tools.php';
$pid = pcntl_fork();
if($pid){
    usleep(100000);
    posix_kill($pid, SIGUSR1);
    pcntl_waitpid($pid, $status);
    exit;
}

$loop = new UVLoop();
$signal = new UVSignal($loop);
$signal->start(function($signal, $signo){
    True(true, "UVSignal");
    $signal->stop();
}, SIGUSR1);
$loop->run();
$signal = null; //FIXME: prevent strange hhvm Assertion `!MemoryManager::sweeping()' failed.