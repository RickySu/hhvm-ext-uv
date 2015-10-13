<?hh
<<__NativeData("UVLoop")>>
class UVLoop
{
    private static ?UVLoop $loop = null;
    
    final public static function defaultLoop():UVLoop
    {
        if(self::$loop === null){
            self::$loop = self::makeDefaultLoop();
        }
        return self::$loop;
    }
    
    <<__Native>> private static function makeDefaultLoop():UVLoop;
    <<__Native>> function run(int $option = self::RUN_DEFAULT):void;
    <<__Native>> function stop():void;
    <<__Native>> function alive():int;
    <<__Native>> function updateTime():void;
    <<__Native>> function now():int;
    <<__Native>> function backendFd():int;
    <<__Native>> function backendTimeout():int;
}
