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
->onRequest('/post/{id:\\d+}/{foo}', function(UVHttpClient $client, $id, $foo){
    $client->sendReply("match /post/$id/$foo");
})
->onDefaultRequest(function(UVHttpClient $client){
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

## benchmark

[benchmark](https://gist.github.com/RickySu/8edb9bcc58829e5478ac)

## LICENSE

This software is released under MIT License.
