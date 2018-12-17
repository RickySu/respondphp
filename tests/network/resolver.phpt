--TEST--
Check for Respond\Network\Resolver
--FILE--
<?php
$promise0 = Respond\Network\Resolver::getaddrinfo('a.root-servers.net');
$promise1 = Respond\Network\Resolver::getnameinfo('198.41.0.4');
$promise2 = Respond\Network\Resolver::getnameinfo('2001:503:ba3e::2:30');
$promises = Respond\Async\Promise::all([$promise0, $promise1, $promise2]);
$promises
    ->then(function($result){
        print_r($result[0]);
        echo $result[1]."\n";
        echo $result[2]."\n";
    });
Respond\Event\Loop::create()->run();

--EXPECT--
Array
(
    [IPv4] => Array
        (
            [0] => 198.41.0.4
        )

    [IPv6] => Array
        (
            [0] => 2001:503:ba3e::2:30
        )

)
a.root-servers.net
a.root-servers.net
