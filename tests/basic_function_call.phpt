--TEST--
Test basic function call
--INI--
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
function foo($param1, $param2) {
	$val1 = $param1;
	$val2 = $param2;
}

foo(1, 2);
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%sbasic_function_call.php	9	F	2	foo	2	$param1	1	$param2	2
%sbasic_function_call.php	5	A	2	$val1	1
%sbasic_function_call.php	6	A	2	$val2	2
%sbasic_function_call.php	10	F	2	ytrace_disable	0
