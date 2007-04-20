# Copyright (c) 2007 Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""This package contains implementations of speech recognition
lattices in both backpointer table and DAG format.  The DAG format
includes routines for loading and saving lattices in Sphinx3 and HTK
format."""

import gzip
import re
import math

LOGZERO = -100000

class LatticeEntry(object):
    """Backpointer entry in a phone/word lattice"""
    __slots__ = 'frame', 'sym', 'score', 'prev'
    def __init__(self, frame, sym, score, prev):
        self.frame = frame
        self.sym = sym
        self.score = score
        self.prev = prev
    def __str__(self):
        if self.prev:
            prev = (self.prev.sym, self.prev.frame)
        else:
            prev = None
        return "(%d, %s, %f, %s)" % (self.frame, self.sym, self.score, prev)

class Lattice(list):
    """Backpointer table representation of a phone/word lattice"""
    def enter(self, frame, sym, score, history):
        while frame >= len(self):
            self.append([])
        entry = len(self[frame])
        if history:
            score -= history.score
        self[frame].append(LatticeEntry(frame, sym, score, history))
        return self[frame][-1]

    def backtrace(self, frame=-1):
        if frame == -1:
            frame = len(self)-1

        bestscore = LOGZERO
        for e in self[frame]:
            if e.score > bestscore:
                bestscore = e.score
                bestentry = e

        backtrace = []
        while bestentry:
            backtrace.append(bestentry)
            bestentry = bestentry.prev

        backtrace.reverse()
        return backtrace

class DagNode(object):
    """Node in a DAG representation of a phone/word lattice"""
    __slots__ = 'sym', 'entry', 'exits', 'score', 'prev'
    def __init__(self, sym, entry):
        self.sym = sym
        self.entry = entry
        self.exits = []
        self.score = LOGZERO
        self.prev = None
    def __str__(self):
        return "(%s, %d, %s)" % (self.sym, self.entry, self.exits)

class Dag(list):
    """Directed acyclic graph representation of a phone/word lattice"""
    __slots__ = 'start', 'end', 'header', 'frate'
    def __init__(self, lattice=None, frate=100):
        "Construct a DAG, optionally from a pre-existing backpointer lattice."
        self.frate = frate
        if lattice != None:
            self.lat2dag(lattice)

    fieldre = re.compile(r'(\S+)=(?:"((?:[^\\"]+|\\.)*)"|(\S+))')
    def htk2dag(self, htkfile):
        """Read an HTK-format lattice file to populate a DAG."""
        fh = open(htkfile)
        del self[0:len(self)]
        self.header = {}
        state='header'
        for spam in fh:
            if spam.startswith('#'):
                continue
            fields = dict(map(lambda (x,y,z): (x, y or z), self.fieldre.findall(spam.rstrip())))
            if 'N' in fields:
                nnodes = int(fields['N'])
                nodes = [None] * nnodes
                nlinks = int(fields['L'])
                links = [None] * nlinks
                state = 'items'
            if state == 'header':
                self.header.update(fields)
            else:
                if 'I' in fields:
                    frame = int(float(fields['t']) * self.frate)
                    node = DagNode(fields['W'], frame)
                    nodes[int(fields['I'])] = node
                    while len(self) <= frame:
                        self.append({})
                    self[frame][fields['W']] = node
                elif 'J' in fields:
                    # Link up existing nodes
                    fromnode = int(fields['S'])
                    tonode = int(fields['E'])
                    tofr = nodes[fromnode].entry
                    ascr = float(fields['a'])
                    lscr = float(fields['n'])
                    # FIXME: Not sure if this is a good idea
                    if not (tofr,ascr) in nodes[int(fromnode)].exits:
                        nodes[int(fromnode)].exits.append((nodes[int(tonode)].entry, ascr))
        # FIXME: Not sure if the first and last nodes are always the start and end?
        self.start = nodes[0]
        self.end = nodes[-1]

    headre = re.compile(r'# (-\S+) (\S+)')
    def sphinx2dag(self, s3file):
        """Read a Sphinx3-format lattice file to populate a DAG."""
        fh = gzip.open(s3file)
        del self[0:len(self)]
        self.header = {}
        state = 'header'
        logbase = math.log(1.0001)
        for spam in fh:
            spam = spam.rstrip()
            m = self.headre.match(spam)
            if m:
                arg, val = m.groups()
                self.header[arg] = val
                if arg == '-logbase':
                    logbase = math.log(float(val))
            if spam.startswith('#'):
                continue
            else:
                fields = spam.split()
                if fields[0] == 'Frames':
                    for i in range(0,int(fields[1]) + 1):
                        self.append({})
                elif fields[0] == 'Nodes':
                    state='nodes'
                    nnodes = int(fields[1])
                    nodes = [None] * nnodes
                elif fields[0] == 'Initial':
                    state = 'crud'
                    self.start = nodes[int(fields[1])]
                elif fields[0] == 'Final':
                    self.end = nodes[int(fields[1])]
                elif fields[0] == 'Edges':
                    state='edges'
                elif fields[0] == 'End':
                    state='done'
                else:
                    if state == 'nodes':
                        nodeid, word, sf, fef, lef = fields
                        node = DagNode(word, int(sf))
                        nodes[int(nodeid)] = node
                        self[int(sf)][word] = node
                    elif state == 'edges':
                        fromnode, tonode, ascr = fields
                        ascr = float(ascr) * logbase
                        tofr = nodes[int(tonode)].entry
                        # FIXME: Not sure if this is a good idea
                        if not (tofr,ascr) in nodes[int(fromnode)].exits:
                            nodes[int(fromnode)].exits.append((nodes[int(tonode)].entry, ascr))

    def dag2htk(self, htkfile):
        """Write out an HTK-format lattice file from a DAG."""
        fh = open(htkfile, 'w')

    def dag2sphinx(self, s3file):
        """Write out a Sphinx3-format lattice file from a DAG."""
        fh = gzip.open(s3file, 'wb')

    def lat2dag(self, lattice):
        """Turn a lattice inside-out to populate a DAG."""
        del self[0:len(self)]
        for i in range(0,len(lattice)):
            self.append({})
        # Don't include the start node in the lattice as it would
        # create a loop!
        self.start = DagNode('<s>', 0)
        self.start.exits.append((0, 0))
        # Create an end node for all the last exits to point to
        self.end = DagNode('</s>', len(lattice))
        self[-1]['</s>'] = self.end
        # A dummy start word if needed
        sword = LatticeEntry(0, '<s>', 0, None)
        # Build word nodes from history table
        for i, exits in enumerate(lattice):
            for e in exits:
                if e.prev == None:
                    e.prev = sword
                if e.sym in self[e.prev.frame]:
                    self[e.prev.frame][e.sym].exits.append((i, e.score))
                else:
                    self[e.prev.frame][e.sym] = DagNode(e.sym, e.prev.frame)
                    self[e.prev.frame][e.sym].exits.append((i, e.score))
        # Now clean up the lattice
        for frame in reversed(self):
            # First prune away word exits that go nowhere
            for node in frame.itervalues():
                # Remove dangling pointers
                node.exits = filter(lambda x: self[x[0]], node.exits)
                # Sort descending by score (actually this may be meaningless)
                node.exits.sort(lambda x,y: cmp(y[1], x[1]))
            # Now remove nodes that don't have any exits
            for sym in filter(lambda x: len(frame[x].exits) == 0, frame.iterkeys()):
                # But NOT the end node!!!
                if frame[sym] == self.end:
                    continue
                del frame[sym]

    def edges(self, node, lm=None):
        """Return a generator for the set of edges exiting node."""
        for frame, score in node.exits:
            for next in self[frame].itervalues():
                if lm:
                    # Build history for LM score if it exists
                    if node.prev:
                        syms = []
                        prev = node.prev
                        for i in range(2,lm.n):
                            if prev == None:
                                break
                            syms.append(prev.sym)
                            prev = prev.prev
                        syms.reverse()
                        syms.extend((node.sym, next.sym))
                        yield next, frame, score, lm.score(*syms)
                    else:
                        yield next, frame, score, lm.score(node.sym, next.sym)
                else:
                    yield next, frame, score, 0

    def n_nodes(self):
        """Return the number of nodes in the DAG"""
        return 1 + sum(map(len, self))

    def nodes(self):
        """Return all the nodes in the DAG"""
        return [self.start] + reduce(lambda x,y: x+y, map(lambda x: x.values(), self))

    def n_edges(self):
        """Return the number of edges in the DAG"""
        return (len(tuple(self.edges(self.start)))
                + sum(map(lambda x:
                          sum(map(lambda y:
                                  len(tuple(self.edges(y))), x.itervalues())), self)))

    def paths(self, start, lm=None, lw=3.5, ip=0.7):
        """Return all paths exiting start along with their scores.  WARNING:
        This will cause a combinatorial explosion, and is not actually
        recommended."""
        if start == self.end:
            return (([start],0),)
        paths = []
        if lm:
            for next, frame, ascr, lscr in self.edges(start, lm):
                for path, pscore in self.paths(next, lm):
                    paths.append(([start] + path, ascr + lw * lscr + math.log(ip) + pscore))
        else:
            for next, frame, score in self.edges(start):
                for path, pscore in self.paths(next):
                    paths.append(([start] + path, score + pscore))
        return paths

    def bestpath(self, lm=None, lw=3.5, ip=0.7, start=None):
        """Find best path through lattice using Dijkstra's algorithm"""
        Q = self.nodes()
        for u in Q:
            u.score = LOGZERO
        if start == None:
            start = self.start
        start.score = 0
        while Q:
            bestscore = LOGZERO
            bestidx = 0
            for i,u in enumerate(Q):
                if u.score > bestscore:
                    bestidx = i
                    bestscore = u.score
            u = Q[bestidx]
            del Q[bestidx]
            if u == self.end:
                return u
            for v, frame, ascr, lscr in self.edges(u, lm):
                score = ascr + lw * lscr + math.log(ip)
                if u.score + score > v.score:
                    v.score = u.score + score
                    v.prev = u

    def backtrace(self, end=None):
        """Return a backtrace from an optional end node after bestpath"""
        if end == None:
            end = self.end
        backtrace = []
        while end:
            backtrace.append(end)
            end = end.prev
        backtrace.reverse()
        return backtrace
