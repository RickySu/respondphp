--TEST--
Check for Respond\Server\Routine run from actor
--FILE--
<?php
function fibonacci($n)
{
    if($n < 2){
        return $n;
    }
    return fibonacci($n - 1) + fibonacci($n - 2);
}

$routine = new Respond\Server\Routine(function ($n){
    return fibonacci($n);
});

$promise1 = $routine->execute(10);
$promise2 = $routine->execute(5);
Respond\Async\Promise::all([$promise1, $promise2])
    ->then(function($resolve) {
        echo "{$resolve[0]}, {$resolve[1]}";
        Respond\Event\Loop::create()->end();
    })
    ->catch(function($error){
        print_r($error);
    })
    ;
Respond\Event\Loop::create()->run();

--EXPECT--
55, 5