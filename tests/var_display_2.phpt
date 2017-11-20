--TEST--
Test basic var_display_max_data
--INI--
ytrace.var_display_max_data=4
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
$arr = '12345678';
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%svar_display_2.php	4	A	1	$arr	\'1234...\'
%svar_display_2.php	5	F	2	ytrace_disable	0
