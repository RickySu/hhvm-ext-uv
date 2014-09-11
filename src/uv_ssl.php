<?hh
class UVSSL extends UVTcp
{   
    const SSL_HANDSHAKE_FINISH = 99999;
    <<__Native>> function __construct():void;     
    <<__Native>> function __destruct():void;
    <<__Native>> function accept(): UVSSL;
    <<__Native>> function setCallback(mixed $onRead, mixed $onWrite, mixed $onError):int;
    <<__Native>> function write(string $buf): int;        
    <<__Native>> function setCertFile(string $certFile):bool;
    <<__Native>> function setCertChainFile(string $certChainFile):bool;
    <<__Native>> function setPrivateKeyFile(string $privateKeyFile):bool;
}
