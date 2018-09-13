#!/usr/bin/env php
<?php
namespace Respond\Pack;

if (PHP_SAPI !== 'cli') {
    echo "must run under cli mode!";
    exit(-1);
}

if (file_exists(__DIR__ . '/../vendor/autoload.php')) {
    // Called from local git clone.
    require __DIR__ . '/../vendor/autoload.php';
} elseif (file_exists(__DIR__ . '/../../../autoload.php')) {
    // Called from your project's vendor dir.
    require __DIR__ . '/../../../autoload.php';
} else {
    echo 'You need to set up the project dependencies using the following commands:' . PHP_EOL .
        'curl -s http://getcomposer.org/installer | php' . PHP_EOL .
        'php composer.phar install --dev' . PHP_EOL;
    exit(-1);
}

$application = new \Symfony\Component\Console\Application();
$application->add(new \Respond\Pack\Command\PackCommand());
$application->add(new \Respond\Pack\Command\DumpCommand());
$application->run();