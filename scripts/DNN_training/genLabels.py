import numpy as np
from functools import reduce
import os
import threading
import time
import sys
from sklearn.preprocessing import StandardScaler
import utils
from keras.preprocessing.sequence import pad_sequences
import ctc
import fst
import librosa

done = 0
def ping():
	curr_time = int(time.time())
	while done == 0:
		if (int(time.time()) != curr_time):
			curr_time = int(time.time())
			sys.stdout.write('.')
			sys.stdout.flush()


def read_sen_labels_from_mdef(fname, onlyPhone=True):
	labels = np.loadtxt(fname,dtype=str,skiprows=10,usecols=(0,1,2,3,6,7,8))
	labels = map(lambda x: 
					[reduce(lambda a,b: a+' '+b, 
						filter(lambda y: y != '-', x[:4]))] + list(x[4:]), labels)
	if onlyPhone:
		labels = labels[:44]
	phone2state = {}
	for r in labels:
		phone2state[r[0]] = map(int, r[1:])
	return phone2state

def frame2state(fname, phone2state, onlyPhone=True):
	with open(fname,'r') as f:
		lines = f.readlines()[2:]
	lines = map(lambda x: x.split()[2:], lines)
	if onlyPhone:
		lines = map(lambda x: x[:2],lines)
	else:
		lines = map(lambda x: [x[0], reduce(lambda a,b: a+' '+b,x[1:])],lines)
	for l in lines:
		if l[1] not in phone2state:
			l[1] = l[1].split()[0]
	states = map(lambda x: phone2state[x[1]][int(x[0])], lines)
	return (list(states))

def loadDict(filename):
	def mySplit(line):
		line = line.split()
		for i in range(1,len(line)):
			line[i] = "{0}1 {0}2 {0}3".format(line[i])
		line = [line[0], reduce(lambda x,y: x+ ' ' +y, line[1:])]
		return line
	with open(filename) as f:
		d = f.readlines()
	d = map(lambda x: x.split(),d)
	myDict = {}
	for r in d:
		myDict[r[0]] = r[1:]
	return myDict

def loadTrans(trans_file,pDict):
	trans = {}
	with open(trans_file) as f:
		lines = f.readlines()
	for line in lines:
		line = line.split()
		fname = line[-1][1:-1]
		labels = map(lambda x: pDict.setdefault(x,-1), line[:-1])
		labels = filter(lambda x: x!=-1, labels)
		if labels == []:
			continue
		labels = reduce(lambda x,y: x + y, labels)
		trans[fname] = labels
	return trans

def trans2labels(trans,phone2state):
	d = {}
	for u in trans:
		labels = trans[u]
		labels = map(lambda x: phone2state[x], labels)
		labels = reduce(lambda x,y: x + y, labels)
		d[u] = labels
	return d

def genDataset(train_flist, dev_flist, test_flist, n_feats,
				feat_path, feat_ext, stseg_path, stseg_ext, mdef_fname, 
				outfile_prefix, context_len=None, 
				keep_utts=False, ctc_labels=False, pDict_file=None,
				trans_file=None, make_graph=False,cqt=False,
				max_len=None,n_deltas=0,pad=False):
	assert(os.path.exists(stseg_path))
	train_files = np.loadtxt(train_flist,dtype=str)	
	dev_files = np.loadtxt(dev_flist,dtype=str)
	if test_flist != None:
		test_files = np.loadtxt(test_flist,dtype=str)
	else:
		test_files = []
	phone2state = read_sen_labels_from_mdef(mdef_fname,onlyPhone=False)

	if ctc_labels:
		pDict = loadDict(pDict_file)
		trans = loadTrans(trans_file,pDict)
		label_dict = trans2labels(trans, phone2state)
		stseg_files_train = filter(lambda x: x.split('/')[-1] in label_dict, train_files)
		stseg_files_test = filter(lambda x: x.split('/')[-1] in label_dict, test_files)
		stseg_files_dev = filter(lambda x: x.split('/')[-1] in label_dict, dev_files)
		stseg_files = stseg_files_train + stseg_files_dev + stseg_files_test
		print "Training Files: %d 	Dev Files: %d	Testing Files: %d" % (len(stseg_files_train), len(stseg_files_dev), len(stseg_files_test))
	else:
		stseg_files_train = map(lambda x: x.split('/')[-1]+stseg_ext,train_files)
		stseg_files_test = map(lambda x: x.split('/')[-1]+stseg_ext,test_files)
		stseg_files_dev = map(lambda x: x.split('/')[-1]+stseg_ext,dev_files)
		stseg_files_train = filter(lambda x: os.path.exists(stseg_path + x), stseg_files_train)
		stseg_files_test = filter(lambda x: os.path.exists(stseg_path + x), stseg_files_test)
		stseg_files_dev = filter(lambda x: os.path.exists(stseg_path + x), stseg_files_dev)

		stseg_files = stseg_files_train + stseg_files_dev + stseg_files_test
		print "Training Files: %d 	Dev Files: %d	Testing Files: %d" % (len(stseg_files_train), len(stseg_files_dev), len(stseg_files_test))

	train_files = map(lambda x: feat_path+x+feat_ext,train_files)
	dev_files = map(lambda x: feat_path+x+feat_ext,dev_files)
	test_files = map(lambda x: feat_path+x+feat_ext,test_files)

	X_Train = []
	Y_Train = []
	X_Test = []
	Y_Test = []
	X_Dev = []
	Y_Dev = []
	framePos_Train = []
	framePos_Test = []
	framePos_Dev = []
	filenames_Train = []
	filenames_Test = []
	filenames_Dev = []
	active_states_Train = []
	active_states_Test = []
	active_states_Dev = []
	# allData = []
	# allLabels = []
	pos = 0
	scaler = StandardScaler(copy=False,with_std=False)
	n_states = np.max(phone2state.values())+1
	print n_states
	state_freq_Train = [0]*n_states
	state_freq_Dev = [0]*n_states
	state_freq_Test = [0]*n_states



	for i in range(len(stseg_files)):
		if i < len(stseg_files_train):
			# print '\n train'
			frames = framePos_Train
			allData = X_Train
			allLabels = Y_Train
			filenames = filenames_Train
			state_freq = state_freq_Train
			files = train_files
			active_state = active_states_Train
		elif i < len(stseg_files_train) + len(stseg_files_dev):
			# print '\n dev'
			frames = framePos_Dev
			allData = X_Dev
			allLabels = Y_Dev
			filenames = filenames_Dev
			state_freq = state_freq_Dev
			files = dev_files
			active_state = active_states_Dev
		else:
			# print '\n test'
			frames = framePos_Test
			allData = X_Test
			allLabels = Y_Test
			filenames = filenames_Test
			state_freq = state_freq_Test
			files = test_files
			active_state = active_states_Test

		sys.stdout.write("\r%d/%d 	" % (i,len(stseg_files)))
		sys.stdout.flush()
		f = stseg_files[i]
		
		[data_file] = filter(lambda x: f[:-9] in x, files)
		if cqt:
			y,fs=librosa.load(data_file,sr=None)
			data = np.absolute(librosa.cqt(y,sr=fs,window=np.hamming,
								hop_length=160, n_bins=64, bins_per_octave=32).T)
			# print data.shape
		else:
			data = utils.readMFC(data_file,n_feats).astype('float32')
		data = scaler.fit_transform(data)

		if ctc_labels:
			labels = label_dict[data_file.split('/')[-1][:-4]]
			if make_graph:
				t = ctc.genBigGraph(ctc.ran_lab_prob(data.shape[0]),
										set(labels),
										data.shape[0])
				t2 = ctc.gen_utt_graph(trans[data_file.split('/')[-1][:-4]],phone2state)
				assert set([e for (e,_) in t.osyms.items()]) == set([e for (e,_) in t2.isyms.items()])
				t.osyms = t2.isyms
				t3 = t >> t2
				parents = ctc.gen_parents_dict(t3)
				y_t_s = ctc.make_prob_dict(t3,data.shape[0],n_states)
				active_state.append(y_t_s)
			# print active_state
			nFrames = data.shape[0]
		else:
			labels = frame2state(stseg_path + f, phone2state,onlyPhone=False)
			nFrames = min(len(labels), data.shape[0])
			sys.stdout.write('(%d,%d) (%d,)' % (data.shape + np.array(labels).shape))
			data = data[:nFrames]
			labels = labels[:nFrames]
		if context_len != None:
			pad_top = np.zeros((context_len,data.shape[1]))
			pad_bot = np.zeros((context_len,data.shape[1]))
			padded_data = np.concatenate((pad_top,data),axis=0)
			padded_data = np.concatenate((padded_data,pad_bot),axis=0)

			data = []
			for j in range(context_len,len(padded_data) - context_len):
				new_row = padded_data[j - context_len: j + context_len + 1]
				new_row = new_row.flatten()
				data.append(new_row)
			data = np.array(data)
		if n_deltas > 0:
			pad_top = np.zeros((n_deltas,data.shape[1]))
			pad_bot = np.zeros((n_deltas,data.shape[1]))
			padded_data = np.concatenate((pad_top,data),axis=0)
			padded_data = np.concatenate((padded_data,pad_bot),axis=0)
			data = []
			for j in range(n_deltas,len(padded_data) - n_deltas):
				delta_top = padded_data - padded_data[j - n_deltas:j]
				delta_bot = padded_data - padded_data[j:j + n_deltas]
				new_row = delta_top + padded_data + delta_bot
				data.append(new_row)
		for l in labels:
			state_freq[l] += 1
		filenames.append(data_file)
		frames.append(nFrames)
		if keep_utts:
			allData.append(data)
			allLabels.append(np.array(labels))			
		else:
			allData += list(data)
			allLabels += list(labels)
		pos += nFrames
		if not ctc_labels:
			assert(len(allLabels) == len(allData))
	# print allData
	print len(allData), len(allLabels)
	if max_len == None:
		max_len = 100 * ((max(map(len,X_Train)) + 99)/ 100)
	print 'max_len', max_len
	if keep_utts and pad:
		X_Train = pad_sequences(X_Train,maxlen=max_len,dtype='float32',padding='post')
		Y_Train = pad_sequences(Y_Train,maxlen=max_len,dtype='float32',padding='post',value=n_states)
		Y_Train = Y_Train.reshape(Y_Train.shape[0],Y_Train.shape[1],1)
		X_Dev = pad_sequences(X_Dev,maxlen=max_len,dtype='float32',padding='post')
		Y_Dev = pad_sequences(Y_Dev,maxlen=max_len,dtype='float32',padding='post',value=n_states)
		Y_Dev = Y_Dev.reshape(Y_Dev.shape[0],Y_Dev.shape[1],1)
		X_Test = pad_sequences(X_Test,maxlen=max_len,dtype='float32',padding='post')
		Y_Test = pad_sequences(Y_Test,maxlen=max_len,dtype='float32',padding='post',value=n_states)
		Y_Test = Y_Test.reshape(Y_Test.shape[0],Y_Test.shape[1],1)
	# np.savez('wsj0_phonelabels_NFrames',NFrames_Train=NFrames_Train,NFrames_Test=NFrames_Test)
	# t = threading.Thread(target=ping)
	# t.start()
	if context_len != None:
		np.save('wsj0_phonelabels_bracketed_train.npy',X_Train)
		np.save('wsj0_phonelabels_bracketed_test.npy',X_Test)
		np.save('wsj0_phonelabels_bracketed_dev.npy',X_Dev)
		np.save('wsj0_phonelabels_bracketed_train_labels.npy',Y_Train)
		np.save('wsj0_phonelabels_bracketed_test_labels.npy',Y_Test)
		np.save('wsj0_phonelabels_bracketed_dev_labels.npy',Y_Dev)
		if make_graph:
			np.save('wsj0_phonelabels_bracketed_train_active.npy',active_states_Train)
			np.save('wsj0_phonelabels_bracketed_test_active.npy',active_states_Test)
			np.save('wsj0_phonelabels_bracketed_dev_active.npy',active_states_Dev)
		np.savez('wsj0_phonelabels_bracketed_meta.npz',framePos_Train=framePos_Train,
														framePos_Test=framePos_Test,
														framePos_Dev=framePos_Dev,
														filenames_Train=filenames_Train,
														filenames_Dev=filenames_Dev,
														filenames_Test=filenames_Test,
														state_freq_Train=state_freq_Train,
														state_freq_Dev=state_freq_Dev,
														state_freq_Test=state_freq_Test)
	else:	
		np.save(outfile_prefix + 'train.npy',X_Train)
		np.save(outfile_prefix + 'dev.npy',X_Dev)
		np.save(outfile_prefix + 'train_labels.npy',Y_Train)		
		np.save(outfile_prefix + 'dev_labels.npy',Y_Dev)
		if len(X_Test) != 0:
			np.save(outfile_prefix + 'test.npy',X_Test)
			np.save(outfile_prefix + 'test_labels.npy',Y_Test)
		if make_graph:
			np.save(outfile_prefix + 'active.npy',active_states_Train)
			np.save(outfile_prefix + 'active.npy',active_states_Test)
			np.save(outfile_prefix + 'active.npy',active_states_Dev)
		np.savez(outfile_prefix + 'meta.npz',framePos_Train=framePos_Train,
														framePos_Test=framePos_Test,
														framePos_Dev=framePos_Dev,
														filenames_Train=filenames_Train,
														filenames_Dev=filenames_Dev,
														filenames_Test=filenames_Test,
														state_freq_Train=state_freq_Train,
														state_freq_Dev=state_freq_Dev,
														state_freq_Test=state_freq_Test)
	# done = 1

def normalizeByUtterance():
	data = np.load('wsj0_phonelabels_bracketed_data.npy')
	nFrames = np.load('wsj0_phonelabels_bracketed_meta.npz')['framePos_Train']
	# print 'calculating frame indices...'
	# nFrames = map(lambda i: sum(nFrames[:i]),xrange(len(nFrames)))
	print 'normalizing...'
	scaler = StandardScaler(copy=False)
	print data
	pos = 0
	for i in xrange(len(nFrames)):
		sys.stdout.write("\rnormalizing utterance no %d " % i)
		sys.stdout.flush()
		data[pos:nFrames[i]] = scaler.fit_transform(data[pos:nFrames[i]])
		pos = nFrames[i]
	print data
if __name__ == '__main__':
	#print(read_sen_labels_from_mdef('../wsj_all_cd30.mllt_cd_cont_4000/mdef'))
	# frame2state('../wsj/wsj0/statesegdir/40po031e.wv2.flac.stseg.txt', '../wsj_all_cd30.mllt_cd_cont_4000/mdef')
	genDataset('../wsj/wsj0/etc/wsj0_train.fileids','../wsj/wsj0/etc/wsj0_dev.fileids','../wsj/wsj0/etc/wsj0_test.fileids',40,'../wsj/wsj0/feat_cd_mls/','../wsj/wsj0/stateseg_ci_dir/','../en_us.ci_cont/mdef',
				keep_utts=True, context_len=None, cqt=True,
				trans_file='../wsj/wsj0/etc/wsj0.transcription', 
				pDict_file='../wsj/wsj0/etc/cmudict.0.6d.wsj0')
	# normalizeByUtterance()
	# ../wsj/wsj0/feat_mls/11_6_1/wsj0/sd_dt_20/00b/00bo0t0e.wv1.flac.mls 00bo0t0e.wv1.flac.stseg.txt