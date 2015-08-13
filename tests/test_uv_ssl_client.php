<?php
require __DIR__ . '/../test-tools.php';
$html = '';
$host = gethostbyname('github.com');
$loop = new UVLoop();
$ssl = new UVSSL($loop);
$ssl->connect($host, 443, function($ssl) use(&$html){
    $ssl->setSSLHandshakeCallback(function($ssl) use(&$html){
        echo "handshake: ok\n";
        $ssl->setCallback(function($ssl, $recv) use(&$html){
            $html.=$recv;
        }, function(){}, function($ssl) use(&$html){
            if(($pos = strpos($html, "\r\n\r\n")) !== false){
               $header = substr($html, 0, $pos);
               preg_match('/Status:\s(\d+)/i', $header, $match);
               Equal(200, $match[1], "http status:");
            }
            $ssl->close();
        });
        $request = "GET /RickySu/php_ext_uv HTTP/1.0\r\nUser-Agent: UVSSL\r\nAccept: */*\r\nHost: github.com\r\n\r\n";
        $ssl->write($request);
    });
});

$loop->run();
