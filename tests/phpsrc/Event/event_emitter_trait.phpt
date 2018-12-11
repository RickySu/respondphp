--TEST--
Check for Respond\Event\EventEmitterTrait
--FILE--
<?php
$reflect = new ReflectionClass(Respond\Event\EventEmitterTrait::class);
var_dump($reflect->isTrait());
--EXPECT--
bool(true)