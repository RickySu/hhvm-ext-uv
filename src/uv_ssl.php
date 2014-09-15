<?hh
class UVSSL extends UVTcp
{   
    protected ?callable $sslHandshakeCallback;
    <<__Native>> function __construct():void;     
    <<__Native>> function __destruct():void;
    <<__Native>> function accept(): UVSSL;
    <<__Native>> function setCallback(mixed $onRead, mixed $onWrite, mixed $onError):int;
    function setSSLHandshakeCallback($callback):void
    {
        $this->sslHandshakeCallback = $callback;
    }
    <<__Native>> function write(string $buf): int;        
    <<__Native>> function setCertFile(string $certFile):bool;
    <<__Native>> function setCertChainFile(string $certChainFile):bool;
    <<__Native>> function setPrivateKeyFile(string $privateKeyFile):bool;
    <<__Native>> function connect(string $host, int $port, mixed $onConnectCallback):int;
}
