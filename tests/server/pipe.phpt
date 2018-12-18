--TEST--
Check for Respond\Server\Pipe
--FILE--
<?php
$socketPath = sys_get_temp_dir()."/".uniqid('test_sock').".sock";
$pid = pcntl_fork();
if($pid) {
    usleep(100000);
    $fp = stream_socket_client("unix://$socketPath");
    fwrite($fp, "hello world!");
    $result = fread($fp, 1024);
    fclose($fp);
    echo "result: $result";
    posix_kill($pid, SIGINT);
    pcntl_wait($pid);
    exit;
}

$server = new Respond\Server\Pipe($socketPath);
$server->on('connect', function(Respond\Stream\Connection $connection){
    $connection->on('data', function(Respond\Stream\Connection $connection, $data){
        $connection->end($data);
    });
});
Respond\Event\Loop::create()->run();
--EXPECT--
result: hello world!