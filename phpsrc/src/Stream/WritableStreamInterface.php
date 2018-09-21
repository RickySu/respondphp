<?php
namespace Respond\Stream;

interface WritableStreamInterface
{
    public function isWritable():bool;
    public function write($data):bool;
    public function close():void;
}
