<?hh
final class UVLoop
{
    const UV_RUN_DEFAULT = 0;
    const UV_RUN_ONCE = 1;
    const UV_RUN_NOWAIT = 2;
    <<__Native>> function __construct():void;
    <<__Native>> function __destruct():void;
    <<__Native>> function run(int $option = self::UV_RUN_DEFAULT):void;
    <<__Native>> function alive():int;
}
