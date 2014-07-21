<?hh
class UVSignal
{
    private ?resource $_rs = null;
    <<__Native>> function __construct():void;
    <<__Native>> function start(mixed $cb, int $signo):int;
    <<__Native>> function stop():int;
    <<__Native>> function __destruct():void;
}
