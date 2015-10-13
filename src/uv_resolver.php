<?hh
<<__NativeData("UVResolver")>>
class UVResolver
{
    private ?UVLoop $loop = null;
    <<__Native>> public function __construct(?UVLoop $loop = null):void;
    <<__Native>> function getaddrinfo(string $node, mixed $service, mixed $callbck): int;
    <<__Native>> function getnameinfo(string $addr, mixed $callbck): int;
}
