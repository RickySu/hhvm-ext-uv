<?hh
class UVSSL extends UVTcp
{   
    <<__Native>> function __construct(UVLoop $loop, int $sslMethod = self::SSL_METHOD_TLSV1, int $nContexts = 1):void;
    <<__Native>> function accept():UVSSL;
    <<__Native>> function setSSLHandshakeCallback(mixed $callback):void;
    <<__Native>> function setSSLServerNameCallback(mixed $callback):void;
    <<__Native>> function write(string $buf): int;        
    <<__Native>> function setCert(string $cert, int $n = 0):bool;
    <<__Native>> function setPrivateKey(string $privateKey, int $n = 0):bool;
    <<__Native>> function connect(string $host, int $port, mixed $onConnectCallback):int;
}
