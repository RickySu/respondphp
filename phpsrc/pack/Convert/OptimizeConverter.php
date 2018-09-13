<?php
namespace Respond\Pack\Convert;


use Respond\Pack\Analyze\NamespaceAnalyze;

class OptimizeConverter extends BaseConverter
{

    protected function convertData(string $content)
    {
        $content = $this->removeAnnotation($content);
        $content = $this->trimSpace($content);
        $content = $this->optimizeTag($content);
        $content = $this->removeEndTag($content);
        $content = $this->mergeNamespace($content);
        return $content;
    }

    protected function optimizeTag(string $content)
    {
        return str_replace('?><?php', '', $content);
    }

    protected function removeEndTag(string $content)
    {
        $tokens = token_get_all($content);
        $lastToken = $tokens[count($tokens) - 1];

        if(!is_array($lastToken) || $lastToken[0] != T_CLOSE_TAG){
            return $content;
        }

        return substr($content, 0, -2);
    }

    protected function trimSpace(string $content)
    {
        $convertContent = '';
        foreach (token_get_all($content) as $token){
            if(!is_array($token)){
                $convertContent .= $token;
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;

            if($tokenValue == ' '){
                $convertContent .= $tokenValue;
                continue;
            }

            $convertContent .= trim($tokenValue);
        }

        return $convertContent;
    }

    protected function removeAnnotation(string $content):string
    {
        $convertContent = '';
        foreach (token_get_all($content) as $token){

            if(!is_array($token)){
                $convertContent .= $token;
                continue;
            }

            list($tokenType, $tokenValue, $line) = $token;

            if($tokenType == T_COMMENT || $tokenType == T_DOC_COMMENT){
                continue;
            }

            $convertContent .= $tokenValue;
        }

        return $convertContent;
    }

    protected function mergeNamespace(string $content)
    {
        $mergedContent = '<?php ';
        $blocks = array();
        $analyze = new NamespaceAnalyze();
        foreach($analyze->analyze($content) as $block){
            if(!isset($blocks[$block['namespace']])){
                $blocks[$block['namespace']] = array(
                    'content' => '',
                    'use' => array(),
                );
            }
            $blocks[$block['namespace']]['content'] .= $block['content'];
            $blocks[$block['namespace']]['use'] = array_merge($blocks[$block['namespace']]['use'], $block['use']);
        }

        foreach ($blocks as $namespace => $block){
            $blockUse = array_unique($block['use']);
            $mergedContent .= sprintf("namespace %s{%s%s}", $namespace, implode("", $blockUse), $block['content']);
        }
        return $mergedContent;
    }
}
