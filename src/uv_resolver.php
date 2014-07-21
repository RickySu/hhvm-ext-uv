<?hh
class UVResolver
{
    private ?resource $_rs = null;
    <<__Native>> function __construct():void;
    <<__Native>> function getaddrinfo(string $node, mixed $service, mixed $callbck): int;
    <<__Native>> function getnameinfo(string $addr, mixed $callbck): int;
}
