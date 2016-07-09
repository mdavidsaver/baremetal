#!/usr/bin/env perl
# Run command, capture stdout and parse as TAP

use strict;
use warnings;
use 5.010;

use TAP::Parser;

my ($cmd) = @ARGV;

# travis-ci.org passes something wierd as stdin
# qemu hangs up when some ioctl() on stdin returns ERESTARTSYS when this is clearly not expected...
# so replace stdin with /dev/null
open(STDIN, '/dev/null');

my $out = `$cmd`;
my $result = ${^CHILD_ERROR_NATIVE};

my ($nplan, $npass, $nfail, $total) = (0,0,0,0);

if(length $out > 0) {
  my $parser = TAP::Parser->new({source => $out});

  while(my $result = $parser->next) {
      print $result->as_string;
      print "\n";
  }

  $nplan = $parser->tests_planned || 0;
  $npass = $parser->passed;
  $nfail = $parser->failed;
  $total = $npass+$nfail;

} else {
  print "<no stdout>\n";
}

# no tests pass.  Assume early crash...
$result = 1 if $npass==0;
# some tests not run
$result = 1 if $nplan!=$total;
# some tests fail
$result = 1 if $nfail>0;

print <<EOF;
Ran    $total/$nplan
Passed $npass/$nplan
Fail   $nfail/$nplan
EOF

exit($result);
