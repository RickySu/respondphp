--TEST--
Check for Respond\Server\Tcp for IP v6
--FILE--
<?php
$pid = pcntl_fork();
if($pid) {
    sleep(1);
    $fp = fsockopen('[::1]', 8080, $err, $errStr);
    fwrite($fp, "hello world!");
    $result = fread($fp, 1024);
    fclose($fp);
    echo "result: $result";
    posix_kill($pid, SIGINT);
    pcntl_wait($pid);
    exit;
}

$server = new Respond\Server\Tcp(8080, '::1');
$server->on('connect', function(Respond\Stream\Connection $connection){
    $connection->on('data', function(Respond\Stream\Connection $connection, $data){
        $connection->end($data);
    });
});
Respond\Event\Loop::create()->run();
--EXPECT--
result: hello world!