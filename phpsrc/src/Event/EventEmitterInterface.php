<?php
namespace Respond\Event;

interface EventEmitterInterface
{
    public function on(string $event, callable $listener):void;
    public function off(string $event, callable $listener):void;
    public function removeListeners(string $event):void;
    public function getListeners(string $event):array;
    public function emit(string $event, ...$arguments):void;
}
