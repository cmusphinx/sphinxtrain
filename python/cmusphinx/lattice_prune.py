#!/usr/bin/env python

import os
import sys
import lattice
import sphinxbase



if __name__ == '__main__':
    if len(sys.argv) != 11:
        sys.stderr.write("Usage: %s ABEAM NBEAM LMWEIGHT LMFILE DENLATDIR PRUNED_DENLATDIR FILELIST TRANSFILE FILECOUNT FILEOFFSET\n" % (sys.argv[0]))
        sys.exit(1)

    # print command line
    command = ''
    for argv in sys.argv:
        command += argv + ' '
    print "%s\n" % command

    abeam, nbeam, lw, lmfile, denlatdir, pruned_denlatdir, ctlfile, transfile, filecount, fileoffset = sys.argv[1:]

    abeam = float(abeam)
    nbeam = float(nbeam)
    lw = float(lw)
    start = int(fileoffset)
    end = int(fileoffset) + int(filecount)

    # load language model
    lm = sphinxbase.NGramModel(lmfile)

    # read control file
    f = open(ctlfile, 'r')
    ctl = f.readlines()
    f.close()

    # read transcription file
    f = open(transfile, 'r')
    ref = f.readlines()
    f.close()

    sentcount = 0
    wer = 0
    nodecount = 0
    edgecount = 0
    density = 0
    # prune lattices one by one
    for i in range(start, end):
        c = ctl[i].strip()
        r = ref[i].split()
        del r[-1]
        if r[0] != '<s>': r.insert(0, '<s>')
        if r[-1] != '</s>': r.append('</s>')
        r = filter(lambda x: not lattice.is_filler(x), r)

        print "process sent: %s" % c
        
        # load lattice
        print "\t load lattice ..."
        dag = lattice.Dag(os.path.join(denlatdir, c + ".lat.gz"))
        dag.bypass_fillers()
        dag.remove_unreachable()

        # prune lattice
	dag.edges_unigram_score(lm,lw)
        dag.dt_posterior()

        # edge pruning
        print "\t edge pruning ..."
        dag.forward_edge_prune(abeam)
        dag.backward_edge_prune(abeam)
        dag.remove_unreachable()

        # node pruning
        print "\t node pruning ..."
        dag.post_node_prune(nbeam)
        dag.remove_unreachable()

        # calculate error
        err, bt = dag.minimum_error(r)

        # save pruned lattice
        print "\t saving pruned lattice ...\n"
        dag.dag2sphinx(os.path.join(pruned_denlatdir, c + ".lat.gz"))

        sentcount += 1
        nodecount += dag.n_nodes()
        edgecount += dag.n_edges()
        wer += float(err) / len(r)
        density += float(dag.n_edges())/len(r)

    print "Average Lattice Word Error Rate: %.2f%%" % (wer / sentcount * 100)
    print "Average Lattice Density: %.2f" % (float(density) / sentcount)
    print "Average Number of Node: %.2f" % (float(nodecount) / sentcount)
    print "Average Number of Arc: %.2f" % (float(edgecount) / sentcount)
    print "ALL DONE"
