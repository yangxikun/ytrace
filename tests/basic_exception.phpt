--TEST--
Test basic exception
--INI--
--SKIPIF--
<?php
if (!extension_loaded("ytrace")) print "skip";
if (!version_compare(phpversion(), "7.0", '<')) print "skip >= PHP 7.0";
?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
$e = new Exception('test basic exception');
throw $e;
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
Fatal error: Uncaught exception 'Exception' with message 'test basic exception' in %sbasic_exception.php:4
Stack trace:
#0 {main}
  thrown in %sbasic_exception.php on line 4
