<?php
namespace Respond\Pack\Command;

use function Respond\Async\async as aa;
use function Respond\Async\async as bb;
use Respond\Pack\Analyze\ClassnameAnalyze;
use Respond\Pack\Analyze\NamespaceAnalyze;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

class DumpCommand extends Command
{
    /** @var string */
    protected $outputFile;
    /** @var string */
    protected $inputFile;

    protected function configure()
    {
        $this
            ->setName('dump')
            ->setDescription('Dump respond runtime for defined constant')
            ->addOption('input', 'i', InputOption::VALUE_OPTIONAL, 'set packed file input path', realpath(__DIR__.'/../..').'/dist/packed.php')
            ->addOption('output', 'o', InputOption::VALUE_OPTIONAL, 'set dumped file output path', realpath(__DIR__.'/../..').'/dist/dumped.h')
        ;
    }

    public function execute(InputInterface $input, OutputInterface $output)
    {
        $outputContent = array();
        $this->outputFile = $input->getOption('output');
        $this->inputFile = $input->getOption('input');
        $content = file_get_contents($this->inputFile);
        $analyze = new NamespaceAnalyze();
        foreach($analyze->analyze($content) as $block){
            $outputContent = $this->appendNamespaceBlock($outputContent, $block);
            $outputContent = $this->appendClassnameBlock($outputContent, $block);
        }
        $output->writeln("dump from <info>{$this->inputFile}</info> to <info>{$this->outputFile}</info>");
        $this->dumpContent($outputContent);
    }

    protected function dumpContent($contentBLocks)
    {
        $output = '';
        file_put_contents($this->outputFile, null);
        foreach ($contentBLocks as $namespace => $block){
            $output .= sprintf('namespace %s{%s%s}', $namespace, $block['use'], $block['content']);
            foreach ($block['entries'] as $entry) {
                $fullname = $namespace.'\\'.$entry;
                file_put_contents($this->outputFile, sprintf("#define PREDEFINED_PHP_%s %s\n", str_replace('\\', '_', $fullname), json_encode($fullname)), FILE_APPEND);
            }
        }

        file_put_contents($this->outputFile, '#define PREDEFINED_PHP '.json_encode($output, JSON_UNESCAPED_SLASHES)."\n", FILE_APPEND);
    }

    protected function appendNamespaceBlock(array $content, array $block):array
    {
        if(!isset($content[$block['namespace']])){
            $content[$block['namespace']] = array(
                'content' => '',
                'use' => implode('', $block['use']),
                'entries' => array(),
            );
        }
        $content[$block['namespace']]['content'] .= $block['content'];
        return $content;
    }

    protected function appendClassnameBlock(array $content, array $block):array
    {
        $analyze = new ClassnameAnalyze();
        foreach($analyze->analyze($block['content']) as $class){
            $content[$block['namespace']]['entries'][] = $class;
        }
        return $content;
    }
}
