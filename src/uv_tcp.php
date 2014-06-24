<?hh
class UVTcp
{
    private ?resource $_rs = null;
    <<__Native>> function __construct(UVLoop $loop):void;
    <<__Native>> function __destruct():void;
    <<__Native>> function listen(string $host, int $port, callable $onConnectCallback):bool;
    <<__Native>> function accept(): UVTcp;
    <<__Native>> function close(): void;
    <<__Native>> function setCallback(callable $onRead, callable $onWrite): void;
}
