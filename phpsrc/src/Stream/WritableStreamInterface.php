<?php
namespace Respond\Stream;

interface WritableStreamInterface
{
    public function isReadable():bool;
    public function close():void;
}
