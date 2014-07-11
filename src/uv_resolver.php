<?hh
class UVSignal
{
    private ?resource $_rs = null;
    <<__Native>> function __construct(UVLoop $loop):void;
    <<__Native>> function getaddrinfo(string $node, mixed $service, mixed $callbck): bool;
    <<__Native>> function __destruct():void;
}
