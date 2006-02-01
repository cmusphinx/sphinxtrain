#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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

die "Configuration (e.g. etc/sphinx_train.cfg) not defined\n" unless defined ($CFG_DONE);

$CFG_OKAY_COLOR = '00D000';
$CFG_WARNING_COLOR = '555500';
$CFG_ERROR_COLOR = 'DD0000';

1;


sub ST_DateStr()
{
    my (@time) = localtime(time());
    my ($day_str) = sprintf ("%4d-%02d-%02d %02d:%02d",(1900 + $time[5]), ($time[4]+1),$time[3],$time[2],$time[1]);
}

sub ST_CreateHeader {
    my $logfile = shift;

    open ST_HTML_LOG,">$logfile";
    print ST_HTML_LOG "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n";
    print ST_HTML_LOG "<html>\n";
    print ST_HTML_LOG "<head>\n";
    print ST_HTML_LOG "<META HTTP-EQUIV=REFRESH CONTENT=60>\n"; # Force reloading every 60 seconds
    print ST_HTML_LOG "<meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\">\n";
    print ST_HTML_LOG "<title>$CFG_EXPTNAME</title>\n";
    print ST_HTML_LOG "</head>\n";
    print ST_HTML_LOG "<body BGCOLOR = \"#F0F0F5\">\n";
    print ST_HTML_LOG "<pre>\n";
    print ST_HTML_LOG "<CENTER><h1>${CFG_BASE_DIR}/${CFG_EXPTNAME}</h1></CENTER>\n";
    print ST_HTML_LOG "<CENTER> <h3>$hostname</h3></CENTER>\n";
    print ST_HTML_LOG "<hr>\n";
    close ST_HTML_LOG;
}

sub ST_Log ()
{
    my $message = shift;
    my (@time) = localtime(time());
    my ($day_str) = sprintf ("%4d-%02d-%02d %02d:%02d",(1900 + $time[5]), ($time[4]+1),$time[3],$time[2],$time[1]);
    my $logfile = "$CFG_BASE_DIR/$CFG_EXPTNAME.html";

    print "$message" if $CFG_VERBOSE;
    if (!-s $logfile) {
        ST_CreateHeader($logfile);
    }
    open ST_HTML_LOG,">>$logfile";
    if ($message =~ m/^(MODULE:).*/) {
        print ST_HTML_LOG "<hr>\n";
        chomp ($message);
	print ST_HTML_LOG "<b>$message\t($day_str)</b>\n";	# Put Date information on Module headers
    } else {
        print ST_HTML_LOG "$message";
    }
    close ST_HTML_LOG;
}

sub ST_HTML_Print () 
{
    my $message = shift;
    my $logfile = "$CFG_BASE_DIR/$CFG_EXPTNAME.html";

    if (! -s $logfile) {
        ST_CreateHeader($logfile);
    }
    open ST_HTML_LOG,">>$logfile";
    print ST_HTML_LOG "$message";
    close ST_HTML_LOG;

}

sub ST_FormatURL ()
{
    my $path = shift;
    my $label = shift;
    my $url;

    if ($path =~ m/^\/.*/) {
        $url = "file://";
    } else {
        $url = "file:///";
    }
    $url = "<a href=\"" . $url . "$path\">$label</a>";
    return $url;
}

sub ST_ImgSrc ()
{
    my $path = shift;
    my $url;

    if ($path =~ m/^\/.*/) {
        $url = "file://";
    } else {
        $url = "file:///";
    }
    $url = "<img src=\"" . $url . "$path\">";
    return $url;
}

sub ST_LogWarning ()
{
    my $message = shift;
    my (@time) = localtime(time());
    my ($day_str) = sprintf ("%4d-%02d-%02d %02d:%02d",(1900 + $time[5]), ($time[4]+1),$time[3],$time[2],$time[1]);
    my $logfile = "$CFG_BASE_DIR/$CFG_EXPTNAME.html";

    print "WARNING: $message" if $CFG_VERBOSE;
    if (! -s $logfile) {
        ST_CreateHeader($logfile);
    }
    open ST_HTML_LOG,">>$logfile";
    print ST_HTML_LOG "<p>WARNING: $message</p>";
    close ST_HTML_LOG;
}

sub ST_LogError ()
{
    my $message = shift;
    my (@time) = localtime(time());
    my ($day_str) = sprintf ("%4d-%02d-%02d %02d:%02d",(1900 + $time[5]), ($time[4]+1),$time[3],$time[2],$time[1]);
    my $logfile = "$CFG_BASE_DIR/$CFG_EXPTNAME.html";

    chomp ($message);		# remove \n if it exists

    print "$message" if $CFG_VERBOSE;
    if (! -s $logfile) {
        ST_CreateHeader($logfile);
    }
    open ST_HTML_LOG,">>$logfile";
    print ST_HTML_LOG "<p>$message</p>";
    close ST_HTML_LOG;
}

sub ST_Converged ()
{
  my $iter = shift;
  my $logdir = shift;
  my ($l,$p,$ratio);

  # See what happened at this iteration
  $l = `grep \"overall>\" $logdir/${CFG_EXPTNAME}.${iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}'`;

  if ($iter > 1) {
      my $tmp_iter = $iter - 1;
      # See what happened last iteration
      $p = `grep \"overall>\" $logdir/${CFG_EXPTNAME}.${tmp_iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}'`;
      # Compute it's ratio
      $ratio = ($l-$p)/abs($p);
      &ST_Log ("\t\tRatio: $ratio\n");
  }

  # Don't even bother checking convergence until we've reached a minimum number of loops
  return 0 if ($iter < $CFG_MIN_ITERATIONS);
  return 1 if ($ratio < $CFG_CONVERGENCE_RATIO);
  return 2 if ($iter > $CFG_MAX_ITERATIONS);
}

sub RunTool {

  my $cmd = shift;
  my $logfile = shift;
  my $progress = shift;

  open LOG,">$logfile";

  my $pipe = $|;
  $| = 1;				# Turn on autoflushing

  my $returnvalue = 0;

  if (open PIPE, "$cmd 2>&1 |") {

    while (<PIPE>) {
      print LOG "$_";
      if (/(ERROR).*/) {
	&ST_LogError ($_ . "\n");
      }
      if (/(FATAL).*/) {
	&ST_LogError ($_ . "\n");
        $returnValue = 1;
        last;
      }
      if ($progress) {
	# Keep track of progress being made.
	$processed_counter++  if (/.*(utt\>).*/);
	$percentage = int (($processed_counter / $ctl_counter) * 100);
	if (!($percentage % 10)) {
	  &ST_Log ("${percentage}% ") unless $printed;
	  $printed = 1;
	} else {
	  $printed = 0;
	}
      }
    }
    close PIPE;
    $| = $pipe;
    $date = localtime;
    print LOG "$date\n";
    close LOG;
  }
  if ($returnvalue) {
    &ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> FAILED </font>\n");
  } else {
    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");
  }

  return ($returnvalue);
}
