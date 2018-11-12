<?php
namespace Respond\Async;

class Promise implements PromiseInterface
{
    protected $onFullfilled;
    protected $onRejected;
    protected $onFinaled;
    protected $nextPromise;
    protected $value;
    protected $canceller;
    protected $settleState = self::PENDING;

    public static function wrap($value)
    {
        return new static(function ($resolve, $reject) use ($value) {
            $resolve($value);
        });
    }

    public function __construct(callable $executor = null, callable $canceller = null)
    {
        if ($executor === null) {
            return;
        }
        $this->execute($executor);
        $this->canceller = $canceller;
    }

    public function cancel()
    {
        if($this->canceller){
            call_user_func($this->canceller);
        }
    }

    public function then(callable $onFullfilled = null, callable $onRejected = null, callable $onFinaled = null): PromiseInterface
    {
        if($onFullfilled !== null){
            $this->onFullfilled = $onFullfilled;
        }
        if($onRejected !== $onRejected){
            $this->onRejected = $onRejected;
        }
        if($onFinaled !== null){
            $this->onFinaled = $onFinaled;
        }

        $this->nextPromise =  new static(function ($resolve, $reject) {
            $this->resolve = $resolve;
            $this->reject = $reject;
        });

        if ($this->settleState === self::FULFILLED) {
            $this->settleFullfill();
        }

        if ($this->settleState === self::REJECTED) {
            $this->settleReject();
        }

        return $this->nextPromise;
    }

    public function catch(callable $onRejected): PromiseInterface
    {
        return $this->then(null, $onRejected);
    }

    public function finally(callable $onFinaled): PromiseInterface
    {
        return $this->then(null, null, $onFinaled);
    }

    public static function all(array $promises): PromiseInterface
    {
        return new self(function ($resolve, $reject) use ($promises) {
            $promisesCount = count($promises);
            $results = [];
            /** @var PromiseInterface $promise */
            foreach ($promises as $index => $promise) {
                $promise
                    ->then(function ($value) use (&$results, $index, &$promisesCount, $resolve) {
                        $results[$index] = $value;
                        $promisesCount--;
                        if ($promisesCount == 0) {
                            $resolve($results);
                        }
                    })
                    ->catch(function ($reason) use ($reject) {
                        $reject($reason);
                    })
                ;
            }
        });
    }

    public function resolve($value = null)
    {
        if ($this->settleState == self::REJECTED) {
            throw new \RuntimeException("Cannot resolve a rejected promise!");
        }

        if ($this->settleState == self::FULFILLED) {
            throw new \RuntimeException("Cannot resolve a resolved promise!");
        }

        $this->value = $value;
        $this->settleState = self::FULFILLED;
        $this->settleFullfill();
    }

    public function reject(\Throwable $reason)
    {
        if ($this->settleState == self::FULFILLED) {
            throw new \RuntimeException("Cannot reject a fullfilled promise!");
        }

        if ($this->settleState == self::REJECTED) {
            throw new \RuntimeException("Cannot reject a rejected promise!");
        }

        $this->value = $reason;
        $this->settleState = self::REJECTED;
        $this->settleReject();
        $this->value = null;
    }

    protected function settleFullfill()
    {
        $value = $this->value;

        if (is_callable($this->onFullfilled)) {
            $value = $this->invokeCall($this->onFullfilled, $value);
        }

        if (is_callable($this->onFinaled)) {
            $value = $this->invokeCall($this->onFinaled, $value);
        }

        if ($this->nextPromise) {
            $value = $this->invokeCall(array($this->nextPromise, 'resolve'), $value);
        }

        return $value;
    }

    protected function invokeCall(callable $function, $value)
    {
        if ($value instanceof PromiseInterface) {
            return $value->then(function ($value) use ($function) {
                $function($value);
            });
        }

        return $function($value);
    }

    protected function settleReject()
    {
        if (is_callable($this->onRejected)) {
            $this->invokeCall($this->onRejected, $this->value);
        }

        if (is_callable($this->onFinaled)) {
            $this->invokeCall($this->onFinaled, $this->value);
        }

        if ($this->nextPromise) {
            $this->invokeCall(array($this->nextPromise, 'reject'), $this->value);
        }

        return $this->value;
    }

    private function execute(callable $executor)
    {
        try {
            $resolve = function ($value = null) {
                $this->resolve($value);
            };

            $reject = function (\Throwable $reason) {
                $this->reject($reason);
            };

            $executor($resolve, $reject);
        } catch (\Throwable $e) {
            $this->reject($e);
        }
    }

    public static function race(array $promises): PromiseInterface
    {
        return new self(function ($resolve, $reject) use ($promises) {
            /** @var PromiseInterface $promise */
            foreach ($promises as $index => $promise) {
                $promise
                    ->then(function ($value) use ($resolve) {
                        $resolve($value);
                    })
                    ->catch(function ($reason) use ($reject) {
                        $reject($reason);
                    })
                ;
            }
        });
    }
}
