<?php
namespace Respond\Pack\Analyze;

class ClassnameAnalyze
{
    public function analyze(string $content): \Generator
    {
        $feed = $this->tokenFeed("<?php $content");

        while($feed->valid()){
            $token = $feed->current();
            $feed->next();
            if(!is_array($token)){
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;

            switch ($tokenType){
                case T_CLASS:
                case T_FUNCTION:
                case T_INTERFACE:
                case T_TRAIT:
                    yield $this->extractClassnameBlock($feed);
                    break;
                default:
            }
        }
    }

    protected function tokenFeed($content)
    {
        foreach(token_get_all($content) as $token){
            yield $token;
        }
    }

    protected function extractClassnameBlock(\Generator $feed)
    {
        $classname = $this->extractClassname($feed);
        $this->extractContentBlock($feed);
        return $classname;
    }

    protected function extractContentBlock(\Generator $feed)
    {
        $content = '';
        $moustacheCount = 1;
        while($feed->valid()) {
            $token = $feed->current();
            $feed->next();

            if($moustacheCount == 0){
                break;
            }

            if(!is_array($token)){
                if($token == '}'){
                    $moustacheCount--;
                }
                $content .= $token;
                continue;
            }
            list($tokenType, $tokenValue, $line) = $token;
            $content .= $tokenValue;
        }
        return $content;
    }

    protected function extractClassname(\Generator $feed)
    {
        $classname = '';
        $stopClassNameSearch = false;

        while($feed->valid()){
            $token = $feed->current();
            $feed->next();

            if(!is_array($token)){
                if($token == '{'){
                    break;
                }
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;

            if($stopClassNameSearch){
                continue;
            }

            if($tokenType == T_IMPLEMENTS || $tokenType == T_FUNCTION || $tokenType == T_EXTENDS){
                $stopClassNameSearch = true;
                continue;
            }

            if($tokenType == T_STRING){
                $classname .= $tokenValue;
                continue;
            }
        }
        return $classname;
    }
}