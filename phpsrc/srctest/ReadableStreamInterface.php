<?php
namespace Respond\Stream;

interface ReadableStreamInterface
{
    public function isWritable():bool;
    public function write($data):bool;
    public function end($data = null):void ;
    public function close():void;
}
