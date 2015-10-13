<?php
require __DIR__ . '/../../test-tools.php';
$host = '127.0.0.1';
$port = 8443;
$pid = pcntl_fork();
function createSSLSocketClient($host, $port){
    $contextOptions = array(
        'ssl' => array(
            'verify_peer'   => false,
        ),
    );
    $sslContext = stream_context_create($contextOptions);
    $fp = stream_socket_client("ssl://$host:$port", $errno, $errstr, ini_get("default_socket_timeout"), STREAM_CLIENT_CONNECT, $sslContext);
    return $fp;
}
if($pid){
    usleep(100000);    
    $fp = createSSLSocketClient($host, $port);
    $randomData = sha1(rand());
    fwrite($fp, $randomData);
    Equal($randomData, fread($fp, 1000), "Echo Server");
    fclose($fp);
    
    $fp = createSSLSocketClient($host, $port);
    fclose($fp);
    
    $fp = createSSLSocketClient($host, $port);
    Equal('client closed', $a = fread($fp, 1000), "Client close Trigger");
    fclose($fp);
    
    $fp = createSSLSocketClient($host, $port);
    fwrite($fp, '!close!');
    fclose($fp);
    
    pcntl_waitpid($pid, $status);
    exit;
}

$server = new UVSSL();
$server->setCert(file_get_contents(__DIR__."/../cert/server.crt"));
$server->setPrivateKey(file_get_contents(__DIR__."/../cert/server.key"));
$server->clientCloseTriggered = false;
Equal(0, $server->listen($host, $port, function($server) {

    $client = $server->accept();
    $client->setSSLHandshakeCallback(function($client) use($server){
        if($server->clientCloseTriggered){
            $client->write("client closed");
            $server->clientCloseTriggered = false;
        }    
    });
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
    

}), "Server listen");
UVLoop::defaultLoop()->run();