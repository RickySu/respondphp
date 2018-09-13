<?php
namespace Respond\Async;

interface PromiseInterface
{
    const PENDING = 'pending';
    const FULFILLED = 'fullfilled';
    const REJECTED = 'rejected';

    public function __construct(callable $executor);
    public function then(callable $onFullfiled = null, callable $onRejcted = null): ?PromiseInterface;
    public function catch(callable $onRejcted): ?PromiseInterface;
    public function finally(callable $onFinal): ?PromiseInterface;
    public function resolve($value);
    public function reject(\Throwable $reason);
    public static function all(array $promises): ?PromiseInterface;
    public static function race(array $promises): ?PromiseInterface;
}
