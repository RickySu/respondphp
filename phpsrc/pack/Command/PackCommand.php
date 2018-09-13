<?php
namespace Respond\Pack\Command;

use Respond\Pack\Convert\ConverterInterface;
use Respond\Pack\Convert\NamespaceConverter;
use Respond\Pack\Convert\OptimizeConverter;
use Respond\Pack\Convert\PHPEndTagConverter;
use Respond\Pack\Convert\PHPTagRemoveConverter;
use Respond\Pack\Dumper\PhpPacker;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Filesystem\Filesystem;
use Symfony\Component\Finder\Finder;

class PackCommand extends Command
{
    /** @var string */
    protected $outputFile;

    protected function configure()
    {
        $this
            ->setName('pack')
            ->setDescription('Pack respond runtime.')
            ->addOption('source', 's', InputOption::VALUE_OPTIONAL, 'set source path', realpath(__DIR__.'/../../src'))
            ->addOption('output', 'o', InputOption::VALUE_OPTIONAL, 'set packed file output path', realpath(__DIR__.'/../..').'/dist/packed.php')
        ;
    }

    public function execute(InputInterface $input, OutputInterface $output)
    {
        $this->outputFile = $input->getOption('output');
        $sourcePath = $input->getOption('source');
        $output->writeln("create folder <info>{$this->makeOutputPath($this->outputFile)}</info>");
        $files = $this->getPhpFiles($sourcePath);
        $packer = new PhpPacker($this->outputFile);

        /** @var \SplFileInfo $file */
        foreach ($files as $file) {
            $output->writeln("pack <info>{$file->getPathname()}</info>");
            $content = file_get_contents($file->getPathname());
            $packer->appendContent($this->getSingleOptimizeConvert()->convert($content));
        }

        $converter = new OptimizeConverter(
            new PHPTagRemoveConverter()
        );

        $packer->replaceContent($converter->convert(file_get_contents($this->outputFile)));
    }

    protected function getSingleOptimizeConvert(): ConverterInterface
    {
        return new PHPEndTagConverter(
            new NamespaceConverter()
        );
    }

    protected function getPhpFiles(string $sourcePath): \Traversable
    {
        $finder = new Finder();
        return $finder
            ->name('/\.php$/i')
            ->in($sourcePath)
            ->files()
        ;
    }

    protected function makeOutputPath(string $outputFile): string
    {
        $filename = basename($outputFile);
        $dir = substr($outputFile, 0,  -strlen($filename));
        $filesystem = new Filesystem();
        $filesystem->mkdir($dir, 0755);
        return realpath($dir);
    }
}
