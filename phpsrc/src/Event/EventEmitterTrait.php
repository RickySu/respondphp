<?php
namespace Respond\Event;

trait EventEmitterTrait
{
    protected $listeners = array();

    public function on(string $event, callable $listener):void
    {
        if(!isset($this->listeners[$event])){
            $this->listeners[$event] = array();
        }
        $this->listeners[$event][] = $listener;
    }

    public function off(string $event, callable $listener):void
    {
        if(!isset($this->listeners[$event])){
            return;
        }

        if(($index = array_search($listener, $this->listeners[$event], true)) !== false){
            unset($this->listeners[$event][$index]);
        }

        if(count($this->listeners[$event]) == 0){
            unset($this->listeners[$event]);
        }
    }

    public function removeListeners(string $event):void
    {
        unset($this->listeners[$event]);
    }

    public function getListeners(string $event): array
    {
        return $this->listeners[$event]??array();
    }

    public function emit(string $event, ...$arguments):void
    {
        if(!isset($this->listeners[$event])){
            return;
        }

        foreach ($this->listeners[$event] as $listener){
            $listener(...$arguments);
        }
    }
}
