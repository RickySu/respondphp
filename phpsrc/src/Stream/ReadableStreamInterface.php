<?php
namespace Respond\Stream;

interface ReadableStreamInterface
{
    public function isReadable():bool;
    public function end($data = null):void ;
    public function close():void;
}
