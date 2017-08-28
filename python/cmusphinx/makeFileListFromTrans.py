import numpy as np
def getFileListFromTran(trans,outFile):
	with open(trans,'r') as f:
		lines = f.readlines()
		lines = map(lambda x: x.strip(), lines)
		files = map(lambda x: x.split('(')[-1][:-1], lines)
	np.savetxt(outFile,files,fmt='%s')

def fixTran(trans):
	with open(trans,'r') as f:
		lines = f.readlines()
	lines = map(lambda x: x.strip(), lines)
	lines = map(lambda x: x.split('('), lines)
	for l in lines:
		l[-1] = l[-1].split('/')[-1]
	lines = map(lambda x: reduce(lambda a,b: a+'('+b,x),lines)
	np.savetxt(trans+'2',lines,fmt='%s')
# getFileListFromTran("../wsj/wsj0/transcripts/wsj0/wsj0.trans", "../wsj/wsj0/wsj0.filelist")
fixTran("../wsj/wsj0/transcripts/wsj0/wsj0.trans")