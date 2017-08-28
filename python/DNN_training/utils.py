import numpy as np
import struct
import matplotlib.pyplot as plt
import pylab as pl
from sys import stdout
import os
from keras.preprocessing.sequence import pad_sequences
import keras.backend as K
from scipy.sparse import coo_matrix
from sklearn.preprocessing import StandardScaler

def dummy_loss(y_true,y_pred):
	return y_pred
def decoder_dummy_loss(y_true,y_pred):
	return K.zeros((1,))
def ler(y_true, y_pred, **kwargs):
    """
        Label Error Rate. For more information see 'tf.edit_distance'
    """
    return tf.reduce_mean(tf.edit_distance(y_pred, y_true, **kwargs))

def dense2sparse(a):
	# rows,cols = a.nonzero()
	# data = map(lambda i: a[rows[i],cols[i]], range(rows.shape[0]))
	# return coo_matrix((data,(rows,cols)), shape=a.shape,dtype='int32')
	return coo_matrix(a,shape=a.shape,dtype='int32')
	
def readMFC(fname,nFeats):
	data = []
	with open(fname,'rb') as f:
		v = f.read(4)
		head = struct.unpack('I',v)[0]
		v = f.read(nFeats * 4)
		while v:
			frame = list(struct.unpack('%sf' % nFeats, v))
			data .append(frame)
			v = f.read(nFeats * 4)
	data = np.array(data)
	# print data.shape, head
	assert(data.shape[0] * data.shape[1] == head)
	return data

def ctc_labels(labels, blank_labels = []):
	new_labels = []
	for i in range(len(labels)):
		l_curr = labels[i]
		if l_curr not in blank_labels:
			if i == 0:
				new_labels.append(l_curr)
			else:
				if l_curr != labels[i-1]:
					new_labels.append(l_curr)
	return np.array(new_labels)
def _gen_bracketed_data_2D(x,y,nFrames,
						context_len,fix_length,
						for_CTC):
	max_len = ((np.max(nFrames) + 50)/100) * 100 #rounding off to the nearest 100
	batch_size = 2
	while 1:
		pos = 0
		nClasses = np.max(y) + 1
		if for_CTC:
			alldata = []
			alllabels = []
		for i in xrange(len(nFrames)):
			data = x[pos:pos + nFrames[i]]
			labels = y[pos:pos + nFrames[i]]
			# if for_CTC:
			# 	labels = ctc_labels(labels,blank_labels=range(18) + [108,109,110])
			# if len(labels.shape) == 1:
			# 	labels = to_categorical(labels,num_classes=nClasses)
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
			if for_CTC:
				if batch_size != None:
					alldata.append(data)
					alllabels.append(labels)
				
					if len(alldata) == batch_size:
						alldata = np.array(alldata)
						alllabels = np.array(alllabels)
						if fix_length:
							alldata = pad_sequences(alldata,maxlen=1000,dtype='float32',truncating='post')
							alllabels = pad_sequences(alllabels,maxlen=1000,dtype='float32',value=138,truncating='post')
						inputs = {'x': alldata,
								'y': alllabels,
								'x_len': np.array(map(lambda x: len(x), alldata)),
								'y_len': np.array(map(lambda x: len(x), alllabels))}
						outputs = {'ctc': np.ones([batch_size])}
						yield (inputs,outputs)
						alldata = []
						alllabels = []
				else:
					data = np.array([data])
					labels = np.array([labels])
					inputs = {'x': data,
								'y': labels,
								'x_len': [data.shape[0]],
								'y_len': [labels.shape[0]]}
					outputs = {'ctc': labels}
					yield (inputs,outputs)
			else:
				yield (data,labels)
			pos += nFrames[i]

def _gen_bracketed_data_3D(x,y,batch_size,context_len):
	epoch_no = 1
	while 1:
		print epoch_no
		batch_data = []
		batch_labels = []
		for i in range(len(x)):
			data = x[i]
			labels = y[i]
			if context_len != 0:
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
			seq_len = 0
			while seq_len < len(data) and data[seq_len].any():
				seq_len += 1
			idxs = range(seq_len)
			np.random.shuffle(idxs)
			for j in idxs:
				if len(batch_data) < batch_size:
					batch_data.append(data[j])
					batch_labels.append(labels[j])
				else:
					batch_data = np.array(batch_data)
					batch_labels = np.array(batch_labels)
					yield(batch_data,batch_labels)
					batch_data = []
					batch_labels = []
		epoch_no += 1
def gen_ctc_data(alldata,alllabels,batch_size, n_states):
	while 1:
		for i in range(batch_size,alldata.shape[0]+1,batch_size):
			x = alldata[i-batch_size:i]
			y = alllabels[i-batch_size:i]
				# .reshape(batch_size,alllabels.shape[1])
			max_len = max(map(len,x))
			x = pad_sequences(x,maxlen=max_len,dtype='float32',padding='post')
			y = pad_sequences(y,maxlen=max_len,dtype='float32',padding='post',value=n_states)
			x = np.array(x)
			y = np.array(y)
			# print x.shape, y.shape
			y_len = []
			# print y
			for b in y:
				# # print b[-1], int(b[-1]) != 138
				pad_len = 0
				while pad_len < len(b) and int(b[pad_len]) != 138:
					pad_len += 1
				y_len.append(pad_len)
			y_len = np.array(y_len)
			x_len = []
			for b in x:
				# print b[-1], int(b[-1]) != 138
				pad_len = 0
				while pad_len < len(b) and b[pad_len].any():
					pad_len += 1
				x_len.append(pad_len)
			x_len = np.array(x_len)
			# x_len = np.array(map(lambda x: len(x), x))
			# y_len = np.array(map(lambda x: len(x), y))
			# print x.shape,y.shape,x_len,y_len
			# print y.shape
			# y = dense2sparse(y)
			inputs = {'x': x,
					'y': y,
					'x_len': x_len,
					'y_len': y_len}
			outputs = {'ctc': np.ones([batch_size]),
						'decoder': dense2sparse(y),
						'softmax': y.reshape(y.shape[0],y.shape[1],1)}
			yield(inputs,outputs)

def gen_bracketed_data(context_len=None,fix_length=False,
						for_CTC=False, n_states=None):
	if for_CTC:
		assert(n_states != None)
		return lambda x,y,batch_size: gen_ctc_data(x,y,batch_size,n_states)
	else:
		return lambda x,y,batch_size: _gen_bracketed_data_3D(x,y,batch_size,context_len)
	# return lambda x,y,nf: _gen_bracketed_data(x,y,nf,context_len,fix_length,
	# 					for_CTC)

def plotFromCSV(modelName, loss_cols=[2,5], acc_cols=[1,4]):
	data = np.loadtxt(modelName+'.csv',skiprows=1,delimiter=',')
	epoch = data[:,[0]]
	acc = data[:,[acc_cols[0]]]
	loss = data[:,[loss_cols[0]]]
	val_acc = data[:,[acc_cols[1]]]
	val_loss = data[:,[loss_cols[1]]]

	fig, ax1 = plt.subplots()
	ax1.plot(acc)
	ax1.plot(val_acc)
	ax2 = ax1.twinx()
	ax2.plot(loss,color='r')
	ax2.plot(val_loss,color='g')
	plt.title('model loss & accuracy')
	ax1.set_ylabel('accuracy')
	ax2.set_ylabel('loss')
	ax1.set_xlabel('epoch')
	ax1.legend(['training acc', 'testing acc'])
	ax2.legend(['training loss', 'testing loss'])
	fig.tight_layout()
	plt.savefig(modelName+'.png')
	plt.clf()

def writeSenScores(filename,scores,weight,offset):
	n_active = scores.shape[1]
	s = ''
	s = """s3
version 0.1
mdef_file ../../en_us.cd_cont_4000/mdef
n_sen 138
logbase 1.000100
endhdr
"""
	s += struct.pack('I',0x11223344)

	scores = np.log(scores)/np.log(1.0001)
	scores *= -1
	scores -= np.min(scores,axis=1).reshape(-1,1)
	# scores = scores.astype(int)
	scores *= weight
	scores += offset
	truncateToShort = lambda x: 32676 if x > 32767 else (-32768 if x < -32768 else x)
	vf = np.vectorize(truncateToShort)
	scores = vf(scores)
	# scores /= np.sum(scores,axis=0)
	for r in scores:
		# print np.argmin(r)
		s += struct.pack('h',n_active)
		r_str = struct.pack('%sh' % len(r), *r)
		# r_str = reduce(lambda x,y: x+y,r_str)
		s += r_str
	with open(filename,'w') as f:
		f.write(s)

def getPredsFromArray(model,data,nFrames,filenames,res_dir,res_ext,freqs,preds_in=False,weight=0.1,offset=0):
	if preds_in:
		preds = data
	else:
		preds = model.predict(data,verbose=1,batch_size=2048)
	pos = 0
	for i in range(len(nFrames)):
		fname = filenames[i][:-4]
		fname = reduce(lambda x,y: x+'/'+y,fname.split('/')[4:])
		stdout.write("\r%d/%d 	" % (i,len(filenames)))
		stdout.flush()
		res_file_path = res_dir+fname+res_ext
		dirname = os.path.dirname(res_file_path)
		if not os.path.exists(dirname):
			os.makedirs(dirname)
		# preds = model.predict(data[pos:pos+nFrames[i]],batch_size=nFrames[i])
		writeSenScores(res_file_path,preds[pos:pos+nFrames[i]],freqs,weight,offset)
		pos += nFrames[i]

def getPredsFromFilelist(model,filelist,file_dir,file_ext,
							res_dir,res_ext,n_feat=40,context_len=None,
							weight=1,offset=0, data_preproc_fn=None,
							data_postproc_fn=None):
	with open(filelist) as f:
		files = f.readlines()
		files = map(lambda x: x.strip(),files)
	filepaths = map(lambda x: file_dir+x+file_ext,files)
	scaler = StandardScaler(copy=False,with_std=False)
	for i in range(len(filepaths)):
		stdout.write("\r%d/%d 	" % (i,len(filepaths)))
		stdout.flush()

		f = filepaths[i]
		if not os.path.exists(f):
			print ("\n",f)
			continue
		data = readMFC(f,n_feat)
		data = scaler.fit_transform(data)

		if context_len != None:
			pad_top = np.zeros((context_len,data.shape[1])) + data[0]
			pad_bot = np.zeros((context_len,data.shape[1])) + data[-1]
			padded_data = np.concatenate((pad_top,data),axis=0)
			padded_data = np.concatenate((padded_data,pad_bot),axis=0)

			data = []
			for j in range(context_len,len(padded_data) - context_len):
				new_row = padded_data[j - context_len: j + context_len + 1]
				new_row = new_row.flatten()
				data.append(new_row)
		data = np.array(data)
		if data_preproc_fn != None:
			_data = data_preproc_fn(data)
			preds = model.predict(_data)
			preds = np.squeeze(preds)
		else:
			preds = model.predict(data)
	
		if data_postproc_fn != None:
			preds = data_postproc_fn(preds)
		if preds.shape[0] != data.shape[0]:
			preds = preds[:data.shape[0]]
			# print np.sum(preds)
		# print preds.shape
		res_file_path = res_dir+files[i]+res_ext
		dirname = os.path.dirname(res_file_path)
		if not os.path.exists(dirname):
			os.makedirs(dirname)
		writeSenScores(res_file_path,preds,weight,offset)

# a = dense2sparse(np.array([[1,2,3],[4,0,6]]))
# print a.shape
# print np.asarray(a,dtype=long)