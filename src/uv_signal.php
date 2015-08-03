<?hh
<<__NativeData("UVSignal")>>
class UVSignal
{
    private ?UVLoop $loop = null;
    <<__Native>> function __construct(UVLoop $loop):void;
    <<__Native>> function start(mixed $cb, int $signo):int;
    <<__Native>> function stop():int;
    <<__Native>> function __destruct():void;
}
