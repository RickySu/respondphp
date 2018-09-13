<?php
namespace Respond\Async;

function async(callable $value)
{
    $asyncToGenerator = function ($asyncToGenerator, \Generator $generator) {
        while (true) {
            if (!$generator->valid()) {
                return;
            }

            $await = $generator->current();

            if ($await instanceof PromiseInterface) {
                break;
            }

            $generator->send($await);
        }

        $await->then(function ($value) use ($generator, $asyncToGenerator) {
            $generator->send($value);
            $asyncToGenerator($asyncToGenerator, $generator);
        });
    };

    return function (...$args) use ($value, $asyncToGenerator) {
        $asyncToGenerator($asyncToGenerator, $value(...$args));
    };
}
