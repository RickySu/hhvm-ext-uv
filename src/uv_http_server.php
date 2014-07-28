<?hh
class UVHttpServer
{
    const METHOD_GET = 2;
    const METHOD_POST = 4;
    const METHOD_PUT = 8;
    const METHOD_DELETE = 16;
    const METHOD_PATCH = 32;
    const METHOD_HEAD = 64;
    const METHOD_OPTIONS = 128;
    
    private ?UVTcp $server = null;
    protected array $routes = [];
    protected ?mixed $defaultCallback = null;

    <<__Native>>private function _R3Match(array $routes, string $uri, int $method):array;
    
    function __construct(string $host, int $port): void
    {
        $this->defaultCallback = function(UVHttpCLient $client){
            $client->sendReply('Page not found.', 404);
            $client->setCloseOnBufferEmpty();
        };
        $this->server = new UVTcp();
        return $this->server->listen($host, $port, function($server){
            new UVHttpClient($server->accept(), function(UVHttpCLient $client){
                $result = $this->_R3Match($this->routes, $client->getRequest()['request']['uri'], $this->convertMethod([$client->getRequest()['request']['method']]));
                if($result){
                    call_user_func_array($this->routes[$result[0]][2], array_merge(array($client), $result[1]));
                    return;
                }
                ($this->defaultCallback)($client);
            });
        });        
    }
    
    protected function convertMethod(?array<string> $allowMethod):int
    {
        $allowMethodPacked = 0;
        if(is_array($allowMethod)){
            foreach($allowMethod as $method){
                switch(strtoupper($method)){
                    case 'GET':
                        $allowMethodPacked|=self::METHOD_GET;
                        break;
                    case 'POST':
                        $allowMethodPacked|=self::METHOD_POST;
                        break;                                            
                    case 'PUT':
                        $allowMethodPacked|=self::METHOD_PUT;
                        break;                    
                    case 'DELETE':
                        $allowMethodPacked|=self::METHOD_DELETE;
                        break;                    
                    case 'PATCH':
                        $allowMethodPacked|=self::METHOD_PATCH;
                        break;                    
                    case 'HEAD':
                        $allowMethodPacked|=self::METHOD_HEAD;
                        break;
                    case 'OPTIONS':
                        $allowMethodPacked|=self::METHOD_OPTIONS;
                        break;
                }
            }
        }
        return $allowMethodPacked;
    }
    
    function onRequest(string $pattern, mixed $callback, ?array<string> $allowMethod = null):UVHttpServer
    {    
        $this->routes[] = array(
            $pattern,            
            $this->convertMethod($allowMethod),            
            $callback,
        );
    }
    
}
