<?php
require __DIR__ . '/../test-tools.php';
$host = '127.0.0.1';
$port = 8888;
$pid = pcntl_fork();
if($pid){
    usleep(100000);    

    $fp = stream_socket_client("tcp://$host:$port", $errno, $errstr);
    $randomData = sha1(rand());
    fwrite($fp, $randomData);
    Equal($randomData, fread($fp, 1000), "Echo Server");
    fclose($fp);
    
    $fp = stream_socket_client("tcp://$host:$port", $errno, $errstr);
    fclose($fp);
    
    $fp = stream_socket_client("tcp://$host:$port", $errno, $errstr);
    Equal('client closed', fread($fp, 1000), "Client close Trigger");
    fclose($fp);
    
    $fp = stream_socket_client("tcp://$host:$port", $errno, $errstr);
    fwrite($fp, '!close!');
    fclose($fp);
    
    pcntl_waitpid($pid, $status);
    exit;
}

$loop = new UVLoop();
$server = new UVTcp($loop);
$server->clientCloseTriggered = false;
True($server->listen($host, $port, function($server) {
    $client = $server->accept();
    $client->setCallback(function($client, $recv) use($server){
        $client->write($recv);    
        if($recv == '!close!'){            
            $server->close();
        }
    }, function($client, $status){
        $client->close();
    }, function($client) use($server){
        $server->clientCloseTriggered = true;
        $client->close();
    });
    
    if($server->clientCloseTriggered){
        $client->write("client closed");
        $server->clientCloseTriggered = false;
    }    
}), "Server listen");
$loop->run();
$server = null; //FIXME: prevent strange hhvm Assertion `!MemoryManager::sweeping()' failed.
