<?php
require __DIR__ . '/../test-tools.php';
$host = '0.0.0.0';
$port = 8889;
$pid = pcntl_fork();
if($pid){
    usleep(100000);
    $fp = stream_socket_client("tcp://127.0.0.3:$port", $errno, $errstr);
    $randomData = sha1(rand());
    fwrite($fp, $randomData);
    $resp = unserialize(fread($fp, 1000));
    fclose($fp);
    Equal('127.0.0.3', $resp['local'][0], "Client socket local address");
    True($resp['local'][1] != -1, "Client socket local port");
    Equal('127.0.0.1', $resp['remote'][0], "Client socket remote address");
    True($resp['remote'][1] != -1, "Client socket remote port");    
    pcntl_waitpid($pid, $status);
    exit;
}

$loop = UVLoop::defaultLoop();
$server = new UVTcp();
Equal(0, $server->listen($host, $port, function($server) use($host, $port){
    Equal("$host:$port", "{$server->getSockname()}:{$server->getSockport()}", "Server socket address");
    $client = $server->accept();
    $client->setCallback(function($client, $recv) use($server){
        $client->write(serialize(array(
            'local' => array($client->getSockname(), $client->getSockport()),
            'remote' => array($client->getPeername(), $client->getPeerport()),
        )));
    }, function($client, $status) use($server){
        $client->close();
        $server->close();
    }, function($client) use($server){
        $client->close();
        $server->close();
    });
}), "Server Listen");
$loop->run();
$server = null; //FIXME: prevent strange hhvm Assertion `!MemoryManager::sweeping()' failed.
