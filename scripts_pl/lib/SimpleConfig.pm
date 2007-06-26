# -*- cperl -*-
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
