#!/usr/bin/perl 

if(@ARGV<3){
    printf "$0 <rawcmdline> <texcmdline> <toolname> \n";
    exit(-1);
}

($rawcmdline,$texcmdline, $toolname)=@ARGV;

open(RAW,"$rawcmdline") ||die "Cannot open file $rawcmdline for reading\n";
@rawlines=<RAW>;
close(RAW);

open(TEX,"> $texcmdline") || die "Cannot open file $texcmdline for writing\n";

$toolname =~ s/_/\\_/g;
print TEX "\{\\it Usage: $toolname [options]\}\n";
print TEX "\\begin\{itemize\}\n";




$titleline=shift(@rawlines);

@chars=split(//,$titleline);

$pos=0;
$wanted_pos;
foreach $char (@chars){
    if($char eq "s"){
	$wanted_pos=$pos;
    }
    $pos++;
#    print "$char";
}


foreach $line (@rawlines){

    chomp($line);
    @chars=split(//,$line);

    $opt_def="";
    for($i=0;$i<$wanted_pos-3;$i++){
	$opt_def=sprintf("%s%s",$opt_def,$chars[$i]) ;
    }
    $opt_def=sprintf("%s%s",$opt_def,"\n");
    
    ($option, $default)=split(/\s+/,$opt_def);

#    print "$option, $default, ";

    $description="";
    for($i=$wanted_pos-3;$i<length($line);$i++){
	$description=sprintf("%s%s",$description,$chars[$i]);
    }
    @desc_words=split(/\s+/,$description);
    $description=join(" ",@desc_words);

#    print "$description\n";
    
    if(! defined $default || $default eq ""){
	$default = "NO DEFAULT VALUE";
    }
    print TEX "\\item\{\\bf $option\} (Default Value : $default) \\newline\n Description: $description \n \\newline\n";
}
print TEX "\\end\{itemize\}\n";

close(TEX);

