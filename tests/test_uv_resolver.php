<?php
require __DIR__ . '/../test-tools.php';
$resolver = new UVResolver();
$nameinfook = false;
$addrinfook = false;
$resolver->getnameinfo("127.0.0.1", function($status, $host, $service) use(&$nameinfook){
    $nameinfook =  "$status, $host, $service" == "0, localhost, 0";
});

$resolver->getaddrinfo("localhost", null,function($status, $host) use(&$addrinfook){
    $addrinfook = "$status, $host" == "0, 127.0.0.1";
});
UVLoop::defaultLoop()->run();
True($nameinfook, "getnameinfo");
True($addrinfook, "getaddrinfo");