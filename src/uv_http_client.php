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
    protected ?mixed $onCustomRead = null;
    protected ?mixed $onCustomWrite = null;
    protected ?mixed $onCustomError = null;
    
    function __construct(UVTcp $client, ?callable $onRequest)
    {
        $client->customData = $this;    
        $this->client = $client;
        $this->initCallback();
        $client->setCallback($this->onRead, $this->onWrite, $this->onError);
        $this->http = [
            'request' => null,
            'header' => null,
            'rawheader' => null,
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
        $request = explode(' ', $line);
        if(
            count($request) != 3 ||
            !in_array($request[0], array('GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'HEAD', 'OPTIONS')) ||
            !preg_match('/^HTTP\/\d+\.\d+/', $request[2])
        ){
            //bad request method
            $this->close();
        }
        $pos = strpos($request[1], '?');
        if($pos === false){
            $uri = $request[1];
            $parameters = [];
        }
        else{
            $uri = substr($request[1], 0, $pos);
            parse_str(substr($request[1], $pos + 1), $parameters);
        }
        return [
            'method' => $request[0],
            'uri' => $uri,            
            'parameters' => $parameters,
            'protocol' => $request[2],
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
        $this->http['rawheader'] .= $data;    
        $pos = strpos($this->http['rawheader'], "\r\n\r\n");
        if($pos === false){
            if(strlen($this->http['rawheader']) > 4096){
                $this->close();
            }
            return false;
        }
        else{                
            $this->http['rawheader'] .= substr($data, 0, $pos);
            $this->http['body'] .= substr($data, $pos + 4);
        }
        $this->http['request'] = $this->requestParser($this->http['rawheader']);
        $this->http['header'] = $this->headerParser($this->http['rawheader']);
        unset($this->http['rawheader']);
        return $this->http['header'] !== null;
    }
    
    protected function initCallback(): void
    {
        $this->onRead = function(UVTcp $client, string $data)
        {
            if($this->onCustomRead !== null){
                ($this->onCustomRead)($this, $data);
                return;
            }
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
            if($this->onCustomWrite !== null){
                ($this->onCustomWrite)($this, $statuc, $sendSize);
                return;
            }
            if($status != 0){
                $this->close();
                return;
            }

            $this->dataSizeInBuffer -= $sendSize;
            
            if($this->closeOnEmpty && $this->dataSizeInBuffer == 0){
                $this->close();
            }
        };
        
        $this->onError = function(UVTcp $client, int $nread)
        {
            if($this->onCustomError !== null){
                ($this->onCustomError)($this, $nread);
                return;
            }            
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

    public function onRead(mixed $onRead): void
    {
        $this->onCustomRead = $onRead;
    }
    
    public function onWrite(mixed $onWrite): void
    {
        $this->onCustomWrite = $onWrite;
    }    
    
    public function onError(mixed $onError): void
    {
        $this->onCustomError = $onError;
    }    
}
