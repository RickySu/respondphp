<?php
namespace Respond\Pack\Convert;

interface ConverterInterface
{
    public function __construct(ConverterInterface $converter = null);

    public function convert(string $content);
}