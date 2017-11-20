--TEST--
Test basic call_user_func
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
function foo($a, $b) {
	$c = $a;
	$d = $b;
}
class A
{
	public function __call($name, $arguments) {
		call_user_func_array($name, $arguments);
	}
}
$a = new A();
call_user_func([$a, 'foo'], 1, 2);
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%sbasic_call_user_func.php	14	A	1	$a	class A {}
%sbasic_call_user_func.php	15	F	2	call_user_func	3	$function_name	array (\n\t0 => class A {},\n\t1 => \'foo\'\n)	$%s	1	$...	2
%sbasic_call_user_func.php	11	F	5	call_user_func_array	2	$function_name	\'foo\'	$parameters	array (\n\t0 => 1,\n\t1 => 2\n)
%sbasic_call_user_func.php	5	A	6	$c	1
%sbasic_call_user_func.php	6	A	6	$d	2
%sbasic_call_user_func.php	16	F	2	ytrace_disable	0
