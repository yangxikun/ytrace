--TEST--
Test basic assignment
--INI--
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
$a = 10;
$a /= 2;
$a++;
$a--;
$a *= 2;
++$a;
--$a;
$a %= 3;
$a .= '0';
$a = $a;
$b = $a ? $a : '';
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%sbasic_assignment.php	4	A	1	$a	10
%sbasic_assignment.php	5	A	1	$a	10
%sbasic_assignment.php	6	A	1	$a	5
%sbasic_assignment.php	7	A	1	$a	6
%sbasic_assignment.php	8	A	1	$a	5
%sbasic_assignment.php	9	A	1	$a	10
%sbasic_assignment.php	10	A	1	$a	11
%sbasic_assignment.php	11	A	1	$a	10
%sbasic_assignment.php	12	A	1	$a	1
%sbasic_assignment.php	13	A	1	$a	\'10\'
%sbasic_assignment.php	14	A	1	$b	\'10\'
%sbasic_assignment.php	15	F	2	ytrace_disable	0
