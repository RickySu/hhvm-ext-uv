<?hh
class UVSignal
{
    private ?resource $_rs = null;
    <<__Native>> function __construct(UVLoop $loop):void;
    <<__Native>> function start(callable $cb, int $signo):int;
    <<__Native>> function stop():int;
    <<__Native>> function __destruct():void;
}
