<?php
namespace Respond\Pack\Convert;


class NamespaceConverter extends BaseConverter
{
    protected function convertData(string $content)
    {
        $feed = $this->tokenFeed($content);
        return $this->convertNamespace($feed);
    }

    protected function tokenFeed($content)
    {
        foreach(token_get_all($content) as $token){
            yield $token;
        }
    }

    protected function convertNamespace(\Generator $feed): string
    {
        $content = '';
        $namespaceStart = false;
        $needNamespaceCloseTag = false;

        while ($feed->valid()) {
            $token = $feed->current();
            $feed->next();

            if (!is_array($token)) {
                if($namespaceStart && $token == ';'){
                    $namespaceStart = false;
                    $needNamespaceCloseTag = true;
                    $content .= "{";
                    continue;
                }
                $content .= $token;
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;

            if ($tokenType != T_NAMESPACE) {
                $content .= $tokenValue;
                continue;
            }

            $content .= ' '.$tokenValue;
            $namespaceStart = true;
        }

        if($needNamespaceCloseTag){
            $content = $content."}";
        }
        return $content;
    }
}