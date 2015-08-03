<?hh
<<__NativeData("UVLoop")>>
class UVLoop
{
    <<__Native>> function run(int $option = self::RUN_DEFAULT):void;
    <<__Native>> function stop():void;
    <<__Native>> function alive():int;
    <<__Native>> function updateTime():void;
    <<__Native>> function now():int;
    <<__Native>> function backendFd():int;
    <<__Native>> function backendTimeout():int;
}
