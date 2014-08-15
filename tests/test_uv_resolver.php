<?php
require __DIR__ . '/../test-tools.php';
$resolver = new UVResolver();
$nameinfook = false;
$addrinfook = false;
$hostexpect = gethostbyaddr("127.0.0.1");
$resolver->getnameinfo("127.0.0.1", function($status, $host, $service) use(&$nameinfook, $hostexpect){
    $nameinfook =  "$status, $host, $service" == "0, $hostexpect, 0";
});

$addrexpect = gethostbyname($hostexpect);
$resolver->getaddrinfo("localhost", null,function($status, $addr) use(&$addrinfook, $addrexpect){
    $addrinfook = "$status, $addr" == "0, $addrexpect";
});
UVLoop::defaultLoop()->run();
True($nameinfook, "getnameinfo");
True($addrinfook, "getaddrinfo");