--TEST--
Check for Respond\Server\Udp for IP v4
--FILE--
<?php
$port = rand(50000, 60000);
$host = '127.0.0.1';
$message = "hello world!";

function receive_udp_message($host, $port, $message, $callback) {
    $localPort = rand(30000, 40000);
    $socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
    @socket_bind($socket, $host, $localPort);
    @set_time_limit(0);
    socket_sendto($socket, $message, strlen($message), 0, $host, $port);
    while(true){
        usleep(1000);
        if(socket_recvfrom($socket, $data, 4096, 0, $remoteHost, $remotePort) !== false){
            if(!$callback($remoteHost, $remotePort, $data)){
                break;
            }
        }

    }
}

$pid = pcntl_fork();

if($pid) {
    usleep(500000);
    receive_udp_message($host, $port, $message, function($host, $port, $result) use($pid){
        echo "result: $result\n";
        if($result == 'udp send done!') {
            posix_kill($pid, SIGINT);
            return false;
        }
        return true;
    });

    pcntl_wait($pid);
    exit;
}

$server = new Respond\Server\Udp($port, $host);
$server->on('recv', function(Respond\Server\Udp $server, $fromHost, $fromPort, $msg){
    $promise = $server->send($fromHost, $fromPort, $msg);
    $promise
        ->then(function() use($server, $fromHost, $fromPort){
            return $server->send($fromHost, $fromPort, "udp send done!");
        });
});
Respond\Event\Loop::create()->run();
--EXPECT--
result: hello world!
result: udp send done!
