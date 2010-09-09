test: check

check:
	perl ./scripts/test_cp_parm.pl
	perl ./scripts/test_init_gau.pl
	perl ./scripts/test_init_gau_lda.pl
	perl ./scripts/test_printp.pl
	perl ./scripts/test_prunetree.pl
	perl ./scripts/test_norm.pl
	perl ./scripts/test_make_topology.pl
	perl ./scripts/test_mk_flat.pl
	perl ./scripts/test_mk_mdef_gen.pl
	perl ./scripts/test_bugcase1.pl
	perl ./scripts/test_bugcase2.pl

clean:
	rm -f printp.mean.txt
	rm -f printp.mixw.txt
	rm -f printp.tmat.txt
	rm -f printp.var.txt