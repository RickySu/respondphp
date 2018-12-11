--TEST--
Check for Respond\Network\Resolver
--FILE--
<?php
$promise = Respond\Network\Resolver::getaddrinfo('a.root-servers.net');
$promise
    ->then(function($result){
        print_r($result);
    });
$promise = Respond\Network\Resolver::getnameinfo('198.41.0.4');
$promise
    ->then(function($result){
        echo "$result\n";
    });
$promise = Respond\Network\Resolver::getnameinfo('2001:503:ba3e::2:30');
$promise
    ->then(function($result){
        echo "$result\n";
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
