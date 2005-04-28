use strict;
use Cwd;
use File::Path;

my $SphinxTrain_dir = getcwd();

my $task = "an4";
if ($#ARGV > -1) {
  $task = $ARGV[0];
}

print "Building task $task\n";

my $task_dir = "../$task";
chdir "$task_dir";

system("perl \"$SphinxTrain_dir/scripts_pl/setup_SphinxTrain.pl\" " .
       "-force -sphinxtraindir \"$SphinxTrain_dir\" -task $task");

mkdir "feat" if (! -e "feat");
open (CTL, "etc/${task}_train.fileids") 
  or die "Could not open control file etc/${task}_train.fileids\n";
while (<CTL>) {
  s/[\/\\][^\/\\]+$//g;
  mkpath ("feat/$_", 0, 0755) if (! -e "feat/$_");
}
close(CTL);

print "\n\nNow, please do:\n";
print "\tcd $task_dir\n";
print "And then, in Unix/Linux:\n";
print "\tperl scripts_pl/make_feats.pl -ctl etc/${task}_train.fileids (if needed)\n";
print "\tperl scripts_pl/RunAll.pl\n";
print "Or in Windows:\n";
print "\tperl scripts_pl\\make_feats.pl -ctl etc\\${task}_train.fileids (if needed)\n";
print "\tperl scripts_pl\\RunAll.pl\n";
