--TEST--
Check for Respond\Async\Promise
--FILE--
<?php
$reflect = new ReflectionClass(Respond\Async\Promise::class);
var_dump($reflect->implementsInterface(Respond\Async\PromiseInterface::class));
--EXPECT--
bool(true)