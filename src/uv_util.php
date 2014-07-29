<?hh
class UVUtil
{
    const UV_EOF = -4095;
    final private function __construct():void {}
    <<__Native>> static function version(): int;
    <<__Native>> static function versionString(): string;
    <<__Native>> static function errorMessage(int $err): string;
}
