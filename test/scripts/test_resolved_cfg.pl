#!/usr/bin/env perl
## Resolved config: HMM type -> ci_hmm_dir; JSON shape; stale refresh via sphinxtrain.
use strict;
use File::Basename;
use File::Path qw(make_path remove_tree);
use File::Spec::Functions qw(catdir catfile);
use Cwd qw(abs_path getcwd);

my $root = abs_path(catdir(dirname($0), "..", ".."));
my $dump = catfile($root, "scripts", "util", "dump_resolved_cfg.pl");
my $src_cfg = catfile($root, "etc", "sphinx_train.cfg");
my $work = catfile($root, "test", "work", "resolved_cfg_test");
my $bindir = catfile($root, "build");

remove_tree($work) if -d $work;
make_path(catdir($work, "etc"));

sub write_project_cfg {
    open my $in, "<", $src_cfg or die "open $src_cfg: $!\n";
    open my $out, ">", catfile($work, "etc", "sphinx_train.cfg")
        or die "open project cfg: $!\n";
    while (my $line = <$in>) {
        $line =~ s/___DB_NAME___/testdb/g;
        $line =~ s|___BASE_DIR___|$work|g;
        $line =~ s|___SPHINXTRAIN_DIR___|$root|g;
        $line =~ s|___SPHINXTRAIN_BIN_DIR___|$bindir|g;
        print $out $line;
    }
    close $in;
    close $out;
}

sub set_hmm_type {
    my ($mode) = @_;
    return if $mode eq "cont";
    my $cfg = catfile($work, "etc", "sphinx_train.cfg");
    open my $fh, "+<", $cfg or die "open $cfg: $!\n";
    my @lines = <$fh>;
    for (@lines) {
        s/^\$CFG_HMM_TYPE = '\.cont\.';/#$&/;
        if ($mode eq "semi") {
            s/^#(\$CFG_HMM_TYPE  = '\.semi\.';)/$1/;
        } else {
            s/^#(\$CFG_HMM_TYPE  = '\.ptm\.';)/$1/;
        }
    }
    seek $fh, 0, 0 or die "seek: $!\n";
    print $fh @lines;
    truncate $fh, tell($fh) or die "truncate: $!\n";
    close $fh;
}

sub resolved_path {
    return catfile($work, "etc", "sphinx_train.resolved.json");
}

sub run_dump {
    my $cfg = catfile($work, "etc", "sphinx_train.cfg");
    my $out = resolved_path();
    system($^X, $dump, "-cfg", $cfg, "-out", $out) == 0
        or die "dump_resolved_cfg.pl failed\n";
    return $out;
}

sub cfg_dirlabel_via_config {
    my ($cfg) = @_;
    my $helper = catfile($root, "scripts", "util", "print_cfg_dirlabel.pl");
    open my $fh, "-|", $^X, $helper, $cfg or die "fork config check: $!\n";
    chomp(my $label = <$fh>);
    close $fh;
    wait;
    return $label;
}

sub check_hmm_modes {
    for my $mode (qw(cont semi ptm)) {
        write_project_cfg();
        set_hmm_type($mode);
        my $cfg = catfile($work, "etc", "sphinx_train.cfg");
        my $out = run_dump();
        my $py = <<"PY";
import json, sys
doc = json.load(open(sys.argv[1]))
label = sys.argv[2]
assert doc["variables"]["CFG_DIRLABEL"] == label, doc["variables"]["CFG_DIRLABEL"]
suffix = "ci_" + label
assert doc["derived"]["ci_hmm_dir"].endswith("." + suffix), doc["derived"]["ci_hmm_dir"]
assert "source_mtime" in doc["meta"]
PY
        system("python3", "-c", $py, $out, $mode) == 0
            or die "JSON check failed for HMM $mode\n";
        my $st_label = cfg_dirlabel_via_config($cfg);
        die "resolved CFG_DIRLABEL=$st_label expected $mode after Config load\n"
            if $st_label ne $mode;
        print "ok $mode -> ci_$mode (json + resolved)\n";
    }
}

sub check_stale_refresh {
    write_project_cfg();
    run_dump();
    my $resolved = resolved_path();
    my $cfg = catfile($work, "etc", "sphinx_train.cfg");
    my $before = (stat($resolved))[9];
    sleep 2;
    utime(time, time + 2, $cfg);
    my $driver = catfile($root, "scripts", "sphinxtrain");
    my $cwd = getcwd();
    chdir $work or die "chdir $work: $!\n";
    system("python3", $driver, "resolve-config") == 0
        or die "sphinxtrain resolve-config failed\n";
    chdir $cwd or die "chdir back: $!\n";
    my $after = (stat($resolved))[9];
    die "resolved json mtime did not advance after cfg touch\n" if $after <= $before;
    print "ok stale cfg triggers resolve-config\n";
}

sub check_decode_derived {
    write_project_cfg();
    my $out = run_dump();
    my $py = <<"PY";
import json, sys
from pathlib import Path
doc = json.load(open(sys.argv[1]))
v = doc["variables"]
model_dir = Path(v["CFG_MODEL_DIR"])
model_name = v["DEC_CFG_MODEL_NAME"]
d = doc["derived"]
assert d["decode_hmm_dir"] == str(model_dir / model_name)
assert d["decode_sendump"] == str(model_dir / model_name / "sendump")
assert d["pocketsphinx_batch"].endswith("pocketsphinx_batch")
assert d["decode_dictionary"].endswith(".dic")
assert d["decode_language_model"].endswith(".lm.DMP")
PY
    system("python3", "-c", $py, $out) == 0
        or die "decode derived check failed\n";
    print "ok decode paths in resolved json\n";
}

sub check_project_cfg_load {
    write_project_cfg();
    run_dump();
    my $out = resolved_path();
    my $py = <<"PY";
import sys
from pathlib import Path
sys.path.insert(0, sys.argv[1])
from cmusphinx.project_cfg import load_resolved
doc = load_resolved(Path(sys.argv[2]))
assert doc["derived"]["ci_hmm_dir"].endswith(".ci_cont")
PY
    system("python3", "-c", $py, catfile($root, "python"), catfile($work, "etc")) == 0
        or die "project_cfg load failed\n";
    print "ok project_cfg.load_resolved\n";
}

write_project_cfg();
check_hmm_modes();
check_decode_derived();
check_stale_refresh();
check_project_cfg_load();

print "test_resolved_cfg passed\n";
