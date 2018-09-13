<?php
namespace Respond\Socket;

use Respond\Event\EventEmitterInterface;

interface ServerInterface extends EventEmitterInterface
{
    public function close():void ;
}
