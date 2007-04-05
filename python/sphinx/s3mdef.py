# Copyright (c) 2006 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""Read/write Sphinx-III model definition files.

This module reads and writes the text format model definiton (triphone
to senone mapping) files used by SphinxTrain, Sphinx-III, and
PocketSphinx.
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision$"

from numpy import *

def open(file):
    return S3Mdef(file)

class S3Mdef:
    "Read Sphinx-III format model definition files"
    def __init__(self, filename=None):
        self.info = {}
        if filename != None:
            self.read(filename)

    def read(self, filename):
        self.fh = file(filename)

        while True:
            version = self.fh.readline().rstrip()
            if not version.startswith("#"):
                break
        if version != "0.3":
            raise Exception("Model definition version %s is not 0.3" % version)
        info = {}
        while True:
            spam = self.fh.readline().rstrip()
            if spam.startswith("#"):
                break
            val, key = spam.split()
            info[key] = int(val)
        self.n_phone = info['n_base'] + info['n_tri']
        self.n_ci = info['n_base']
        self.n_tri = info['n_tri']
        self.n_ci_sen = info['n_tied_ci_state']
        self.n_sen = info['n_tied_state']
        self.n_tmat = info['n_tied_tmat']
        print("Loaded mdef (%d phones, %d ci, %d tri, %d sen)"
              % (self.n_phone, self.n_ci, self.n_tri, self.n_sen))

        # Skip field description lines
        spam = self.fh.readline().rstrip()
        spam = self.fh.readline().rstrip()
        
        ssidmap = {}
        self.phonemap = {}
        self.trimap = []
        self.fillermap = zeros(self.n_phone, 'b')
        self.tmatmap = zeros(self.n_phone, 'h')
        self.sidmap = zeros(self.n_sen, 'i')
        phoneid = 0
        self.max_emit_state = 0
        while True:
            spam = self.fh.readline().rstrip()
            if spam == "":
                break
            fields = spam.split()
            base, lc, rc, wpos, attrib, tmat = fields[0:6]
            sids = fields[6:-1]
            self.max_emit_state = max(self.max_emit_state, len(sids))

            # Build phone mappings
            if wpos not in self.phonemap:
                self.phonemap[wpos] = {}
            if base not in self.phonemap[wpos]:
                self.phonemap[wpos][base] = {}
            if lc not in self.phonemap[wpos][base]:
                self.phonemap[wpos][base][lc] = {}
            self.phonemap[wpos][base][lc][rc] = phoneid
            self.trimap.append((base, lc, rc, wpos))
            self.fillermap[phoneid] = (attrib == 'filler')
            self.tmatmap[phoneid] = int(tmat)
            
            # Build senone sequence mapping
            sseq = ",".join(sids)
            if sseq not in ssidmap:
                ssidmap[sseq] = []
            ssidmap[sseq].append(phoneid)
            for s in sids:
                # FIXME: Note this will break for one-to-many mappings
                self.sidmap[int(s)] = phoneid
            phoneid = phoneid + 1


        # Now invert the senone sequence mapping
        self.sseqmap = zeros(self.n_phone, 'i')
        print "Found %d unique senone sequences" % len(ssidmap)
        # This is a bogus way to fill an array with -1
        # (which is the ID for non-emitting states)
        self.sseq = fromfunction(lambda x,y:x-x-1, (len(ssidmap),
                                                    self.max_emit_state+1))
        sseqid = 0
        self.pidmap = []
        for sseq, phones in ssidmap.iteritems():
            sids = map(int, sseq.split(','))
            self.sseq[sseqid,0:len(sids)] = sids
            self.pidmap.append(phones)
            for p in phones:
                self.sseqmap[p] = sseqid
            sseqid = sseqid + 1

    def is_ciphone(self, sid):
        return sid >= 0 and sid < self.n_ci

    def is_cisenone(self, sid):
        return sid >= 0 and sid < self.n_ci_sen

    def phone_id(self, ci, lc='-', rc='-', wpos=None):
        if wpos == None:
            if lc != '-':
                # Try all word positions to find one that matches
                for wpos, pmap in self.phonemap.iteritems():
                    if ci in pmap and lc in pmap[ci] and rc in pmap[ci][rc]:
                        break
            else:
                wpos = '-' # context-independent phones have no wpos
        return self.phonemap[wpos][ci][lc][rc]

    def phone_from_id(self, pid):
        return self.trimap[pid]

    # FIXME: This may be bogus, see def. of sidmap above
    def phone_id_from_senone_id(self,sid):
        return self.sidmap[sid]
    
    # FIXME: This may be bogus, see def. of sidmap above
    def phone_from_senone_id(self, sid):
        return self.trimap[int(self.sidmap[sid])]

    def triphones(self, ci, lc, wpos=None):
        if wpos == None:
            out = []
            for wpos in self.phonemap:
                out.extend(self.triphones(ci, lc, wpos))
        else:
            try:
                return [(ci, lc, rc, wpos) for rc in self.phonemap[wpos][ci][lc]]
            except KeyError:
                return []
        return out

    def pid2ssid(self, pid):
        return int(self.sseqmap[pid])

    def pid2sseq(self, pid):
        return self.sseq[self.sseqmap[pid]]

    def pid2tmat(self, pid):
        return int(self.tmatmap[pid])

    def is_filler(self, pid):
        return int(self.fillermap[pid])
