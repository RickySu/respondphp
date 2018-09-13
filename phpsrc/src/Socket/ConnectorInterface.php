<?php
namespace Respond\Socket;

use Respond\Async\PromiseInterface;

interface ConnectorInterface
{
    public function connect($uri):PromiseInterface;
}
