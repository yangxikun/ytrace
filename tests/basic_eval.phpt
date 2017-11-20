--TEST--
Test basic eval
--INI--
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
$a = eval('return ["foo" => "bar"];');
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%sbasic_eval.php	4	E	2	{eval}	return ["foo" => "bar"];
%sbasic_eval.php	4	A	1	$a	array (\n\t\'foo\' => \'bar\'\n)
%sbasic_eval.php	5	F	2	ytrace_disable	0
