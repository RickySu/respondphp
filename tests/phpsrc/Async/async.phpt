--TEST--
Check for function Respond\Async\async
--FILE--
<?php
var_dump(function_exists('Respond\\Async\\async'));
--EXPECT--
bool(true)