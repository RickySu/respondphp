<?php
namespace Respond\Pack\Convert;


class PHPEndTagConverter extends BaseConverter
{
    protected function convertData(string $content)
    {
        $tokens = token_get_all($content);
        $lastToken = $tokens[count($tokens) - 1];
        if(!is_array($lastToken) || $lastToken[0] != T_CLOSE_TAG){
            $content .= '?>';
        }
        return $content;
    }
}
