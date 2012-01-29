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

package Queue;
use Queue::Job;

sub new {
    my $this = shift;
    my $class = ref $this || $this;

    my $self = {};
    bless $self, $class;
}

sub submit_job {
    my $self = shift;

    my $job = Queue::Job->new(@_);

    my $rv;
    if (defined($job->{command})) {
	$rv = system @{$job->{command}};
    } elsif (defined($job->{coderef})) {
	$rv = $job->{coderef}(@{$job->{codeargs}});
    } else {
	die "Invalid Queue::Job object!\n";
    }

    return $rv;
}

sub query_job {
    my ($self, $jobid) = @_;

    # Always false
    return 0;
}

sub waitfor_job {
    my ($self, $jobid) = @_;

    # Do nothing
}

sub cancel_job {
    my ($self, $job) = @_;

    # Do nothing
}

1;
__END__

=head1 NAME

Queue - Base class for queue functions

=head1 SYNOPSIS

  use Queue;
  my $q = Queue->new();
  $q->submit_job($program, @args);
  $q->submit_job(\&subroutine, @args);

=head1 DESCRIPTION

This is a base class for various queueing systems.  It also serves as
a synchronous queue for local execution.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
