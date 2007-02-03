#!/usr/local/bin/perl

use strict;
require './scripts/testlib.pl';

chomp(my $host=`../config.guess | xargs ../config.sub`);
my $bindir="../bin.$host/";
my $resdir="res/";
my $exec_resdir="mk_mdef_gen";
my $bin="$bindir$exec_resdir";
my $phonelist="mk_mdef_gen/communicator.phone.cmu";
my $matchdir="mk_mdef_gen";
my $dictfn="./res/communicator.dic.cmu.full";
my $transfn="./res/training.trans.falign.1of10";
test_help($bindir,$exec_resdir);
test_example($bindir,$exec_resdir);

#Testing CI mdef generation. 
my $i=0;
for($i=1;$i<=6;$i++)
{
    test_this("$bin -phnlstfn $phonelist -ocimdef ${i}.mdef -n_state_pm $i","DRY RUN mk_mdef_gen CI with ${i} states");
    open(TMP,"${i}.mdef")||die "can't read ${i}.mdef\n";
    my @tmp=<TMP>;
    close(TMP);

    shift(@tmp);
    open(TMP,">${i}.mod.mdef");    
    foreach my $tmpline (@tmp){
	print TMP $tmpline;
    }
    close(TMP);

    compare_these_two("./${i}.mod.mdef","${matchdir}/${i}st.ci.mdef","MAKE_MDEF_GEN CI, generate ${i} states");
    unlink("./${i}.mod.mdef","./${i}.mdef");
}

#Testing for CD untied mdef generation. 

for($i=1;$i<=6;$i++)
{

    test_this("$bin -phnlstfn $phonelist -dictfn $dictfn  -lsnfn $transfn  -ountiedmdef $i.mdef -n_state_pm $i","DRY RUN mk_mdef_gen UT with ${i} states");

    open(TMP,"${i}.mdef")||die "can't read ${i}.mdef\n";
    my @tmp=<TMP>;
    close(TMP);

    shift(@tmp);
    open(TMP,">${i}.mod.mdef");    
    foreach my $tmpline (@tmp){
	print TMP $tmpline;
    }
    close(TMP);

    compare_these_two("./${i}.mod.mdef","${matchdir}/${i}st.ut.mdef","MAKE_MDEF_GEN UT, generate ${i} states");
    unlink("./${i}.mod.mdef","./${i}.mdef");
}

#Testing for CD generate all triphone operation. 
$dictfn="./res/communicator.dic.cmu";

test_this("$bin -phnlstfn $phonelist -dictfn $dictfn -oalltphnmdef 2.mdef -n_state_pm 2","DRY RUN mk_mdef_gen ALL with 2 states");

open(TMP,"2.mdef")||die "can't read 2.mdef\n";
my @tmp=<TMP>;
close(TMP);

shift(@tmp);
open(TMP,">2.mod.mdef");    
foreach my $tmpline (@tmp){
    print TMP $tmpline;
}
close(TMP);

compare_these_two("./2.mod.mdef","${matchdir}/2st.all.mdef","MAKE_MDEF_GEN ALL, generate 2 states");
unlink("./2.mod.mdef","./2.mdef");
