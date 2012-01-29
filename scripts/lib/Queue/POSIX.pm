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

package Queue::POSIX;
use Queue::Job;
use POSIX qw(_exit setsid);

sub new {
    my $this = shift;
    my $class = ref $this || $this;

    my $self = {};
    bless $self, $class;
}

sub submit_job {
    my $self = shift;

    # Create a job object from arguments (not so elegant really)
    my $job = Queue::Job->new(@_);

    die "Failed to fork: $!" unless defined(my $pid = fork());
    if ($pid) {
	return $pid;
    }
    else {
	# Detach from parent process
	setsid();
	if (defined($job->{outfile})) {
	    open STDOUT, ">$job->{outfile}" or die "Failed to open $job->{outfile}: $!";
	}
	if (defined($job->{errfile})) {
	    open STDERR, ">$job->{errfile}" or die "Failed to open $job->{errfile}: $!";
	}
	if (defined($job->{deps})) {
	    my $alive = @{$job->{deps}};
	    while ($alive > 0) {
		$alive = kill(0, @{$job->{deps}});
		sleep(1);
	    }
	}
	if (defined($job->{command})) {
	    exec @{$job->{command}} or die "Failed to exec: $!";
	}
	elsif (defined($job->{coderef})) {
	    my $rv = $job->{coderef}(@{$job->{codeargs}});
	    _exit($rv);
	}
	else {
	    warn "Invalid Queue::Job object!\n";
	    _exit(1);
	}
    }
}

sub query_job {
    my ($self, $jobid) = @_;

    kill 0 => $jobid;
}

sub waitfor_job {
    my ($self, $jobid) = @_;

    waitpid $jobid, 0;
}

sub cancel_job {
    my ($self, $job) = @_;

    kill TERM => $job;
}

1;
__END__

=head1 NAME

Queue::POSIX - Simulate a queue using processes on POSIX systems

=head1 SYNOPSIS

  use Queue::POSIX;
  my $q = Queue::POSIX->new();
  my $id = $q->submit_job($cmd, @args);
  waitfor_job($id);
  cancel_job($id);

=head1 DESCRIPTION

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
