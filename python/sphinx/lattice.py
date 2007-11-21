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
import numpy

LOGZERO = -100000

def logadd(x,y):
    if x < y:
        return logadd(y,x)
    if y == LOGZERO:
        return x
    else:
        return x + math.log(1 + math.exp(y-x))

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

def is_filler(sym):
    """Is this a filler word?"""
    if sym == '<s>' or sym == '</s>': return False
    return ((sym[0] == '<' and sym[-1] == '>') or
            (sym[0] == '+' and sym[-1] == '+'))

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
        fh = gzip.open(htkfile)
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
                            nodes[int(fromnode)].exits.append((tofr, ascr))

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
        return sum(map(len, self))

    def n_edges(self):
        """Return the number of edges in the DAG"""
        return (len(tuple(self.edges(self.start)))
                + sum(map(lambda x:
                          sum(map(lambda y:
                                  len(tuple(self.edges(y))), x.itervalues())), self)))

    def nodes(self):
        """Return a generator over all the nodes in the DAG"""
        for frame in self:
            for node in frame.values():
                yield node

    def bestpath(self, lm=None, lw=3.5, ip=0.7, start=None):
        """Find best path through lattice using Dijkstra's algorithm"""
        Q = list(self.nodes())
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

    def traverse_depth(self, start=None, lm=None):
        """Depth-first traversal of DAG nodes"""
        if start == None:
            start = self.start
        # Initialize the agenda (set of root nodes)
        roots = [start]
        # Keep a table of already seen nodes
        seen = {start:1}
        # Repeatedly pop the first one off of the agenda and push
        # all of its successors
        while roots:
            r = roots.pop()
            for v, f, s, l in self.edges(r, lm):
                if v not in seen:
                    roots.append(v)
                seen[v] = 1
            yield r

    def traverse_breadth(self, start=None, lm=None):
        """Breadth-first traversal of DAG nodes"""
        if start == None:
            start = self.start
        # Initialize the agenda (set of active nodes)
        roots = [start]
        # Keep a table of already seen nodes
        seen = {start:1}
        # Repeatedly pop the first one off of the agenda and shift
        # all of its successors
        while roots:
            r = roots.pop()
            for v, f, s, l in self.edges(r, lm):
                if v not in seen:
                    roots.insert(0, v)
                seen[v] = 1
            yield r

    def reverse_breadth(self, end=None, lm=None):
        """Breadth-first reverse traversal of DAG nodes"""
        if end == None:
            end = self.end
        if end.prev == None:
            self.find_preds()
        # Initialize the agenda (set of active nodes)
        roots = [end]
        # Keep a table of already seen nodes
        seen = {end:1}
        # Repeatedly pop the first one off of the agenda and shift
        # all of its successors
        while roots:
            r = roots.pop()
            for v in r.prev:
                if v not in seen:
                    roots.insert(0, v)
                seen[v] = 1
            yield r

    def bypass_fillers(self):
        """Bypass filler nodes in the lattice."""
        for u in self.traverse_breadth():
            for v, frame, ascr, lscr in self.edges(u):
                if is_filler(v.sym):
                    for vv, frame, ascr, lscr in self.edges(v):
                        if not is_filler(vv.sym):
                            u.exits.append((vv.entry, 0))

    def minimum_error(self, hyp, start=None):
        """Find the minimum word error rate path through lattice."""
        # Get the set of nodes in proper order
        nodes = tuple(self.traverse_breadth())
        # Initialize the alignment matrix
        align_matrix = numpy.ones((len(hyp),len(nodes)), 'i') * 999999999
        # And the backpointer matrix
        bp_matrix = numpy.zeros((len(hyp),len(nodes)), 'O')
        # Remove filler nodes from the reference
        hyp = filter(lambda x: not is_filler(x), hyp)
        # Bypass filler nodes in the lattice
        self.bypass_fillers()
        # Figure out the minimum distance to each node from the start
        # of the lattice, and the set of predecessors for each node
        for u in nodes:
            u.score = 999999999
            u.prev = []
        if start == None:
            start = self.start
        start.score = 1
        for i,u in enumerate(nodes):
            if is_filler(u.sym):
                continue
            for v, frame, ascr, lscr in self.edges(u):
                dist = u.score + 1
                if dist < v.score:
                    v.score = dist
                if not i in v.prev:
                    v.prev.append(i)
        def find_pred(ii, jj):
            bestscore = 999999999
            bestp = -1
            if len(nodes[jj].prev) == 0:
                return bestp, bestscore
            for k in nodes[jj].prev:
                if align_matrix[ii,k] < bestscore:
                    bestp = k
                    bestscore = align_matrix[ii,k]
            return bestp, bestscore
        # Now fill in the alignment matrix
        for i, w in enumerate(hyp):
            for j, u in enumerate(nodes):
                # Insertion = cost(w, prev(u)) + 1
                if len(u.prev) == 0: # start node
                    bestp = -1
                    inscost = i + 2 # Distance from start of ref
                else:
                    # Find best predecessor in the same reference position
                    bestp, bestscore = find_pred(i, j)
                    inscost = align_matrix[i,bestp] + 1
                # Deletion  = cost(prev(w), u) + 1
                if i == 0: # start symbol
                    delcost = u.score + 1 # Distance from start of hyp
                else:
                    delcost = align_matrix[i-1,j] + 1
                # Substitution = cost(prev(w), prev(u)) + (w != u)
                if i == 0 and bestp == -1: # Start node, start of ref
                    subcost = int(w != u.sym)
                elif i == 0: # Start of ref
                    subcost = nodes[bestp].score + int(w != u.sym)
                elif bestp == -1: # Start node
                    subcost = i - 1 + int(w != u.sym)
                else:
                    # Find best predecessor in the previous reference position
                    bestp, bestscore = find_pred(i-1, j)
                    subcost = align_matrix[i-1,bestp] + int(w != u.sym)
                align_matrix[i,j] = min(subcost, inscost, delcost)
                # Now find the argmin
                if align_matrix[i,j] == subcost:
                    bp_matrix[i,j] = (i-1, bestp)
                elif align_matrix[i,j] == inscost:
                    bp_matrix[i,j] = (i, bestp)
                else:
                    bp_matrix[i,j] = (i-1, j)
        # Find last node's index
        last = 0
        for i, u in enumerate(nodes):
            if u == self.end:
                last = i
        # Backtrace to get an alignment
        i = len(hyp)-1
        j = last
        bt = []
        while True:
            ip,jp = bp_matrix[i,j]
            if ip == i: # Insertion
                bt.append(('INS', nodes[j].sym))
            elif jp == j: # Deletion
                bt.append((hyp[i], 'DEL'))
            else:
                bt.append((hyp[i], nodes[j].sym))
            # If we consume both ref and hyp, we are done
            if ip == -1 and jp == -1:
                break
            # If we hit the beginning of the ref, fill with insertions
            if ip == -1:
                while True:
                    bt.append(('INS', nodes[jp].sym))
                    bestp, bestscore = find_pred(i,jp)
                    if bestp == -1:
                        break
                    jp = bestp
                break
            # If we hit the beginning of the hyp, fill with deletions
            if jp == -1:
                while ip >= 0:
                    bt.append((hyp[ip], 'DEL'))
                    ip = ip - 1
                break
            # Follow the pointer
            i,j = ip,jp
        bt.reverse()
        return align_matrix[-1,last], bt

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

    def find_preds(self):
        """Find predecessor nodes for each node in the lattice and store them
           in its 'prev' field."""
        for u in self.nodes():
            u.prev = []
        for w in self.traverse_breadth():
            for f, s in w.exits:
                for u in self[f].itervalues():
                    if w not in u.prev:
                        u.prev.append(w)

    def forward(self, lm=None):
        """Compute forward variable for all arcs in the lattice."""
        for w in self.nodes():
            w.prev = []
        # For each node in self
        for w in self.traverse_breadth(lm):
            # For each outgoing arc from w
            for i,x in enumerate(w.exits):
                wf, wa = x
                # This is alpha_t(w)
                alpha = LOGZERO
                # For each successor node to w
                for u in self[wf].itervalues():
                    # Add w to list of predecessors
                    if w not in u.prev:
                        u.prev.append(w)
                # If w has no predecessors the previous alpha is 1.0
                if len(w.prev) == 0:
                    alpha = wa
                # For each predecessor node to w
                for v in w.prev:
                    # Get language model score P(w|v) (bigrams only for now...)
                    if lm:
                        lscr = lm.score(v.sym, w.sym)
                    else:
                        lscr = 0
                    # Find the arc from v to w to get its alpha
                    for vf, vs in v.exits:
                        vascr, valpha, vbeta = vs
                        if vf == w.entry:
                            # Accumulate alpha for this arc
                            alpha = logadd(alpha, valpha + lscr + wa)
                # Update the acoustic score to hold alpha and beta
                w.exits[i] = (wf, (wa, alpha, LOGZERO))

    def backward(self, lm=None):
        """Compute backward variable for all arcs in the lattice."""
        # If predecessor nodes were not annotated, do that now
        if self.end.prev == None:
            self.find_preds()
        # For each node in self (in reverse):
        for w in self.reverse_breadth(lm):
            # For each predecessor to w
            for v in w.prev:
                # Beta for arcs into </s> = 1.0
                if w == self.end:
                    beta = 0
                else:
                    beta = LOGZERO
                    # Get language model score P(w|v) (bigrams only for now...)
                    if lm:
                        lscr = lm.score(v.sym, w.sym)
                    else:
                        lscr = 0
                    # For each outgoing arc from w
                    for wf, ws in w.exits:
                        wascr, walpha, wbeta = ws
                        # Accumulate beta for arc from v to w
                        beta = logadd(beta, wbeta + lscr + wascr)
                # Find the arc from v to w to update its beta
                for i, arc in enumerate(v.exits):
                    vf, vs = arc
                    if vf == w.entry:
                        vascr, valpha, spam = vs
                        v.exits[i] = (vf, (vascr, valpha, beta))
