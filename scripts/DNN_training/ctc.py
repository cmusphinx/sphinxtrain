import sys  
import fst
import random
import math
import numpy as np
from scipy.sparse import bsr_matrix
reload(sys)  
sys.setdefaultencoding('utf8')

def ran_lab_prob(n_samps):
    r = [random.random() for i in range(138)]
    s = sum(r)
    return [[i/s for i in r]]*n_samps

def genBigGraph(label_prob, symbols, seq_len, label='x'):
    t = fst.Transducer()
    sym=fst.SymbolTable()

    symbols = map(str,symbols)
    x=0
    for j in range(seq_len):
        for i in range(len(symbols)):
            prob =  label_prob[j][i] #"%.4f" %
            t.add_arc(0+x, 1+x,str(label+str(j)),symbols[i],-math.log(prob))
        x+=1
    t[j+1].final = -1
    return t

def gen_utt_graph(labels,symdict):
    t2=fst.Transducer()
    sym=fst.SymbolTable()
    #3x3 states for this example
    count = 0
    x = 0
    # print labels
    for l in labels:
        symbols = symdict[l]
        symbols = map(str,symbols)
        for i in range(len(symbols)):
            if i == 0:
                t2.add_arc(0+x,1+x,symbols[i],str(l+"/"+"("+symbols[i]+")"))
            else:
                t2.add_arc(0+x,1+x,symbols[i],str(sym.find(0)+"("+symbols[i]+")"))
            t2.add_arc(1+x,1+x,symbols[i],str(sym.find(0)+"("+symbols[i]+")"))
            
            x+=1

    t2[x].final=True
    return t2

def gen_parents_dict(graph):
    parents={}
    for state in graph.states:
        for arc in state.arcs:
            if arc.nextstate in parents:
                parents[arc.nextstate].append(state.stateid)
            else:
                parents[arc.nextstate]=[state.stateid]
    return parents

def make_prob_dict(graph,n_samps,n_labels):
    y_t_s = np.zeros((n_samps + 1,n_labels)) # dictionary to store probabilities indexed by time and label
    F = [0]
    for t in range(n_samps + 1):
        # y_t_s[t] = {}
        for s in F:
            arcs = graph[s].arcs
            for a in arcs:
                osym = graph.osyms.find(a.olabel)
                osym = osym[osym.find("(")+1:osym.find(")")]
                y_t_s[t][int(osym)] = np.exp(-1 * float(a.weight))
        F = map(lambda x: map(lambda y: y.nextstate,graph[x].arcs),F)
        F = set([s for ss in F for s in ss])
    y_t_s = bsr_matrix(y_t_s,dtype='float32')
    return y_t_s

def calc_alpha(n_samps, symbols, y_t_s):
    alpha = {}
    # symbols = map(str,symbols)
    for t in range(n_samps + 1):
        alpha[t] = {}
        for i in range(len(symbols)):
            # print alpha
            # print t,i,
            if t == 0:
                if i == 0:
                    alpha[t][symbols[i]] = float(y_t_s[t,symbols[i]])
                else:
                    alpha[t][symbols[i]] = 0.0
            else:
                if i == 0:
                	alpha[t][symbols[i]] = float(y_t_s[t,symbols[i]]) * alpha[t-1][symbols[i]]
                else:
                    alpha[t][symbols[i]] = float(y_t_s[t,symbols[i]]) * (alpha[t-1][symbols[i]] + alpha[t-1][symbols[i-1]])
            # print alpha[t][symbols[i]]
    return alpha

def calc_beta(n_samps,symbols,y_t_s):
    beta = {}
    # symbols = map(str,symbols)
    for t in range(n_samps,0,-1):
        beta[t] = {}
        for i in range(len(symbols)):
            if t == n_samps:
                if i == len(symbols) - 1:
                    beta[t][symbols[i]] = float(y_t_s[t,symbols[i]])
                else:
                    beta[t][symbols[i]] = 0.0
            else:
                if i < len(symbols) - 1:
                    score = beta[t+1][symbols[i]] + beta[t+1][symbols[i+1]]
                else:
                    score = beta[t+1][symbols[i]]
                beta[t][symbols[i]] = float(y_t_s[t,symbols[i]]) * score
    return beta

def print_graph(t):
	for state in t.states:
	    for arc in state.arcs:
	        print('{} -> {} / {}:{} / {}'.format(state.stateid,
                                             arc.nextstate,
                                             t.isyms.find(arc.ilabel),
                                             t.osyms.find(arc.olabel),
                                             arc.weight))
# symbols = ['G1','G2','G3','UH1','UH2','UH3','D1','D2','D3']
# t = genBigGraph(ran_lab_prob,symbols,11)
# labels  = ['G','UH','D']

# symdict={'G': ['G1','G2','G3'],
#          'UH': ['UH1','UH2','UH3'],
#          'D': ['D1','D2','D3']}
# t2 = gen_utt_graph(labels, symdict)
# t3 = t>>t2
# parents = gen_parents_dict(t3)
# y_t_s = make_prob_dict(t3)
# print calc_alpha(10,symbols,y_t_s)
# print calc_beta(10,symbols,y_t_s)