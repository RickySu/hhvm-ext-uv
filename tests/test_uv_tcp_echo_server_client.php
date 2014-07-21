<?php
require __DIR__ . '/../test-tools.php';
$host = '127.0.0.1';
$port = 8890;
$pid = pcntl_fork();
if($pid){
    $socket = stream_socket_server("tcp://$host:$port", $errno, $errstr);
    if(!$socket){
        echo "$errstr ($errno)\n";
    }
    
    $conn = stream_socket_accept($socket, 60);
    $read = fread($conn, 1024);
    fwrite($conn, "resp $read");
    fclose($conn);
    pcntl_waitpid($pid, $status);
    exit;
}

$loop = UVLoop::defaultLoop();
$client = new UVTcp();
$randomValue = md5(rand().microtime());

$client->connect($host, $port, function($client, $status) use($randomValue){
    Equal(0, $status, "on connect");
    $client->setCallback(function($client, $recv) use($randomValue){
        Equal("resp $randomValue", $recv, "on read");
    }, function($client, $status){
        Equal(0, $status, "on write");
        $client->shutdown(function($client, $status){
            Equal(0, $status, "on shutdown");
        });
    }, function($client){
        
    });
    Equal(32, strlen($randomValue), "random value");
    $client->write($randomValue);
});

$loop->run();
$client = null; //FIXME: prevent strange hhvm Assertion `!MemoryManager::sweeping()' failed.
