<?hh
class UVHttpClient
{
    private int $dataSizeInBuffer = 0;
    private bool $closeOnEmpty = false;
    protected ?UVTcp $client = null;
    protected ?callable $onRead = null;
    protected ?callable $onWrite = null;
    protected ?callable $onError = null;
    protected ?array<string, string> $http = null;
    protected ?callable $onRequest = null;
    
    function __construct(UVTcp $client, ?callable $onRequest)
    {
        $client->customData = $this;    
        $this->client = $client;
        $this->initCallback();
        $client->setCallback($this->onRead, $this->onWrite, $this->onError);
        $this->http = [
            'request' => null,
            'header' => null,
            'body' => null,        
        ];
        $this->onRequest = $onRequest;
    }
    
    public function getRequest(): array<string, string>
    {
        return $this->http;
    }
    
    public function write(string $data)
    {
        $this->dataSizeInBuffer += strlen($data);
        $this->client->write($data);
    }

    public function sendReplyStart(int $status = 200, array $headers = array())
    {
        $headers = array_merge($headers, [
            'Content-Type' => 'text/plain;charset=UTF-8',
            'Transfer-Encoding' => 'chunked',
        ]);
        foreach($headers as $key => $val){
            $rawHeader .= "$key: $val\r\n";
        }
        $this->write("HTTP/1.1 $status OK\r\n$rawHeader\r\n");
    }
    
    public function sendReplyChunk(mixed $content = '')
    {
        $this->write(sprintf("%x\r\n%s\r\n", strlen($content), $content));
    }
    
    public function sendReplyEnd()
    {
        $this->write("0\r\n\r\n");
    }
    
    public function sendReply(mixed $content = '', int $status = 200, array $headers = array(), string $httpVersion = '1.0')
    {
        $headers = array_merge($headers, [
            'Content-Type' => 'text/plain;charset=UTF-8',
            'Content-Length' => strlen($content),
        ]);
        foreach($headers as $key => $val){
            $rawHeader .= "$key: $val\r\n";
        }
        $this->write("HTTP/$httpVersion $status OK\r\n$rawHeader\r\n$content");
    }
    
    protected function requestParser(string $rawHeader): ?array<string, string>
    {
        $pos = strpos($rawHeader, "\r\n");
        $line = substr($rawHeader, 0, $pos);
        list($method, $query, $protocol) = explode(' ', $line);
        $pos = strpos($query, '?');
        if($pos === false){
            $uri = $query;
            $parameters = [];
        }
        else{
            $uri = substr($query, 0, $pos);
            parse_str(substr($query, $pos + 1), $parameters);
        }
        return [
            'method' => $method,
            'uri' => $uri,            
            'parameters' => $parameters,
            'protocol' => $protocol,
        ];
    }
    
    protected function headerParser(string $rawHeader): ?array<string, string>
    {
        $header = null;
        foreach(explode("\r\n", $rawHeader) as $line){
            $pos = strpos($line, ':');
            if($pos === false) {
                continue;
            }
            $header[strtolower(trim(substr($line, 0, $pos)))] = trim(substr($line, $pos + 2));
            
        }
        return $header;
    }
    
    protected function initHeader(string $data): bool
    {
        $pos = strpos($data, "\r\n\r\n");
        if($pos === false){
            $this->http['header'] .= $data;
        }
        else{                
            $this->http['header'] .= substr($data, 0, $pos);
            $this->http['body'] .= substr($data, $pos + 4);
        }
        $this->http['request'] = $this->requestParser($this->http['header']);
        $this->http['header'] = $this->headerParser($this->http['header']);
        return $this->http['header'] !== null;
    }
    
    protected function initCallback(): void
    {
        $this->onRead = function(UVTcp $client, string $data)
        {
            if($this->http['header'] === null) {
                // header uninitialized
                if($this->initHeader($data)){
                    ($this->onRequest)($this);
                }
                return;
            }
            $this->http['body'] .= $data;
        };
        
        $this->onWrite = function(UVTcp $client, int $status, int $sendSize)
        {
            if($status != 0){
                $this->close();
                return;
            }

            $this->dataSizeInBuffer -= $sendSize;
            
            if($this->closeOnEmpty && $this->dataSizeInBuffer == 0){
                $this->close();
            }
        };
        
        $this->onError = function(UVTcp $client)
        {
            $this->close();
        };
    }
    
    public function setCloseOnBufferEmpty(bool $v = true)
    {
        $this->closeOnEmpty = $v;
    }
    
    public function close(): void
    {
        if($this->client !== null){
            $this->client->close();
            $this->client->customData = null;
            $this->client = null;
        }
    }
    
}
