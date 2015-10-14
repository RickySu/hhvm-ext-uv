<?php
require __DIR__ . '/../../test-tools.php';
$host = '127.0.0.1';
$port = rand(5000, 9000);
$pid = pcntl_fork();
if($pid){
    usleep(100000);    

    $fp = stream_socket_client("udp://$host:$port", $errno, $errstr);
    $randomData = sha1(rand());
    fwrite($fp, $randomData);
    $recv = fread($fp, strlen($randomData));
    usleep(500000);
    Equal($randomData, $recv, "Client Echo Recv");
    fclose($fp);
    
    pcntl_waitpid($pid, $status);
    exit;
}

function recvCallback($server, $clientIP, $clientPort, $data, $flag){
    $server->sendTo($clientIP, $clientPort, $data);
}

function sendCallback($server, $clientIP, $clientPort, $status){
    True($status == 0, "Server Send");
    $server->close();
}

function errorCallback(){
}

$loop = new UVLoop();
$server = new UVUdp($loop);
Equal(0, $server->bind($host, $port), "Server Bind");
Equal("$host:$port", "{$server->getSockname()}:{$server->getSockport()}", "getSockname() getSockPort()");
Equal(0, $server->setCallback('recvCallback', 'sendCallback', 'errorCallback'), "setCallback");
$loop->run();
