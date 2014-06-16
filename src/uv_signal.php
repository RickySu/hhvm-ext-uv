<?hh
final class UVSignal
{
    <<__Native>> function __construct(UVLoop $loop):void;
//    <<__Native>> function __destruct():void;
    <<__Native>> function start(callable $cb, int $signo):int;
    <<__Native>> function stop():int;
}
