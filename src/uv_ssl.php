<?hh
class UVSSL extends UVTcp
{   
    const SSL_METHOD_SSLV2 = 0;
    const SSL_METHOD_SSLV3 = 1;
    const SSL_METHOD_SSLV23 = 2;
    const SSL_METHOD_TLSV1 = 3;
    const SSL_METHOD_TLSV1_1 = 4;
    const SSL_METHOD_TLSV1_2 = 5;
    
    protected ?callable $sslHandshakeCallback;
    protected ?callable $sslServerNameCallback;
    
    <<__Native>> function __construct(int $sslMethod = self::SSL_METHOD_TLSV1, int $nContexts = 1):void;
    <<__Native>> function __destruct():void;
    <<__Native>> function accept(): UVSSL;
    <<__Native>> function setCallback(mixed $onRead, mixed $onWrite, mixed $onError):int;
    public function setSSLHandshakeCallback($callback):void
    {
        $this->sslHandshakeCallback = $callback;
    }
    public function setSSLServerNameCallback($callback):void
    {
        $this->sslServerNameCallback = $callback;
    }
    <<__Native>> function write(string $buf): int;        
    <<__Native>> function setCert(string $cert, int $n = 0):bool;
    <<__Native>> function setCertChainFile(string $certChainFile):bool;
    <<__Native>> function setPrivateKey(string $privateKey, int $n = 0):bool;
    <<__Native>> function connect(string $host, int $port, mixed $onConnectCallback):int;
}
