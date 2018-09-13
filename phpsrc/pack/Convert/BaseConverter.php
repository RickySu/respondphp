<?php
namespace Respond\Pack\Convert;


abstract class BaseConverter implements ConverterInterface
{
    /** @var ConverterInterface  */
    protected $prevConverter;

    abstract protected function convertData(string $content);

    public function __construct(ConverterInterface $converter = null)
    {
        $this->prevConverter = $converter;
    }

    public function convert(string $content)
    {
        if($this->prevConverter){
            $content = $this->prevConverter->convert($content);
        }

        return $this->convertData($content);
    }

}
