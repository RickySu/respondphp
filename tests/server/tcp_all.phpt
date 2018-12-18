--TEST--
Check for Respond\Server\Tcp for IP v4 and v6
--FILE--
<?php
$randomPort = rand(50000, 60000);
$pid = pcntl_fork();
if($pid) {
    usleep(100000);
    $fp = stream_socket_client("tcp://127.0.0.1:$randomPort");
    fwrite($fp, "hello world 1!");
    $result = fread($fp, 1024);
    fclose($fp);
    echo "result: $result\n";
    $fp = stream_socket_client("tcp://[::1]:$randomPort");
    fwrite($fp, "hello world 2!");
    $result = fread($fp, 1024);
    fclose($fp);
    echo "result: $result";
    posix_kill($pid, SIGINT);
    pcntl_wait($pid);
    exit;
}

$server = new Respond\Server\Tcp($randomPort);
$server->on('connect', function(Respond\Stream\Connection $connection){
    $connection->on('data', function(Respond\Stream\Connection $connection, $data){
        $connection->end($data);
    });
});
Respond\Event\Loop::create()->run();
--EXPECT--
result: hello world 1!
result: hello world 2!