<?php
namespace Respond\Pack\Dumper;

class PhpPacker
{
    /** @var string */
    protected $outputFile;

    public function __construct(string $outputFile)
    {
        $this->outputFile = $outputFile;
        $this->replaceContent();
    }

    public function appendContent(string $content = null, $mode = FILE_APPEND)
    {
        return file_put_contents($this->outputFile, $content, $mode);
    }

    public function replaceContent(string $content = null)
    {
        $this->appendContent($content, FILE_TEXT);
    }

    public function addFile(\SplFileInfo $file)
    {
        $this->appendContent(file_get_contents($file->getPathname()));
    }
}
