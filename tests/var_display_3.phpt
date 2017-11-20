--TEST--
Test basic var_display_max_depth
--INI--
ytrace.var_display_max_depth=2
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
$arr = [1 => ['foo' => ['bar' => 'ytrace']], 2, 3, 4, 5];
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
/home/rokety/Downloads/php-5.6.31/ext/ytrace/tests/var_display_3.php	4	A	1	$arr	array (\n\t1 => array (\n\t\t\t\'foo\' => array (...)\n\t),\n\t2 => 2,\n\t3 => 3,\n\t4 => 4,\n\t5 => 5\n)
/home/rokety/Downloads/php-5.6.31/ext/ytrace/tests/var_display_3.php	5	F	2	ytrace_disable	0
