<?hh
<<__NativeData("UVTcp")>>
class UVTcp
{
    private ?UVLoop $loop = null;
    <<__Native>> function __construct(?UVLoop $loop = null):void;
    <<__Native>> function listen(string $host, int $port, mixed $onConnectCallback):int;
    <<__Native>> function connect(string $host, int $port, mixed $onConnectCallback):int;
    <<__Native>> function shutdown(mixed $onShutdownCallback):int;
    <<__Native>> function accept(): UVTcp;
    <<__Native>> function close(): void;
    <<__Native>> function setCallback(mixed $onRead, mixed $onWrite, mixed $onError):void;
    <<__Native>> function write(string $buf): int;
    <<__Native>> function getSockname(): string;
    <<__Native>> function getPeername(): string;
    <<__Native>> function getSockport(): int;
    <<__Native>> function getPeerport(): int;
}
