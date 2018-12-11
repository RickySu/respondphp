--TEST--
Check for Respond\System\Timer
--FILE--
<?php
$promise = Respond\System\Timer::timeout(3000);
$promise
    ->then(function(){
        echo "time out 0\n";
    })
    ->catch(function($error){
        print_r($error);
    })
;

$promise1 = Respond\System\Timer::timeout(2000);
$promise1
    ->then(function(){
        echo "time out 1\n";
    })
    ->catch(function($error){
        print_r($error);
    })
;

$promise2 = Respond\System\Timer::timeout(1000);
$promise2
    ->then(function() use($promise){
        echo "time out 2\n";
        $promise->cancel();
    })
    ->catch(function($error){
        print_r($error);
    })
;
Respond\Event\Loop::create()->run();

--EXPECT--
time out 2
time out 1