<?hh
<<__NativeData("UVTimer")>>
class UVTimer
{
    <<__Native>> function __construct():void;
    <<__Native>> function __destruct():void;
    <<__Native>> function start(mixed $callbck, int $start, int $repeat = 0): int;
    <<__Native>> function stop(): int;
}
