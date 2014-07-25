<?hh
class UVHttpServer
{
    private ?resource $_rs = null;
    private ?UVTcp $server = null;
    protected array $callbacks = [];

    <<__Native>> private function r3TreeCreate(int $n = 10):void;
    <<__Native>> private function r3TreeFree():void;

    function __construct(string $host, int $port):void
    {
        $this->r3TreeCreate();
        $this->server = new UVTcp();
        $this->server->listen($host, $port, function($server){
            new UVHttpClient($server->accept(), function(UVHttpCLient $client){
                ($this->callbacks[0])($client);
            });
        });        
    }
    
    function __destruct()
    {
        if($this->_rs !== null){
            $this->r3TreeFree();
            $this->_rs = null;
        }
    }
    
    function onRequest(string $pattern, mixed $callback, ?array<string> $allowMethod = null)
    {    
        $this->callbacks[] = $callback;
    }
}
