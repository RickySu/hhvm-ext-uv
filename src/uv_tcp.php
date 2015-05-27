<?hh
class UVTcp
{
    protected ?resource $_rs;
    
    protected ?callable $connectCallback;
    protected ?callable $readCallback;
    protected ?callable $writeCallback;
    protected ?callable $errorCallback;
    protected ?callable $shutdownCallback;

    final public function __clone()
    {
        $this->_rs = null;
        $this->connectCallback = null;
        $this->readCallback = null;
        $this->writeCallback = null;
        $this->errorCallback = null;
        $this->shutdownCallback = null;
    }
        
    public function makeCallable(mixed $callback):callable
    {
        if(is_string($callback)){
            return fun($callback);
        }
        
        if(is_array($callback)){
            list($object, $method) = $callback;
            if(is_string($oject)){
                return class_meth($callback);
            }
            return inst_meth($callback);
        }
        return $callback;
    }
    
    <<__Native>> function __construct():void;
    <<__Native>> function __destruct():void;
    <<__Native>> function listen(string $host, int $port, mixed $onConnectCallback):int;
    <<__Native>> function connect(string $host, int $port, mixed $onConnectCallback):int;
    <<__Native>> function shutdown(mixed $onShutdownCallback):int;
    <<__Native>> function accept(): UVTcp;
    <<__Native>> function close(): void;
    <<__Native>> function setCallback(mixed $onRead, mixed $onWrite, mixed $onError):int;
    <<__Native>> function write(string $buf): int;
    <<__Native>> function getSockname(): string;
    <<__Native>> function getPeername(): string;
    <<__Native>> function getSockport(): int;
    <<__Native>> function getPeerport(): int;
}
