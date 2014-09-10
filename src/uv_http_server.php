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

    private ?resource $_rs = null;
    private ?UVTcp $server = null;
    protected array $routes = [];
    protected ?mixed $defaultCallback = null;
    protected ?string $host;
    protected ?int $port;

    <<__Native>>private function _R3RoutesAdd(array $routes):mixed;
    <<__Native>>private function _R3Match(string $uri, int $method):array;

    function setSocket(UVTcp $socket):void
    {
        $this->server = $socket;
    }
    
    protected function getServer():UVTcp
    {
        if($this->server === null){
            $this->server = new UVTcp();
        }
        return $this->server;
    }
    
    function start():void
    {
        $this->_R3RoutesAdd($this->routes);
        $server = $this->getServer();
        $server->listen($this->host, $this->port, function($server){
            new UVHttpSocket($server->accept(), function(UVHttpSocket $client) {
                if($this->routes){
                    $result = $this->_R3Match($client->getRequest()['request']['uri'], $this->convertMethod([$client->getRequest()['request']['method']]));
                    if($result){
                        if($result[1]){
                            call_user_func_array($this->routes[$result[0]][2], array_merge(array($client), $result[1]));
                        }
                        else{
                            ($this->routes[$result[0]][2])($client);
                        }
                        return;
                    }
                }
                ($this->defaultCallback)($client);
            });
        });
    }

    function __construct(string $host, int $port): void
    {
        $this->defaultCallback = function(UVHttpSocket $client){
            $client->sendReply('Page not found.', 404);
        };
        $this->host = $host;
        $this->port = $port;

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
        return $this;
    }

    function onDefaultRequest(mixed $callback):UVHttpServer
    {
        $this->defaultCallback = $callback;
        return $this;
    }

}
