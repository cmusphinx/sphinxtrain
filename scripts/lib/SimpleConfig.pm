# -*- cperl -*-
## ====================================================================
##
## Copyright (c) 2007 Carnegie Mellon University.  All rights
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

package SimpleConfig;
require Exporter;
require IO::File;

use vars qw(@ISA @EXPORT);
@ISA=qw(Exporter);
@EXPORT=qw(parse_config);

sub parse_config_one {
    my $file = shift;

    my ($key, $val, $continued_val, @values);
    local $_;
    while (<$file>) {
	chomp;
	if (defined($continued_val)) {
	    s/^\s+//;
	    s/\s+$//;
	    if (/"((?:\"|[^"]+)*)"/
		or /'((?:\'|[^']+)*)'/) {
		$continued_val .= $1;
	    }
	    else {
		$continued_val .= " ". $_;
		$continued_val =~ s/\\$//;
	    }
	    unless (/\\$/) {
		push @values, $key => $continued_val;
		$continued_val = undef;
	    }
	}
	else {
	    if (/^\s*(\S+)\s*=\s*"((?:\"|[^"]+)*)"/
		or 
		/^\s*(\S+)\s*=\s*'((?:\'|[^']+)*)'/
		or
		/^\s*(\S+)\s*=\s*([^\s\\]+)/) {
		$key = $1;
		$val = $2;

		if (/\\$/) {
		    $continued_val = $val;
		}
		else {
		    push @values, $key => $val;
		}
	    }
	}
    }
    if (defined($continued_val)) {
	push @values, $key => $continued_val;
    }
    return @values;
}

sub parse_config {
    my @values;

    foreach my $file (@_) {
	my $fh = $file;
	unless (ref($file)) {
	    $fh = IO::File->new($file, 'r')
		or die "Failed to open $file: $!";
	}
	push @values, parse_config_one($fh);
    }

    return @values;
}

1;
__END__

=head1 NAME

SimpleConfig - Really simple configuration file parser

=head1 SYNOPSIS

 use SimpleConfig;
 %vars = parse_config($somefile);

=head1 DESCRIPTION

This module implements a really simple configuration file parser.  No
object oriented gunk or stuff to install from CPAN.  It exports one
function, C<parse_config>, which parses one or more file or
filehandles and returns any variable definitions of the form:

 key = value

as a list of key/value pairs (such that you could assign it to a hash,
as in the example above).

Other things in the files passed to it are ignored.  You can quote
values (not keys) using single or double quotes, which cause any
embedded quotes of the opposite type to be passed through directly.
If that isn't enough you can escape them with backslashes.

You can also split values across multiple lines using a trailing
backslash, like this:

 list_of_stuff = foo \
                 bar \
                 baz

Any leading or trailing whitespace will be skipped!  So add quotes if
you don't want that to happen.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
