hhvm-ext-uv
================

[![Build Status](https://travis-ci.org/RickySu/hhvm-ext-uv.svg?branch=master)](https://travis-ci.org/RickySu/hhvm-ext-uv)

hhvm-ext-uv is a [libuv](https://github.com/joyent/libuv) 
extension for [HHVM](https://github.com/facebook/hhvm/) 
with high performance asynchronous I/O.

## Feature highlights

* Asynchronous TCP and UDP sockets

* Asynchronous DNS resolution

* Signal handling

* SSL support

* Http 1.1 keep alive support

* High performance web server with [R3](https://github.com/c9s/r3) routing

## Build Requirement

* autoconf
* automake
* boost
* check
* g++ 4.8
* gcc 4.8
* google-glog
* pkg-config
* pcre

Example
--------------------

```php
<?php
// Simple Web Server
$server = new UVHttpServer('127.0.0.1', 8080);
$server
->onRequest('/post/{id:\\d+}/{foo}', function(UVHttpSocket $client, $id, $foo){
    $client->sendReply("match /post/$id/$foo");
})
->onDefaultRequest(function(UVHttpSocket $client){
    $client->sendReply("hello world");
})
->start();
UVLoop::defaultLoop()->run();  //run event loop.
```

```php
// TCP Echo Server
$server = new UVTcp();
$server->listen($host, $port, function($server){
    $client = $server->accept();
    $client->setCallback(function($client, $recv){
        //on receive
        $client->write($recv);
    }, function($client, $status){
        //on data sent
        $client->close();
    }, function(){
        //on error maybe client disconnect.
        $client->close();
    });
});
UVLoop::defaultLoop()->run();
```

```php
//SSL Echo Server
$server = new UVSSL();
$server->setCert(file_get_contents("server.crt"));    //PEM format
$server->setPrivateKey(file_get_contents("server.key"));  //PEM format
$server->listen($host, $port, function($server){
    $client = $server->accept();
    $client->setSSLHandshakeCallback(function($client){
        echo "ssl handshake ok\n";
    });
    $client->setCallback(function($client, $recv){
        //on receive if ssl handshake finished.
        //otherwise you won't receive any data before ssl handshake finished
        $client->write($recv);
    }, function($client, $status){
        //on data sent
        $client->close();
    }, function(){
        //on error maybe client disconnect.
        $client->close();
    });
});
```
[Server Name Indication(SNI)](http://en.wikipedia.org/wiki/Server_Name_Indication)

```php
//SSL Echo Server with SNI support
//SNI is an extension to the TLS 
$server = new UVSSL(UVSSL::SSL_METHOD_TLSV1, 2); //with 2 certs
$server->setCert(file_get_contents("server0.crt"), 0);    //PEM format cert 0
$server->setPrivateKey(file_get_contents("server0.key"), 0);  //PEM format cert 0
$server->setCert(file_get_contents("server1.crt"), 1);    //PEM format cert 1
$server->setPrivateKey(file_get_contents("server1.key"), 1);  //PEM format cert 1
$server->setSSLServerNameCallback(function($servername){ //regist Server Name Indication callback
    if($servername == 'server1'){ 
        return 1; //use cert1
    } 
    return 0;  //default use cert0
});
$server->listen($host, $port, function($server){
    $client = $server->accept();
    $client->setSSLHandshakeCallback(function($client){
        echo "ssl handshake ok\n";
    });
    $client->setCallback(function($client, $recv){
        //on receive if ssl handshake finished.
        //otherwise you won't receive any data before ssl handshake finished
        $client->write($recv);
    }, function($client, $status){
        //on data sent
        $client->close();
    }, function(){
        //on error maybe client disconnect.
        $client->close();
    });
});
```

```php
<?php
// Simple Https Web Server
$ssl = new UVSSL();
$ssl->setCertFile("server.crt");    //PEM format
$ssl->setPrivateKeyFile("server.key");  //PEM format

$server = new UVHttpServer('127.0.0.1', 8443);
$server->setSocket($ssl);
$server
->onRequest('/post/{id:\\d+}/{foo}', function(UVHttpSocket $client, $id, $foo){
    $client->sendReply("match /post/$id/$foo");
})
->onDefaultRequest(function(UVHttpSocket $client){
    $client->sendReply("hello world");
})
->start();
UVLoop::defaultLoop()->run();  //run event loop.
```

## benchmark

[simple http server benchmark](https://gist.github.com/RickySu/8edb9bcc58829e5478ac)

## LICENSE

This software is released under MIT License.
