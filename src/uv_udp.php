<?hh
class UVUdp
{
    private ?resource $_rs = null;
    <<__Native>> function __construct():void;
    <<__Native>> function __destruct():void;
    <<__Native>> function bind(string $host, int $port):bool;
    <<__Native>> function close(): void;
    <<__Native>> function setCallback(mixed $onRecv, mixed $onSend, mixed $onError): int;
    <<__Native>> function sendTo(string $dest, int $port, string $buf): bool;
    <<__Native>> function getSockname(): string;
    <<__Native>> function getSockport(): int;
}
