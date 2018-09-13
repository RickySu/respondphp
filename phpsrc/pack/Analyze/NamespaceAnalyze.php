<?php
namespace Respond\Pack\Analyze;

class NamespaceAnalyze
{
    public function analyze(string $content): \Generator
    {
        $feed = $this->tokenFeed($content);
        while($feed->valid()){
            $token = $feed->current();
            $feed->next();
            if(!is_array($token)){
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;

            if($tokenType != T_NAMESPACE){
                continue;
            }

            yield $this->extractNamespaceBlock($feed);
        }
    }

    protected function tokenFeed($content)
    {
        foreach(token_get_all($content) as $token){
            yield $token;
        }
    }

    protected function extractNamespaceBlock(\Generator $feed)
    {
        $namespace = $this->extractNamespaceName($feed);
        return array(
            'namespace' => $namespace,
            'use' => $this->extractUseBlock($feed),
            'content' => $this->extractContentBlock($feed),
        );
    }

    protected function extractNamespaceName(\Generator $feed)
    {
        $namespace = '';
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

            if($tokenType == T_STRING || $tokenType == T_NS_SEPARATOR){
                $namespace .= $tokenValue;
                continue;
            }
        }

        return $namespace;
    }

    protected function extractContentBlock(\Generator $feed)
    {
        $content = '';
        $moustacheCount = 1;
        while($feed->valid()) {
            $token = $feed->current();
            $feed->next();
            if(!is_array($token)){
                if($token == '{'){
                    $moustacheCount++;
                }
                if($token == '}'){
                    $moustacheCount--;
                    if($moustacheCount == 0){
                        break;
                    }
                }
                $content .= $token;
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;
            if($tokenType == T_CURLY_OPEN || $tokenType == T_DOLLAR_OPEN_CURLY_BRACES){
                $moustacheCount++;
            }
            $content .= $tokenValue;
        }
        return $content;
    }

    protected function extractUseBlock(\Generator $feed)
    {
        $blocks = array();

        while($feed->valid()) {
            $token = $feed->current();

            if(!is_array($token)){
                $feed->next();
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;

            switch ($tokenType){
                case T_FUNCTION:
                case T_CLASS:
                case T_ABSTRACT:
                case T_INTERFACE:
                case T_TRAIT:
                    break 2;
                case T_USE:
                    $blocks[] = $this->extractUseExpression($feed);
                default:
                    $feed->next();
            }
        }
        return $blocks;
    }

    protected function extractUseExpression(\Generator $feed)
    {
        $expression = '';
        while($feed->valid()){
            $token = $feed->current();

            if(!is_array($token)){
                $expression .= $token;
                if($token == ';'){
                    break;
                }
            }
            list($tokenType, $tokenValue, $line) = $token;
            $expression .= $tokenValue;
            $feed->next();
        }

        return $expression;
    }
}
