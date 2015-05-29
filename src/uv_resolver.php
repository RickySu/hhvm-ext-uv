<?hh
<<__NativeData("UVResolver")>>
class UVResolver
{
    <<__Native>> function getaddrinfo(string $node, mixed $service, mixed $callbck): int;
    <<__Native>> function getnameinfo(string $addr, mixed $callbck): int;
}
