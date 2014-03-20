#!/usr/bin/env python

import sys
import math

import lattice
import sphinxbase



def baseword(sym):
    paren = sym.rfind('(')
    if paren != -1:
        return sym[0:paren]
    else:
        return sym



def load_lexicon(lexfile):
    # load dict or filler file to get word to phone information
    f = open(lexfile, 'r')
    lines = f.readlines()
    f.close()

    lex = {}
    for line in lines:
        temp = line.split()
        lex[temp[0]] = temp[1:len(temp)]
    return lex



def load_denlat(latfile):
    # Annotate the lattice with posterior probabilities
    dag = lattice.Dag(latfile)
    dag.remove_unreachable()

    arc = []
    for w in dag.nodes:
        for l in w.exits:
            if (w.entry,w.sym,l.dest.entry,l.dest.sym) not in arc:
                arc.append((w.entry,w.sym,l.dest.entry,l.dest.sym))

    lat = {}
    for n in arc:
        if n[1] != '<s>':
            left = []
            right = []
            for m in arc:
                if n[3] == '</s>' and (n[2],n[3],n[2]) not in right:
                    right.append((n[2],n[3],n[2]))
                elif m[0] == n[2] and m[1] == n[3]:
                    if (m[0],m[1],m[2]) not in right:
                        right.append((m[0],m[1],m[2]))
                if m[2] == n[0] and m[3] == n[1]:
                    if (m[0],m[1],m[2]) not in left:
                        left.append((m[0],m[1],m[2]))
            if (n[0],n[1],n[2]) not in lat:
                lat[(n[0],n[1],n[2])] = (left, right)
            else:
                for l in left:
                    if l not in lat[(n[0],n[1],n[2])][0]:
                        lat[(n[0],n[1],n[2])][0].append(l)
                for r in right:
                    if r not in lat[(n[0],n[1],n[2])][1]:
                        lat[(n[0],n[1],n[2])][1].append(r)
    
    return lat



def load_numlat(alignfile):
    # read force alignment result which is numerator lattice
    f = open(alignfile, 'r')
    f.readline()
    lines = f.readlines()
    f.close()
    
    wordseg = []
    for line in lines:
        temp = line.split()
        if len(temp) == 4:
            if '<s>' in temp:
                wordseg.append((0, 1, '<s>'))
                wordseg.append((1, int(temp[1])+2, '<sil>'))
            elif '</s>' in temp:
                wordseg.append((int(temp[0])+1, int(temp[1])+1, '<sil>'))
                wordseg.append((int(temp[1])+1, int(temp[1])+1, '</s>'))
            else:
                wordseg.append((int(temp[0])+1, int(temp[1])+2, temp[3]))

    if wordseg[1][2] == wordseg[2][2] and wordseg[1][2] == '<sil>' and wordseg[1][1] == wordseg[2][0]:
        sf = wordseg[1][0]
        ef = wordseg[2][1]
        wordseg.pop(1)
        wordseg.pop(1)
        wordseg.insert(1, (sf, ef, '<sil>'))
    n = len(wordseg)
    if wordseg[n-2][2] == wordseg[n-3][2] and wordseg[n-2][2] == '<sil>' and wordseg[n-3][1] == wordseg[n-2][0]:
        sf = wordseg[n-3][0]
        ef = wordseg[n-2][1]
        x = wordseg.pop(n-2)
        y = wordseg.pop(n-3)
        wordseg.insert(n-3, (sf, ef, '<sil>'))

    numlat = {}
    for i in range(1, len(wordseg)-1):
        sf = wordseg[i][0]
        ef = wordseg[i][1]
        word = wordseg[i][2]
        left = []
        right = []
        left.append((wordseg[i-1][0], wordseg[i-1][2], wordseg[i-1][1]))
        right.append((wordseg[i+1][0], wordseg[i+1][2], wordseg[i+1][1]))
        numlat[(sf, word, ef)] = (left, right)

    return numlat



def conv_lat_format(key, lat, filler, lm):
    # add word segment IDs and
    # use word IDs to present previsou and next word segments
    # also assign unigram language model score to each word segment

    newlat = []
    i = 0
    for sf, word, ef in key:
        i = i + 1
        
        if word in filler:
            score = math.log(0.1)
        else:
            score = lm.prob([baseword(word)])
        
        left = []
        for context in lat[(sf, word, ef)][0]:
            if '<s>' in context:
                ID = 0
            else:
                ID = key.index(context) + 1
            left.append(ID)
        right = []
        for context in lat[(sf, word, ef)][1]:
            if '</s>' in context:
                ID = 0
            else:
                ID = key.index(context) + 1
            right.append(ID)
        
        newlat.append((i, word, sf, ef, score, left, right))

    return newlat



def add_numlat_into_denlat(numlat, denlat):
    # add numerator lattice into denominator lattice
    n_arc = len(denlat)
    
    for i, word, sf, ef, score, left, right in numlat:
        aid = i + n_arc
        lids = []
        for ids in left:
            if ids == 0:
                lids.append(ids)
            else:
                lids.append(ids+n_arc)
        rids = []
        for ids in right:
            if ids == 0:
                rids.append(ids)
            else:
                rids.append(ids+n_arc)
        denlat.append((aid, word, sf, ef, score, lids, rids))

    return denlat



def output_lattice(lat, n_true_arc, outlatfile):
    # write lattice to a file
    f = open(outlatfile, 'w')
    f.write('Total arcs:\n')
    f.write('%d\n' % len(lat))
    f.write('True arcs:\n')
    f.write('%d\n' % n_true_arc)
    f.write('arc_id, arc_name, start frame, end frame, lmscore, number of preceding acrs, number of succeeding arcs, preceding arc_ids, succeeding arc_ids\n')
    for ID, word, sf, ef, score, left, right in lat:
        if score == None:
            break
        line = str(ID) + ' ' + word + ' ' + str(sf) + ' ' + str(ef) + ' ' + str(score) + ' ' + str(len(left)) + ' ' + str(len(right)) + ' <'
        for lid in left:
            line = line + ' ' + str(lid)
        line = line + ' >'
        for rid in right:
            line = line + ' ' + str(rid)
        f.write("%s\n" % line)
    f.close()



def write_lat(lm, filler, filelist, filecount, fileoffset, denlatdir, numlatdir, outdir):
    # write lattice for mmi training
    f = open(filelist, 'r')
    files = f.readlines()
    f.close()

    start = int(fileoffset)
    end = int(fileoffset) + int(filecount)
    if end > len(files):
        end = len(files)
    
    for i in range(start, end):
        line = files[i]
        fileid = line.strip('\n')
        alignfile = "%s/%s.wdseg" % (numlatdir, fileid)
        latfile = "%s/%s.lat.gz" % (denlatdir, fileid)

        print "process sent: %s" % fileid
        
        print "\t load numerator lattice %s ... " % alignfile
        try:
            numlat = load_numlat(alignfile)
        except IOError:
            print "\t can't open numerator lattice %s to read" % alignfile
            continue

        print "\t load denominator lattice %s ..." % latfile
        try:
            den_wordseg = load_denlat(latfile)
        except IOError:
            print "\t can't open denominator lattice %s to read" % latfile
            continue

        print "\t convert numerator lattice ..."
        numkeys = numlat.keys()
        numkeys.sort()
        conv_numlat = conv_lat_format(numkeys, numlat, filler, lm)

        print "\t convert denominator lattice ..."
        denkeys = den_wordseg.keys()
        denkeys.sort()
        denlat = conv_lat_format(denkeys, den_wordseg, filler, lm)

        print"\t add numerator lattice into denominator lattice ..."
        conv_denlat = add_numlat_into_denlat(conv_numlat, denlat)

        numlatfile = "%s/%s.numlat" % (outdir, fileid)
        denlatfile = "%s/%s.denlat" % (outdir, fileid)

        print "\t write numerator lattice to %s ..." % numlatfile
        try:
            output_lattice(conv_numlat, len(conv_numlat), numlatfile)
        except IOErro:
            print "\t can't write numerator lattice to %s" % numlatfile
            continue

        print "\t write denominator lattice to %s ...\n" % denlatfile
        try:
            output_lattice(conv_denlat, len(conv_numlat), denlatfile)
        except IOError:
            print "\t can't write denominator lattice to %s" % denlatfile

    print "ALL DONE\n"



# If this script is being run from the command-line, do this
if __name__ == '__main__':
    if len(sys.argv) != 9:
        sys.stderr.write("Usage: %s LMFILE FILLERFILE FILELIST FILECOUNT FILEOFFSET DENLATDIR NUMLATDIR OUTLATDIR\n" % (sys.argv[0]))
        sys.exit(1)

    command = ''
    for argv in sys.argv:
        command += argv + ' '
    print "%s\n" % command
    
    lmfile, fillerfile, filelist, filecount, fileoffset, denlatdir, numlatdir, outdir = sys.argv[1:]

    # load the filler dictionary
    filler = load_lexicon(fillerfile)

    # load the language model to score the word hypothesis in a lattice
    lm = sphinxbase.NGramModel(lmfile)

    # convert lattice and output to a file
    write_lat(lm, filler, filelist, filecount, fileoffset, denlatdir, numlatdir, outdir)

