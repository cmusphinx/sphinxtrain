# -*- cperl -*-
## Evaluated CFG_* and derived paths; shared by Perl stages and Python tools.
package SphinxTrain::Resolved;

use strict;
use File::Basename qw(dirname);
use File::Spec;
use POSIX qw(strftime);

our $DOC;
our $CONFIG_PKG = "SphinxTrain::ProjectCfg";

sub resolved_path {
    my $cfg = $ST::CFG_FILE;
    return File::Spec->catfile(dirname($cfg), "sphinx_train.resolved.json");
}

sub _is_stale {
    my ($resolved, $cfg) = @_;
    return 1 unless -f $resolved;
    # >= covers same-second cfg edits (1s mtime resolution on some platforms).
    return (stat($cfg))[9] >= (stat($resolved))[9];
}

sub collect_variables {
    my ($pkg) = @_;
    $pkg = $CONFIG_PKG unless defined $pkg && $pkg ne "";
    my %vars;
    no strict "refs";
    for my $name (keys %{"${pkg}::"}) {
        next if $name =~ /::/;
        next if $name =~ /^(?:ISA|BEGIN|INC|AUTOLOAD|VERSION)$/i;
        my $val = ${"${pkg}::$name"};
        next unless defined $val;
        next if ref($val);
        $vars{$name} = "$val";
    }
    return \%vars;
}

sub _dictionary_path {
    if (defined($ST::CFG_FORCE_ALIGN_SPD) && $ST::CFG_FORCE_ALIGN_SPD eq "yes") {
        return File::Spec->catfile($ST::CFG_BASE_DIR, "falignout",
            "$ST::CFG_EXPTNAME.spdict");
    }
    if (defined($ST::CFG_G2P_MODEL) && $ST::CFG_G2P_MODEL eq "yes") {
        return "$ST::CFG_DICTIONARY.full";
    }
    return $ST::CFG_DICTIONARY;
}

sub _multipron_transcript_path {
    return File::Spec->catfile($ST::CFG_BASE_DIR, "multipron_align",
        "$ST::CFG_EXPTNAME.multipron.transcription");
}

sub _should_use_multipron_transcript {
    return 0 unless defined($ST::CFG_MULTIPRON);
    return 0 if $ST::CFG_MULTIPRON eq "no";
    return -f _multipron_transcript_path() ? 1 : 0;
}

sub compute_derived {
    my %derived;
    my $expt = $ST::CFG_EXPTNAME;
    my $dirlabel = $ST::CFG_DIRLABEL;

    $derived{ci_hmm_dir} = File::Spec->catfile(
        $ST::CFG_MODEL_DIR, "${expt}.ci_${dirlabel}"
    );

    if (defined($ST::CFG_CD_TRAIN) && $ST::CFG_CD_TRAIN eq "yes") {
        $derived{cd_hmm_dir} = File::Spec->catfile(
            $ST::CFG_MODEL_DIR,
            "${expt}.cd_${dirlabel}_$ST::CFG_N_TIED_STATES"
        );
    }

    if (defined($ST::CFG_FORCE_ALIGN_MODELDIR)) {
        $derived{falign_ci_hmm_dir} = $ST::CFG_FORCE_ALIGN_MODELDIR;
    }

    $derived{multipron_align_dir} = File::Spec->catdir(
        $ST::CFG_BASE_DIR, "multipron_align"
    );
    $derived{multipron_transcript} = _multipron_transcript_path();

    $derived{train_listoffiles} = $ST::CFG_LISTOFFILES;
    $derived{train_transcript} = $ST::CFG_TRANSCRIPTFILE;

    if (defined($ST::DEC_CFG_LISTOFFILES) && $ST::DEC_CFG_LISTOFFILES ne "") {
        $derived{test_listoffiles} = $ST::DEC_CFG_LISTOFFILES;
    }

    $derived{pocketsphinx_batch} = File::Spec->catfile(
        $ST::CFG_BIN_DIR, "pocketsphinx_batch"
    );
    if (defined($ST::DEC_CFG_MODEL_NAME) && $ST::DEC_CFG_MODEL_NAME ne "") {
        my $dec_model_dir = $ST::DEC_CFG_MODEL_DIR;
        $dec_model_dir = $ST::CFG_MODEL_DIR
            unless defined $dec_model_dir && $dec_model_dir ne "";
        $derived{decode_hmm_dir} = File::Spec->catfile(
            $dec_model_dir, $ST::DEC_CFG_MODEL_NAME
        );
        $derived{decode_sendump} = File::Spec->catfile(
            $derived{decode_hmm_dir}, "sendump"
        );
    }
    if (defined($ST::DEC_CFG_DICTIONARY) && $ST::DEC_CFG_DICTIONARY ne "") {
        $derived{decode_dictionary} = $ST::DEC_CFG_DICTIONARY;
    }
    if (defined($ST::DEC_CFG_LANGUAGEMODEL) && $ST::DEC_CFG_LANGUAGEMODEL ne "") {
        $derived{decode_language_model} = $ST::DEC_CFG_LANGUAGEMODEL;
    }
    if (defined($ST::DEC_CFG_TRANSCRIPTFILE) && $ST::DEC_CFG_TRANSCRIPTFILE ne "") {
        $derived{decode_transcript} = $ST::DEC_CFG_TRANSCRIPTFILE;
    }
    if (defined($ST::DEC_CFG_RESULT_DIR) && $ST::DEC_CFG_RESULT_DIR ne "") {
        $derived{decode_result_dir} = $ST::DEC_CFG_RESULT_DIR;
    }

    $derived{dictionary} = _dictionary_path();
    $derived{should_use_multipron_transcript} = _should_use_multipron_transcript();

    return \%derived;
}

sub build_document {
    my ($cfg_path) = @_;
    $cfg_path = $ST::CFG_FILE unless defined $cfg_path && $cfg_path ne "";

    my $source_mtime = (stat($cfg_path))[9];
    die "Cannot stat config $cfg_path: $!\n" unless defined $source_mtime;

    my $variables = collect_variables();
    apply_variables($variables);

    return {
        meta => {
            resolved_at    => strftime("%Y-%m-%dT%H:%M:%SZ", gmtime()),
            source         => $cfg_path,
            source_mtime   => $source_mtime + 0,
            sphinxtrain_dir => $ST::CFG_SPHINXTRAIN_DIR,
        },
        variables => $variables,
        derived   => compute_derived(),
    };
}

sub _json_escape {
    my ($s) = @_;
    $s =~ s/\\/\\\\/g;
    $s =~ s/"/\\"/g;
    $s =~ s/\n/\\n/g;
    $s =~ s/\r/\\r/g;
    $s =~ s/\t/\\t/g;
    return $s;
}

sub _json_value {
    my ($v) = @_;
    if (!defined $v) {
        return "null";
    }
    if (ref($v) eq "HASH") {
        my @pairs;
        for my $k (sort keys %$v) {
            push @pairs, _json_string($k) . ":" . _json_value($v->{$k});
        }
        return "{" . join(",", @pairs) . "}";
    }
    if ($v =~ /^-?(?:0|[1-9]\d*)(?:\.\d+)?$/ && $v !~ /^0\d/) {
        return $v;
    }
    if ($v eq "0" || $v eq "1") {
        return $v;
    }
    return _json_string("$v");
}

sub _json_string {
    my ($s) = @_;
    return '"' . _json_escape($s) . '"';
}

sub to_json {
    my ($doc) = @_;
    return _json_value($doc) . "\n";
}

sub read_document {
    my ($path) = @_;
    open my $fh, "<", $path or die "Cannot read $path: $!\n";
    local $/;
    my $text = <$fh>;
    close $fh;

    if (eval { require JSON::PP; 1 }) {
        return JSON::PP->new->decode($text);
    }
    die "JSON::PP is required to read $path\n";
}

sub write_file {
    my ($path, $cfg_path) = @_;
    my $doc = build_document($cfg_path);
    open my $fh, ">", $path or die "Cannot write $path: $!\n";
    print {$fh} to_json($doc);
    close $fh or die "Cannot close $path: $!\n";
    return $doc;
}

sub derived {
    my ($key) = @_;
    return undef unless $DOC && $DOC->{derived};
    return $DOC->{derived}{$key};
}

sub apply_variables {
    my ($vars) = @_;
    return unless $vars && ref($vars) eq "HASH";
    no strict "refs";
    for my $name (keys %$vars) {
        ${"ST::$name"} = $vars->{$name};
    }
}

sub sync_runtime {
    my $cfg = $ST::CFG_FILE;
    my $resolved = resolved_path();
    if (_is_stale($resolved, $cfg)) {
        $DOC = write_file($resolved, $cfg);
    } else {
        $DOC = read_document($resolved);
    }
    apply_variables($DOC->{variables});
    return $DOC;
}

1;
