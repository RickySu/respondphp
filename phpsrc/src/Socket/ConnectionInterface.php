<?php
namespace Respond\Socket;

use Respond\Event\EventEmitterInterface;
use Respond\Stream\ReadableStreamInterface;
use Respond\Stream\WritableStreamInterface;

interface ConnectionInterface extends ReadableStreamInterface, WritableStreamInterface, EventEmitterInterface
{
    public function getRemoteAddress():?string;
    public function getLocalAddress():?string;
}
