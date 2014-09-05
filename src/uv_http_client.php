<?hh
class UVHttpClient
{
    const MAX_HEADER_SIZE = 8192;
    private int $dataSizeInBuffer = 0;
    private bool $closeOnEmpty = false;
    protected ?UVTcp $client = null;
    protected ?callable $onRead = null;
    protected ?callable $onWrite = null;
    protected ?callable $onError = null;
    private bool $keepAlive = false;
    protected ?array<string, string> $http = null;
    protected ?array<string, string> $responseHeaders = null;
    protected ?callable $onRequest = null;
    protected ?mixed $onCustomRead = null;
    protected ?mixed $onCustomWrite = null;
    protected ?mixed $onCustomError = null;

    protected function resetHttpRequest():void
    {
        $this->keepAlive = false;
        $this->http = [
            'request' => null,
            'header' => null,
            'rawheader' => null,
            'body' => null,
        ];
    }

    function __construct(UVTcp $client, ?callable $onRequest):void
    {
        $client->customData = $this;
        $this->client = $client;
        $this->initCallback();
        $client->setCallback($this->onRead, $this->onWrite, $this->onError);
        $this->onRequest = $onRequest;
        $this->resetHttpRequest();
    }

    public function getRequest(): array
    {
        return $this->http;
    }

    public function write(string $data):void
    {
        $this->dataSizeInBuffer += strlen($data);
        $this->client->write($data);
    }

    protected function generateResponseHeader(string $protocolVersion, int $status, array<string, string> $header): string
    {
        $rawHeader = '';
        foreach($header as $key => $val){
            $rawHeader .= "$key: $val\r\n";
        }
        return "HTTP/$protocolVersion $status OK\r\n$rawHeader\r\n";
    }

    public function sendReplyStart(int $status = 200, array $header = array()):void
    {
        $header = array_merge($this->keepAliveHeader('1.1'), [
            'Content-Type' => 'text/plain;charset=UTF-8',
            'Transfer-Encoding' => 'chunked',
        ], $header);
        $this->write($this->generateResponseHeader('1.1', $status, $header));
    }

    public function sendReplyChunk(mixed $content = ''):void
    {
        $this->write(sprintf("%x\r\n%s\r\n", strlen($content), $content));
    }

    public function sendReplyEnd():void
    {
        $this->write("0\r\n\r\n");
        $this->keepAliveConnection();
    }

    public function sendReply(mixed $content = '', int $status = 200, array $header = array(), string $protocolVersion = '1.1'):void
    {
        $header = array_merge($this->keepAliveHeader($protocolVersion), [
            'Content-Type' => 'text/plain;charset=UTF-8',
            'Content-Length' => strlen($content),
        ], $header);
        $this->write("{$this->generateResponseHeader($protocolVersion, $status, $header)}$content");
        $this->keepAliveConnection();
    }

    protected function keepAliveConnection(): void
    {
        if($this->keepAlive){
            $this->resetHttpRequest();
            return;
        }
        $this->setCloseOnBufferEmpty();
    }

    protected function keepAliveHeader($protocolVersion): array
    {
        if(isset($this->http['header']['connection']) && strtolower($this->http['header']['connection']) == 'keep-alive' && $protocolVersion == '1.1'){
            $this->keepAlive = true;
            return ['Connection' => 'keep-alive'];
        }
        return [];
    }

    protected function requestParser(string $rawHeader): ?array<string, string>
    {
        $pos = strpos($rawHeader, "\r\n");
        $line = substr($rawHeader, 0, $pos);
        $request = explode(' ', $line);

        if(
            count($request) != 3 ||
            !in_array($request[0], array('GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'HEAD', 'OPTIONS'))
        ){
            //bad request method
            $this->close();
        }

        $pos = strpos($request[1], '?');
        if($pos === false){
            $uri = $request[1];
            $query = [];
        }
        else{
            $uri = substr($request[1], 0, $pos);
            parse_str(substr($request[1], $pos + 1), $query);
        }
        return [
            'method' => $request[0],
            'uri' => $uri,
            'query' => $query,
            'protocol' => $request[2],
        ];
    }

    protected function headerParser(string $rawHeader): ?array<string, string>
    {
        $header = null;
        $headerLines = explode("\r\n", $rawHeader);
        foreach($headerLines as $line){
            $pos = strpos($line, ':');
            if($pos === false) {
                continue;
            }
            $header[strtolower(trim(substr($line, 0, $pos)))] = trim(substr($line, $pos + 2));

        }
        return $header;
    }

    protected function initMessage(string $data): void
    {
        if($this->http['header'] === null){
            $this->http['rawheader'] .= $data;
            $pos = strpos($this->http['rawheader'], "\r\n\r\n");
            if($pos === false){
                if(strlen($this->http['rawheader']) > self::MAX_HEADER_SIZE){
                    $this->close();
                }
                return false;
            }
            else{
                $this->http['body'] .= substr($this->http['rawheader'], $pos + 4);
                $this->http['rawheader'] = substr($this->http['rawheader'], 0, $pos);
            }
            $this->http['request'] = $this->requestParser($this->http['rawheader']);
            $this->http['header'] = $this->headerParser($this->http['rawheader']);
            unset($this->http['rawheader']);
            return;
        }
        $this->http['body'] .= $data;
    }
    
    protected function isMessageInitialized():bool
    {
        if($this->http['header'] === null){
            return false;
        }
        if(isset($this->http['header']['content-length'])){
            if(strlen($this->http['body']) < $this->http['header']['content-length']){
                return false;
            }
            if(strtolower($this->http['header']['content-type']) == 'application/x-www-form-urlencoded'){
                parse_str($this->http['body'], $this->http['request']['request']);
                unset($this->http['body']);
                unset($this->http['header']['content-type']);
                unset($this->http['header']['content-length']);
            }
        }
        return true;
    }

    protected function initCallback(): void
    {
        $this->onRead = function(UVTcp $client, string $data)
        {
            if($this->onCustomRead !== null){
                ($this->onCustomRead)($this, $data);
                return;
            }
            $this->initMessage($data);
            if($this->isMessageInitialized()){
                ($this->onRequest)($this);
            }
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

    public function setCloseOnBufferEmpty(bool $v = true):void
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
