<?php
namespace Respond\Tests\Event;


use PHPUnit\Framework\TestCase;
use Respond\Event\EventEmitter;
use Respond\Event\EventEmitterInterface;

class EventEmitterTest extends TestCase
{
    public function test_interface()
    {
        //arrange
        $emitter = new EventEmitter();

        //act
        //assert
        $this->assertInstanceOf(EventEmitterInterface::class, $emitter);
    }

    public function test_on()
    {
        //arrange
        $emitter = new EventEmitter();
        $callback = function(){};
        $callback2 = function(){};
        $callback3 = function(){};
        //act
        $emitter->on('debug', $callback);
        $emitter->on('debug', $callback2);
        $emitter->on('debug2', $callback3);

        //assert
        $this->assertSame(array(
            'debug' => array(
                $callback,
                $callback2,
            ),
            'debug2' => array(
                $callback3
            ),
        ), $this->getObjectAttribute($emitter, 'listeners'));
    }

    public function test_off()
    {
        //arrange
        $emitter = new EventEmitter();
        $callback = function(){};
        $callback2 = function(){};
        $callback3 = function(){};
        $emitter->on('debug', $callback);
        $emitter->on('debug', $callback2);
        $emitter->on('debug2', $callback3);

        //act
        $emitter->off('debug', $callback);

        //assert
        $this->assertSame(array(
            'debug' => array(
                1 => $callback2,
            ),
            'debug2' => array(
                $callback3
            ),
        ), $this->getObjectAttribute($emitter, 'listeners'));
    }

    public function test_alloff()
    {
        //arrange
        $emitter = new EventEmitter();
        $callback = function(){};
        $callback2 = function(){};
        $callback3 = function(){};
        $emitter->on('debug', $callback);
        $emitter->on('debug', $callback2);
        $emitter->on('debug2', $callback3);

        //act
        $emitter->off('debug', $callback);
        $emitter->off('debug', $callback2);

        //assert
        $this->assertSame(array(
            'debug2' => array(
                $callback3
            ),
        ), $this->getObjectAttribute($emitter, 'listeners'));
    }

    public function test_removeListeners()
    {
        //arrange
        $emitter = new EventEmitter();
        $callback = function(){};
        $callback2 = function(){};
        $callback3 = function(){};
        $emitter->on('debug', $callback);
        $emitter->on('debug', $callback2);
        $emitter->on('debug2', $callback3);

        //act
        $emitter->removeListeners('debug');

        //assert
        $this->assertSame(array(
            'debug2' => array(
                $callback3
            ),
        ), $this->getObjectAttribute($emitter, 'listeners'));
    }

    public function test_getListeners()
    {
        //arrange
        $emitter = new EventEmitter();
        $callback = function(){};
        $callback2 = function(){};
        $callback3 = function(){};
        $emitter->on('debug', $callback);
        $emitter->on('debug', $callback2);
        $emitter->on('debug2', $callback3);

        //act
        $listeners = $emitter->getListeners('debug');

        //assert
        $this->assertSame(array(
            $callback,
            $callback2,
        ), $listeners);
    }

    public function test_emit()
    {
        //arrange
        $results = array();
        $emitter = new EventEmitter();
        $callback = function($foo, $bar) use(&$results){
            $results[] = array($foo, $bar);
        };
        $callback2 = function($foo, $bar) use(&$results){
            $results[] = array("cb2 $foo", "cb2 $bar");
        };
        $callback3 = function(){};

        $emitter->on('debug', $callback);
        $emitter->on('debug', $callback2);
        $emitter->on('debug2', $callback3);

        //act
        $emitter->emit('debug', 'foo', 'bar');

        //assert
        $this->assertEquals(array(
            array('foo', 'bar'),
            array('cb2 foo', 'cb2 bar'),
        ), $results);
    }
}
