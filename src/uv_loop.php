<?hh
class UVLoop
{
    private static ?UVLoop $loop = null;

    public static function defaultLoop():UVLoop
    {
        if(self::$loop === null){
            self::$loop = new self();
        }
        return self::$loop;
    }
    
    final private function __construct():void {}
    <<__Native>> function run(int $option = self::RUN_DEFAULT):void;
    <<__Native>> function alive():int;
    <<__Native>> function updateTime():void;
    <<__Native>> function now():int;
    <<__Native>> function backendFd():int;
    <<__Native>> function backendTimeout():int;
}
