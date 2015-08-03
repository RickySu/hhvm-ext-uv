<?hh
<<__NativeData("UVTimer")>>
class UVTimer
{
    private ?UVLoop $loop = null;
    <<__Native>> function __construct(UVLoop $loop):void;
    <<__Native>> function __destruct():void;
    <<__Native>> function start(mixed $callbck, int $start, int $repeat = 0): int;
    <<__Native>> function stop(): int;
}
