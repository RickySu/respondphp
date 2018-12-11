--TEST--
Check for Respond\Async\CancelablePromise
--FILE--
<?php
$reflect = new ReflectionClass(Respond\Async\CancelablePromise::class);
var_dump($reflect->hasMethod('cancel'));
var_dump($reflect->implementsInterface(Respond\Async\PromiseInterface::class));
--EXPECT--
bool(true)
bool(true)