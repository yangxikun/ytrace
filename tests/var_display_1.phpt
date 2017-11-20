--TEST--
Test basic var_display_max_children
--INI--
ytrace.var_display_max_children=3
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
$arr = [1, 2, 3, 4, 5];
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%svar_display_1.php	4	A	1	$arr	array (\n\t0 => 1,\n\t1 => 2,\n\t2 => 3,\n\t..\n)
%svar_display_1.php	5	F	2	ytrace_disable	0
