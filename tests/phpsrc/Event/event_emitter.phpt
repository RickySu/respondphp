--TEST--
Check for Respond\Event\EventEmitter
--FILE--
<?php
$reflect = new ReflectionClass(Respond\Event\EventEmitter::class);
var_dump($reflect->implementsInterface(Respond\Event\EventEmitterInterface::class));
--EXPECT--
bool(true)