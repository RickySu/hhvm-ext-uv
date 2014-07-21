<?hh
class UVTcp
{
    private ?resource $_rs = null;
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
