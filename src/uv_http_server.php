<?hh
class UVHttpServer
{
    private resource $_rs = null;
    private ?UVTcp $server = null;
    protected array $callbacks = [];
    private ?string $host = '0.0.0.0';
    private int $port = 80;

    <<__Native>>function __construct():void;
    <<__Native>>private function _R3Compile():void;
    
    function listen(string $host, int $port):UVHttpServer
    {
        $this->host = $host;
        $this->port = $port;    
    }
    
    function onRequest(string $pattern, mixed $callback, ?array<string> $allowMethod = null):UVHttpServer
    {    
        $this->callbacks[] = $callback;
    }
    
    function start():int
    {
        $this->_R3Compile();    
        $this->server = new UVTcp();
        return $this->server->listen($this->host, $this->port, function($server){
            new UVHttpClient($server->accept(), function(UVHttpCLient $client){
                ($this->callbacks[0])($client);
            });
        });    

    }
    
}
