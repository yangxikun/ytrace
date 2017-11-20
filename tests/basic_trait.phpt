--TEST--
Test basic trait
--INI--
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
trait methodCollection {
	public function foo($param1) {
		$param1 = 'bar';
		$this->foo1($param1);
	}
	protected function foo1($param1) {
		$param1 = 'bar';
	}
}

class A {
	use methodCollection;
}

$a = new A();
$a->foo(['1', '2']);
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%sbasic_trait.php	18	A	1	$a	class A {}
%sbasic_trait.php	19	F	2	A->foo	1	$param1	array (\n\t0 => \'1\',\n\t1 => \'2\'\n)
%sbasic_trait.php	6	A	2	$param1	\'bar\'
%sbasic_trait.php	7	F	3	A->foo1	1	$param1	\'bar\'
%sbasic_trait.php	10	A	3	$param1	\'bar\'
%sbasic_trait.php	20	F	2	ytrace_disable	0
