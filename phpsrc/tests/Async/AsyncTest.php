<?php
namespace Respond\Tests\Promise;

use function Respond\Async\async;
use Respond\Async\Promise;

class AsyncTest extends \PHPUnit_Framework_TestCase
{
    public function test_AsyncWait()
    {
        //arrange
        $resolvers = [];
        $results = [];
        $p0 = new Promise(function($resolve, $reject) use(&$resolvers){
            $resolvers[] = $resolve;
        });
        $p1 = new Promise(function($resolve, $reject) use(&$resolvers){
            $resolvers[] = $resolve;
        });
        $p2 = new Promise(function($resolve, $reject) use(&$resolvers){
            $resolvers[] = $resolve;
        });

        $f = async(function() use($p0, $p1, $p2, &$results){
            $a = 10;
            $results[] = yield $p0;
            $results[] = yield "foo";
            $results[] = $result = yield $p1;
            $a += $result;
            $results[] = yield $p2;
            $results[] = yield "bar";
            $results[] = $a;
        });

        //act
        $f();
        $resolvers[2](2);
        $resolvers[1](1);
        $resolvers[0](0);

        //assert
        $this->assertEquals(array(
            0,
            'foo',
            1,
            2,
            'bar',
            11,
        ), $results);
    }
}
