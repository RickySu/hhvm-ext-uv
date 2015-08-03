<?hh
<<__NativeData("UVResolver")>>
class UVResolver
{
    private ?UVLoop $loop = null;
    public function __construct(UVLoop $loop):void{
        $this->loop = $loop;
    }
    <<__Native>> function getaddrinfo(string $node, mixed $service, mixed $callbck): int;
    <<__Native>> function getnameinfo(string $addr, mixed $callbck): int;
}
