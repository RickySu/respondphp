<?php
namespace Respond\Event;

interface EventEmitterInterface
{
    public function on(string $event, callable $listener);
    public function off(string $event, callable $listener);
    public function removeListeners(string $event);
    public function getListeners(string $event):?array;
    public function emit(string $event, ...$arguments);
}
