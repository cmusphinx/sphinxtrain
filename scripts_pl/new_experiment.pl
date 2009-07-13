#!/usr/bin/perl -w
use strict;
use Pod::Usage;

my $new_exptname = shift;
pod2usage(2) unless defined $new_exptname;

open STC, "<etc/sphinx_train.cfg"
    or die "Failed to open etc/sphinx_train.cfg: Are you in the right directory? ($!)";
open SDC, "<etc/sphinx_decode.cfg"
    or die "Failed to open etc/sphinx_decode.cfg: Are you in the right directory? ($!)";
open NSTC, ">etc/sphinx_train_${new_exptname}.cfg"
    or die "Failed to open etc/sphinx_train_${new_exptname}.cfg: Are you in the right directory? ($!)";
open NSDC, ">etc/sphinx_decode_${new_exptname}.cfg"
    or die "Failed to open etc/sphinx_decode_${new_exptname}.cfg: Are you in the right directory? ($!)";

while (<STC>) {
    s/\$CFG_EXPTNAME\s*=\s*\"\${CFG_DB_NAME}([^\"]+)\"/\$CFG_EXPTNAME = \"\${CFG_DB_NAME}$1_$new_exptname\"/;
    s/\$CFG_EXPTNAME\s*=\s*\"\$CFG_DB_NAME\"/\$CFG_EXPTNAME = \"\${CFG_DB_NAME}_$new_exptname\"/;
    print NSTC;
}
close NSTC;

while (<SDC>) {
    s/etc\/sphinx_train\.cfg/etc\/sphinx_train_$new_exptname.cfg/g;
    print NSDC;
}
close NSDC;

__END__

=head1 NAME

new_experiment.pl - Copy training and decoding settings and rename

=head1 SYNOPSIS

B<new_experiment.pl> I<NEW_EXPTNAME>

=head1 DESCRIPTION

This is a simple script which copies the files C<sphinx_decode.cfg>
and C<sphinx_train.cfg>, adding I<NEW_EXPTNAME> to the new filenames
and attempting to also set the C<$CFG_EXPTNAME> variable in both
files.  From this point you can edit them to your needs.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
