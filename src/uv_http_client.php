<?hh
class UVHttpClient
{
    const MAX_HEADER_SIZE = 8192;
    private int $dataSizeInBuffer = 0;
    private bool $closeOnEmpty = false;
    protected ?array $request;
    protected bool $isHttpInitialized = false;
    protected bool $keepAlive = false;
    protected ?UVTcp $client = null;
    protected ?callable $onRead = null;
    protected ?callable $onWrite = null;
    protected ?callable $onError = null;
    private ?UVHttpParser $parser;
    protected ?callable $onRequest = null;
    protected ?mixed $onCustomRead = null;
    protected ?mixed $onCustomWrite = null;
    protected ?mixed $onCustomError = null;

    protected function resetHttpRequest():void
    {
        $this->parser = new UVHttpParser(UVHttpParser::PARSE_TYPE_REQUEST);
        $this->request = null;
        $this->isHttpInitialized = false;
        $this->keepAlive = false;
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

    public function getRequest(): array<string, string>
    {
        return $this->request;
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
        if($this->keepAlive && $protocolVersion == '1.1'){
            return ['Connection' => 'keep-alive'];
        }
        return [];
    }

    protected function initializeHttpRequest()
    {
        $this->keepAlive = $this->parser->keepAlive;
        $this->request = [
            'request' => [
                'method' => $this->parser->method,
                'uri' => $this->parser->url,
                'protocol' => "{$this->parser->httpMajor}.{$this->parser->httpMinor}",
            ],
            'headers' => $this->parser->headers,
            'body' => $this->parser->body,
        ];
    }
    
    protected function initCallback(): void
    {
        $this->onRead = function(UVTcp $client, string $data)
        {
            if($this->onCustomRead !== null){
                ($this->onCustomRead)($this, $data);
                return;
            }
            $this->parser->execute($data);
            if($this->parser->isMessageComplete()){
                if(!$this->isHttpInitialized){
                    $this->isHttpInitialized = true;
                    $this->initializeHttpRequest();
                    ($this->onRequest)($this);
                }
                return;
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
