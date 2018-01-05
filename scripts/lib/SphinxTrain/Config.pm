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
package SphinxTrain::Config;
use File::Spec;

sub import {
    my ($self, %args) = @_;

    if (lc($ARGV[0]) eq '-cfg') {
	(undef, $ST::CFG_FILE) = splice @ARGV, 0 ,2;
    } else {
	$ST::CFG_FILE = $args{-cfg} if (defined($args{-cfg}));
	$ST::CFG_FILE = $args{cfg} if (defined($args{cfg}));
	$ST::CFG_FILE = "./etc/sphinx_train.cfg" unless defined $ST::CFG_FILE;
    }

    if (-r $ST::CFG_FILE) {
	if (defined($args{pkg})) {
	    eval "package $args{pkg}; do \$ST::CFG_FILE";
	    die $@ if $@;
	}
	elsif (defined($args{-pkg})) {
	    eval "package $args{-pkg}; do \$ST::CFG_FILE";
	    die $@ if $@;
	}
	else {
	    package ST;
	    do $ST::CFG_FILE;
	}
    }
}

1;
__END__

=head1 NAME

SphinxTrain::Config - Configuration management for Sphinx Training

=head1 SYNOPSIS

  use SphinxTrain::Config;
  # -cfg specifies the default configuration if nothing was passed on the command-line
  use SphinxTrain::Config -cfg => "./etc/sphinx_train.cfg";
  # -ns specifies the namespace to load the configuration file in
  use SphinxTrain::Config -pkg => 'ST';
  # $ST::CFG_FOO is now defined

=head1 DESCRIPTION

This module locates the configuration file and loads it into the ST::
namespace.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
