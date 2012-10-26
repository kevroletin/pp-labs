use warnings;
use strict;
use Data::Dumper;

my %tests;

for (glob("*.in")) {
    /(\d+)\.(\d+)\.in/;
    my ($test, $subtest) = ($1, $2);
    $tests{$test}{$subtest} = $_;
}

my $total_ok = 1;
for my $t (sort keys %tests) {
    my $cmd = "../task1b ";
    for my $s (sort keys %{$tests{$t}}) {
        $cmd .= ' ' . $tests{$t}{$s};
    }
#    print $cmd;
    `$cmd 2> $t.res`;
    my $res = `tail -n1 $t.res`;
    my $out = `cat $t.out`;
    my $ok = $res == $out;
    printf "%02i - %s\n", $t, ['fail', 'ok']->[$ok];
    $total_ok &= $ok;
}

printf "=== %s ===", ['fail', 'pass']->[$total_ok];

