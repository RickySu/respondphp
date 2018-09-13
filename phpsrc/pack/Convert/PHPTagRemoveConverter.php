<?php
namespace Respond\Pack\Convert;

class PHPTagRemoveConverter extends BaseConverter
{
    protected function convertData(string $content)
    {
        $convertContent = '';
        foreach (token_get_all($content) as $token){
            if(!is_array($token)){
                $convertContent .= $token;
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;

            if($tokenType == T_OPEN_TAG && $line == 0) {

            }

            $convertContent .= $tokenValue;
        }

        return $convertContent;
    }
}
