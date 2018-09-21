<?php
namespace Respond\Stream;

interface ReadableStreamInterface
{
    public function isReadable():bool;
    public function close():void;
}
