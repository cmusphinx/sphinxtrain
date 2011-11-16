# -*- cperl -*-
## ====================================================================
##
## Copyright (c) 2006 Carnegie Mellon University.  All rights
## reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
##
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in
##    the documentation and/or other materials provided with the
##    distribution.
##
## This work was supported in part by funding from the Defense Advanced
## Research Projects Agency and the National Science Foundation of the
## United States of America, and the CMU Sphinx Speech Consortium.
##
## THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND
## ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
## THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
## PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
## NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## ====================================================================
##
## Author: David Huggins-Daines
##
use strict;

package Queue::PBS;
use Fcntl;
use File::Temp qw(tempfile);
use B::Deparse;
use Data::Dumper;
use File::Spec::Functions;
use Queue::Job;
use Sys::Hostname;

sub new {
    my $this = shift;
    my $class = ref $this || $this;

    if (-x '/usr/local/bin/qsub') {
	unshift(@_, pbsbin => '/usr/local/bin');
    }
    my $self = {@_};
    bless $self, $class;
}

sub network_path {
    my ($host, $path) = @_;
    $path = File::Spec->rel2abs($path);
    $path =~ s,/.automount/([^/]+)/root,/net/$1,;
    if ($path !~ m,^/net/,) {
	$path = "/net/${host}$path";
    }
    return $path;
}

sub submit_job {
    my $self = shift;

    # Create a job object from arguments (not so elegant really)
    my $job = Queue::Job->new(@_);

    # Create a temporary script with no args to give to PBS
    my ($tmpnam, $tmpfh);
    if (defined($self->{tempfile})) {
	$tmpnam = $self->{tempfile};
	open $tmpfh, ">$tmpnam" or die "Failed to open $tmpnam: $!";
     }
    else {
	($tmpfh, $tmpnam) = tempfile();
    }

    # Fix up some crucial paths (this is a HACK)
    my $hostname = hostname();
    $hostname =~ s/\..*$//;
    if (defined($job->{command})) {
	foreach my $arg (@{$job->{command}}) {
	    # Does it LOOK like an absolute path?
	    if (file_name_is_absolute($arg)) {
		# Then try to network-ify it...
		$arg = network_path($hostname, $arg);
	    }
	}
    }

    # Use Data::Dumper to generate properly quoted arguments
    local $Data::Dumper::Terse = 1;
    print $tmpfh "#!/usr/bin/perl\n";
    if (defined($job->{command})) {
	# Carry over the PATH and PYTHONPATH envvars
	foreach my $var (qw(PATH PYTHONPATH)) {
	    print $tmpfh "\$ENV{'$var'} = ", Dumper($ENV{$var}), ";\n";
	}
	print $tmpfh "exec(";
	foreach (@{$job->{command}}) {
	    print $tmpfh Dumper($_), ",\n\t";
	}
	print $tmpfh ");\n";
    }
    elsif (defined($job->{coderef})) {
	# Use B::Deparse to generate code
	my $deparse = B::Deparse->new();
	print $tmpfh "sub jobstuff ";
	print $tmpfh $deparse->coderef2text($job->{coderef});
	print $tmpfh "\nexit jobstuff(";
	foreach (@{$job->{codeargs}}) {
	    print $tmpfh Dumper($_), ",\n\t";
	}
	print $tmpfh ");\n";
    }
    else {
	die "Invalid Queue::Job object!\n";
    }
    close $tmpfh or die "Failed to close $tmpnam: $!";

    my ($qsubbin, @pbsargs);
    if (defined($self->{pbsbin})) {
	$qsubbin = catfile($self->{pbsbin}, 'qsub');
    }
    else {
	$qsubbin = 'qsub';
    }
    push @pbsargs, -q => $ST::CFG_QUEUE_NAME if defined $ST::CFG_QUEUE_NAME;
    push @pbsargs, -e => $job->{errfile} if defined $job->{errfile};
    push @pbsargs, -o => $job->{outfile} if defined $job->{outfile};
    push @pbsargs, -N => substr($job->{name}, 0, 15) if defined $job->{name};
    if (defined($job->{deps}) and @{$job->{deps}}) {
	push @pbsargs, -W => ('depend=afterok:'
			      . join(':', @{$job->{deps}}));
    }
    if (defined($job->{resources})) {
	foreach (@{$job->{resources}}) {
	    push @pbsargs, -l => $_;
	}
    }
    if (defined($job->{variables})) {
	foreach (@{$job->{variables}}) {
	    push @pbsargs, -v => $_;
	}
    }

    # Call qsub to submit the script
    die "Failed to fork: $!"
	unless defined(my $pid = open QSUB, "-|");
    if ($pid) {
	my ($output, $jobid);
	local $_;
	while (<QSUB>) {
	    $output .= $_;
	    chomp;
	    if (/^(\d+)\./) {
		$jobid = $1;
		last;
	    }
	}
	die "Queue submission failed: no Job ID\n" unless defined($jobid);
	close QSUB; # This does a waitpid($pid, 0) for us
	die "Queue submission failed with status $?\n" if $?;
	unlink $tmpnam;
	return $jobid;
    }
    else {
	open STDERR, ">&STDOUT";
	exec $qsubbin, @pbsargs, $tmpnam or die "Failed to exec: $!";
    }
}

sub query_job {
    my ($self, $jobid) = @_;

    my $qstatbin = catfile($self->{pbsbin}, 'qstat');
    my $pid = open QSTAT, "-|";
    die "Failed to fork: $!" unless defined $pid;
    if ($pid) {
	# FIXME: There is actually a way to get the exit status of a job
	while (<QSTAT>) {
	    return 1 if /Unknown/;
	    return 1 if /\bC\b/;
	    return -1 if /\bE\b/;
	}
	close QSTAT;
    }
    else {
	open STDERR, ">&STDOUT";
	exec $qstatbin, $jobid or die "Failed to exec: $!";
    }
    return 0;
}

sub waitfor_job {
    my ($self, $jobid, $timeout, $interval) = @_;

    my $elapsed = 0;
    $interval = 5 unless defined($interval);
    while (1) {
	my $rv = $self->query_job($jobid);
	# FIXME: There is actually a way to get the exit status of a job
	return 0 if $rv == 1; # 0 = success
	return 1 if $rv == -1; # 1 = failure
	return -1 if defined($timeout) and $elapsed > $timeout;
	sleep $interval;
	$elapsed += $interval;
    }
}

sub cancel_job {
    my ($self, $jobid) = @_;

    my $qdelbin = catfile($self->{pbsbin}, 'qdel');
    system($qdelbin, $jobid) == 0;
}

1;
__END__

=head1 NAME

Queue::PBS - Access to the PBS queue.

=head1 SYNOPSIS

  use Queue::PBS;
  my $q = Queue::PBS->new();
  my $id = $q->submit_job($cmd, @args);
  waitfor_job($id);
  cancel_job($id);

=head1 DESCRIPTION

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
