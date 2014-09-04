<?hh
class UVHttpParser
{
    private ?resource $_rs = null;
    private bool $headerComplete = false;
    private bool $messageComplete = false;
    
    private ?array<string> $headerFields = [];
    private ?array<string> $headerValues = [];
    public ?array<string> $headers = [];
    public ?array $url;
    public ?string $method;
    public bool $keepAlive = false;
    
    const PARSE_TYPE_REQUEST = 0;
    const PARSE_TYPE_RESPONSE = 1;
    const PARSE_TYPE_BOTH = 2;
    
    <<__Native>>public function __construct(int $type):void;
    <<__Native>>public function execute(string $data):int;
    
    private function onHeaderComplete():void{
    }
    
    public function isHeaderComplete():bool{
        return $this->headerComplete;
    }
    
    public function isMessageComplete():bool{
        return $this->messageComplete;
    }
}
