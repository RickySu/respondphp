--TEST--
Check for Respond\Server\Secure SAN Match
--FILE--
<?php
$host = '127.0.0.1';
$hostName = 'example.com';
$randomPort = rand(50000, 60000);
$pid = pcntl_fork();
if($pid) {
    usleep(500000);
    $streamContext = stream_context_create(array(
        'ssl' => array(
            'peer_name'         => $hostName,
            'verify_peer'       => false,
            'verify_peer_name'  => true,
            'allow_self_signed' => true,
        ),
    ));
    $fp = stream_socket_client("ssl://$host:$randomPort", $errno, $errstr, 30, STREAM_CLIENT_CONNECT, $streamContext);
    fwrite($fp, "hello world!");
    $result = fread($fp, 1024);
    fclose($fp);
    echo "result: $result";
    posix_kill($pid, SIGINT);
    pcntl_wait($pid);
    exit;
}

$secureServer = new Respond\Server\Secure(new Respond\Server\Tcp($randomPort), array(
    'ssl' => array(
        array(
            'hostname' => 'localhost.localdomain',
            'local_cert' => file_get_contents(__DIR__.'/../cert/server-localhost.localdomain.cert'),
            'local_pk' => file_get_contents(__DIR__.'/../cert/server-localhost.localdomain.key'),
            'passphrase' => '12345',
        ),
        array(
            'hostname' => 'localhost',
            'local_cert' => file_get_contents(__DIR__.'/../cert/server-localhost.cert'),
            'local_pk' => file_get_contents(__DIR__.'/../cert/server-localhost.key'),
            'passphrase' => '12345',
        ),
    )
));

$secureServer->on('connect', function(Respond\Stream\ConnectionInterface $connection){
    $connection->on('data', function(Respond\Stream\ConnectionInterface $connection, string $data){
        $connection->end($data);
    });
});

Respond\Event\Loop::create()->run();
--EXPECT--
result: hello world!