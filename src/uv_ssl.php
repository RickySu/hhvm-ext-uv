<?hh
class UVSSL extends UVTcp
{   
    protected ?string $certFile;
    protected ?string $certChainFile;
    protected ?string $privateKeyFile;
     
    <<__Native>> function __destruct():void;
    <<__Native>> function listen(string $host, int $port, mixed $onConnectCallback):int;
    <<__Native>> function accept(): UVSSL;
    
    public function setCertFile(string $certFile):void
    {
        $this->certFile = $certFile;
    }
    
    public function setCertChainFile(string $certChainFile):void
    {
        $this->certChainFile = $certChainFile;
    }
    
    public function setPrivateKeyFile(string $privateKeyFile):void
    {
        $this->privateKeyFile = $privateKeyFile;
    }

}
