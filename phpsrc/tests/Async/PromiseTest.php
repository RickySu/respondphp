<?php
namespace Respond\Tests\Promise;

use Respond\Async\Promise;
use Respond\Async\PromiseInterface;

class PromiseTest extends \PHPUnit_Framework_TestCase
{
    private $resolve;
    private $reject;
    private $result;

    /**
     * @expectedException \RuntimeException
     * @expectedExceptionMessage Cannot resolve a resolved promise!
     */
    public function test_resolve_twice()
    {
        //arrange
        $index = 0;
        $errIndex = 0;
        $promise = new Promise(function($resolve, $reject){
            $resolve('bar');
        });
        $promise
            ->then(function($result) use(&$index){
                $index++;
                $this->assertEquals('bar', $result);
            })
            ->catch(function($err) use(&$errIndex){
                $this->assertInstanceOf(\RuntimeException::class, $err);
            })
        ;

        //act
        $promise->resolve('foo');

        //assert
        $this->assertEquals(1, $index);
        $this->assertEquals(1, $errIndex);
    }

    public function test_wrap()
    {
        //arrange
        $foo = 'foo';

        //act
        $result = Promise::wrap($foo);

        //assert
        $this->assertInstanceOf(PromiseInterface::class, $result);
        $result
            ->then(function($value) use($foo){
                $this->assertEquals($foo, $value);
            })
        ;
    }

    public function test_fullfill_sync()
    {
        //arrange
        $this->result = [];
        //act
        $promise = new Promise(function($resolve, $reject){
            $resolve('bar');
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
        ;

        //assert
        $this->assertEquals(array(
            'bar',
            'foo bar',
            'foo foo bar',
        ), $this->result);
    }

    public function test_fullfill_async()
    {
        //arrange
        $this->result = [];

        //act
        $promise = new Promise(function($resolve, $reject){
            $this->resolve = $resolve;
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
        ;
        call_user_func($this->resolve, 'bar');

        //assert
        $this->assertEquals(array(
            'bar',
            'foo bar',
            'foo foo bar',
        ), $this->result);
    }

    public function test_catch_sync()
    {
        //arrange
        $this->result = [];
        $err = new \Exception('bar');

        //act
        $promise = new Promise(function($resolve, $reject) use($err){
            $reject($err);
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->catch(function($result){
                $this->result[] = $result->getMessage();
                return "foo {$result->getMessage()}";
            })
        ;

        //assert
        $this->assertEquals(array(
            'bar'
        ), $this->result);
    }

    public function test_catch_async()
    {
        //arrange
        $this->result = [];
        $err = new \Exception('bar');

        //act
        $promise = new Promise(function($resolve, $reject){
            $this->reject = $reject;
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->catch(function ($result){
                $this->result[] = $result;
            });
        ;
        call_user_func($this->reject, $err);

        //assert
        $this->assertEquals(array(
            $err,
        ), $this->result);
    }

    public function test_final_fullfill_sync()
    {
        //arrange
        $this->result = [];
        //act
        $promise = new Promise(function($resolve, $reject){
            $resolve('bar');
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->catch(function ($result){
                $this->result[] = $result;
                return $result;
            })
            ->finally(function($result){
                $this->result[] = $result;
                return $result;
            })
        ;

        //assert
        $this->assertEquals(array(
            'bar',
            'foo bar',
            'foo foo bar',
            'foo foo foo bar',
        ), $this->result);
    }

    public function test_final_catcuh_sync()
    {
        //arrange
        $this->result = [];
        $err = new \Exception('bar');

        //act
        $promise = new Promise(function($resolve, $reject) use($err){
            $reject($err);
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->catch(function ($result){
                $this->result[] = $result->getMessage();
                return $result->getMessage();
            })
            ->finally(function($result){
                $this->result[] = $result;
                return $result;
            })
        ;

        //assert
        $this->assertEquals(array(
            'bar',
            $err,
        ), $this->result);
    }

    public function test_final_fullfill_async()
    {
        //arrange
        $this->result = [];
        //act
        $promise = new Promise(function($resolve, $reject){
            $this->resolve = $resolve;
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->catch(function ($result){
                $this->result[] = $result;
                return "$result buz";
            })
            ->finally(function($result){
                $this->result[] = $result;
                return $result;
            })
        ;
        call_user_func($this->resolve, 'bar');

        //assert
        $this->assertEquals(array(
            'bar',
            'foo bar',
            'foo foo bar',
            'foo foo foo bar',
        ), $this->result);
    }

    public function test_final_catcuh_async()
    {
        //arrange
        $this->result = [];
        $err = new \Exception('bar');

        //act
        $promise = new Promise(function($resolve, $reject){
            $this->reject = $reject;
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->then(function($result){
                $this->result[] = $result;
                return "foo $result";
            })
            ->catch(function ($result){
                $this->result[] = $result->getMessage();
            })
            ->finally(function($result){
                $this->result[] = $result;
                return $result;
            })
        ;
        call_user_func($this->reject, $err);

        //assert
        $this->assertEquals(array(
            'bar',
            $err,
        ), $this->result);
    }

    public function test_fullfill_sync_promise()
    {
        //arrange
        $this->result = [];
        //act
        $promise = new Promise(function($resolve, $reject){
            $resolve('bar');
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return new Promise(function($resolve, $reject) use($result){
                    $resolve("foo $result");
                });
            })
            ->then(function($result){
                $this->result[] = $result;
                return new Promise(function($resolve, $reject) use($result){
                    $resolve("foo $result");
                });
            })
            ->then(function($result){
                $this->result[] = $result;
                return new Promise(function($resolve, $reject) use($result){
                    $resolve("foo $result");
                });
            })
        ;

        //assert
        $this->assertEquals(array(
            'bar',
            'foo bar',
            'foo foo bar',
        ), $this->result);
    }


    public function test_fullfill_async_promise()
    {
        //arrange
        $this->result = [];

        //act
        $promise = new Promise(function($resolve, $reject){
            $this->resolve = $resolve;
        });
        $promise
            ->then(function($result){
                $this->result[] = $result;
                return new Promise(function($resolve, $reject) use($result){
                    $resolve("foo $result");
                });

            })
            ->then(function($result){
                $this->result[] = $result;
                return new Promise(function($resolve, $reject) use($result){
                    $resolve("foo $result");
                });
            })
            ->then(function($result){
                $this->result[] = $result;
                return new Promise(function($resolve, $reject) use($result){
                    $resolve("foo $result");
                });
            })
        ;
        call_user_func($this->resolve, 'bar');

        //assert
        $this->assertEquals(array(
            'bar',
            'foo bar',
            'foo foo bar',
        ), $this->result);
    }

    public function test_all_then()
    {
        //arrange
        $result = null;
        $resolves = [];
        $rejects = [];
        $p0 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $p1 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $p2 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $promises = Promise::all([$p0, $p1, $p2]);

        //act
        $resolves[1](1);
        $promises->then(function($value) use(&$result){
            $result = $value;
        });
        $resolves[0](0);
        $resolves[2](2);

        //assert
        $this->assertEquals(array(0, 1, 2), $result);
    }

    public function test_all_reject()
    {
        //arrange
        $result = null;
        $resolves = [];
        $rejects = [];
        $err = new \Exception("foo");
        $p0 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $p1 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $p2 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $promises = Promise::all([$p0, $p1, $p2]);

        //act
        $rejects[1]($err);
        $promises
            ->then(function($value) use(&$result){
                $this->fail("never happen");
            })
            ->catch(function($reason) use(&$result){
                $result = $reason;
            })
        ;

        $resolves[2](2);
        $resolves[0](0);

        //assert
        $this->assertSame($err, $result);
    }

    public function test_race_then()
    {
        //arrange
        $result = null;
        $resolves = [];
        $rejects = [];
        $p0 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $p1 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $p2 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $promises = Promise::race([$p0, $p1, $p2]);

        //act
        $promises->then(function($value) use(&$result){
            $result = $value;
        });
        $resolves[2](2);

        //assert
        $this->assertEquals(2, $result);
    }

    public function test_race_reject()
    {
        //arrange
        $result = null;
        $resolves = [];
        $rejects = [];
        $err = new \Exception("foo");
        $p0 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $p1 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $p2 = new Promise(function($resolve, $reject) use(&$resolves, &$rejects) {
            $resolves[] = $resolve;
            $rejects[] = $reject;
        });
        $promises = Promise::race([$p0, $p1, $p2]);

        //act
        $promises
            ->then(function($value) use(&$result){
                $result = $value;
            })
            ->catch(function($err) use(&$result){
                $result = $err;
            })
        ;
        $rejects[2]($err);

        //assert
        $this->assertSame($err, $result);
    }
}
