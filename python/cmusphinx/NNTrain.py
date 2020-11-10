from keras.models import Sequential, Model
from keras.optimizers import SGD,Adagrad, Adam
from keras.layers.normalization import BatchNormalization
from keras.layers import (
	Input, 
	Dense,
	Activation, 
	Dropout, 
	Conv1D, 
	Conv2D,
	LocallyConnected2D, 
	MaxPooling2D,
	AveragePooling2D, 
	Reshape, 
	Flatten,
	Masking)
from keras.layers.core import Lambda
from keras.layers.merge import add, concatenate
from keras.utils import to_categorical, plot_model
from keras.models import load_model, Model
from keras.callbacks import History,ModelCheckpoint,CSVLogger,ReduceLROnPlateau
from keras import regularizers
from keras.preprocessing.sequence import pad_sequences
import keras.backend as K
import numpy as np
import matplotlib.pyplot as plt
import tensorflow as tf
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import array_ops
from tfrbm import GBRBM,BBRBM
from sys import stdout
from sklearn.preprocessing import StandardScaler
import struct
from utils import *

"""
	This module provides functions for training different neural network architectures
	Below is a bried summary of the different functions and their purpose, more detail 
	can be found below:
		- mlp1: creates a MLP consisting of a number dense layers
		- mlp_wCTC: creates a MLP that calculates the ctc loss during training
		- mlp4: create convolutional neural networks and residual networks
		- DBN_DNN: performs DBN pretraining on simple MLPs
		- preTrain: performs layer-wise pretraining on simple MLPs
		- trainAndTest: runs training on a provided model using the given dataset
"""

def ctc_lambda_func(args):
	import tensorflow as tf
	y_pred, labels, input_length, label_length = args
	label_length = K.cast(tf.squeeze(label_length), 'int32')
	input_length = K.cast(tf.squeeze(input_length), 'int32')
    # return K.ctc_batch_cost(labels, y_pred, input_length, label_length)
	labels = K.ctc_label_dense_to_sparse(labels,label_length)
	return tf.nn.ctc_loss(labels,y_pred,input_length,
    				preprocess_collapse_repeated=True,
    				ctc_merge_repeated=False,
    				time_major=False,
    				ignore_longer_outputs_than_inputs=True)

def ler(y_true, y_pred, **kwargs):
    """
        Label Error Rate. For more information see 'tf.edit_distance'
    """
    return tf.reduce_mean(tf.edit_distance(y_pred, y_true, **kwargs))

def decode_output_shape(inputs_shape):
    y_pred_shape, seq_len_shape = inputs_shape
    return (y_pred_shape[:1], None)

def decode(args):
	import tensorflow as tf
	y_pred, label_len = args
	label_len = K.cast(tf.squeeze(label_len), 'int32')
	# ctc_labels = tf.nn.ctc_greedy_decoder(y_pred, label_len)[0][0]
	# return ctc_labels
	ctc_labels = K.ctc_decode(y_pred,label_len,greedy=False)[0][0]
	return K.ctc_label_dense_to_sparse(ctc_labels, label_len)

def mlp_wCTC(input_dim,output_dim,depth,width,BN=False):
	print locals()
	x = Input(name='x', shape=(1000,input_dim))
	h = x
	h = Masking()(h)
	for i in range(depth):
		h = Dense(width)(h)
		if BN:
			h = BatchNormalization()(h)
		h = Activation('sigmoid')(h)
	out = Dense(output_dim,name='out')(h)
	softmax = Activation('softmax', name='softmax')(out)
	# a = 1.0507 * 1.67326
	# b = -1
	# # out = Lambda(lambda x : a * K.pow(x,3) + b)(h)
	# out = Lambda(lambda x: a * K.exp(x) + b, name='out')(h)
	y = Input(name='y',shape=[None,],dtype='int32')
	x_len = Input(name='x_len', shape=[1],dtype='int32')
	y_len = Input(name='y_len', shape=[1],dtype='int32')

	dec = Lambda(decode, output_shape=decode_output_shape, name='decoder')([out,x_len])
	# edit_distance = Lambda(ler, output_shape=(1,), name='edit_distance')([out,y,x_len,y_len])

	loss_out = Lambda(ctc_lambda_func, output_shape=(1,), name='ctc')([out, y, x_len, y_len])
	model = Model(inputs=[x, y, x_len, y_len], outputs=[loss_out,dec,softmax])

	sgd = SGD(lr=0.001, decay=1e-6, momentum=0.99, nesterov=True, clipnorm=5)
	opt = Adam(lr=0.0001, clipnorm=5)
	model.compile(loss={'ctc': dummy_loss,
						'decoder': decoder_dummy_loss,
						'softmax': 'sparse_categorical_crossentropy'},
					optimizer=opt,
					metrics={'decoder': ler,
							 'softmax': 'accuracy'},
					loss_weights=[1,0,0])
	return model
def ctc_model(model):
	x = model.get_layer(name='x').input

	out = model.get_layer(name='out').output
	softmax = Activation('softmax', name='softmax')(out)
	y = Input(name='y',shape=[None,],dtype='int32')
	x_len = Input(name='x_len', shape=[1],dtype='int32')
	y_len = Input(name='y_len', shape=[1],dtype='int32')

	dec = Lambda(decode, output_shape=decode_output_shape, name='decoder')([out,x_len])
	# edit_distance = Lambda(ler, output_shape=(1,), name='edit_distance')([out,y,x_len,y_len])

	loss_out = Lambda(ctc_lambda_func, output_shape=(1,), name='ctc')([out, y, x_len, y_len])
	model = Model(inputs=[x, y, x_len, y_len], outputs=[loss_out,dec,softmax])

	sgd = SGD(lr=0.001, decay=1e-6, momentum=0.99, nesterov=True, clipnorm=5)
	opt = Adam(lr=0.0001, clipnorm=5)
	model.compile(loss={'ctc': dummy_loss,
						'decoder': decoder_dummy_loss,
						'softmax': 'sparse_categorical_crossentropy'},
					optimizer=opt,
					metrics={'decoder': ler,
							 'softmax': 'accuracy'},
					loss_weights=[1,0,0])
	return model
def _bn_act(input,activation='relu'):
    """Helper to build a BN -> relu block
    """
    norm = BatchNormalization()(input)
    return Activation(activation)(norm)

def make_dense_res_block(inp, size, width, drop=False,BN=False,regularize=False):
	x = inp
	for i in range(size):
		x = Dense(width,
					kernel_regularizer=regularizers.l2(0.05) if regularize else None)(x)
		if i < size - 1:
			if drop:
				x = Dropout(0.15)(x)
			if BN:
				x = _bn_relu(x)			
	return x

def mlp4(input_dim,output_dim,nBlocks,width, n_frames, block_depth=1,
			n_filts=[84], filt_dims=[(11,8)], pooling=[['max',(6,6),(2,2)]],  
			block_width=None, dropout=False, BN=False, activation='relu',
			parallelize=False, conv=False, regularize=False,
			exp_boost=False, quad_boost=False, shortcut=False,
			opt='adam', lr=0.001):
	
	print locals()
	if block_width == None:
		block_width = width
	inp = Input(shape=input_dim, name='x')
	x = inp
	if conv:
		x = Reshape((n_frames,input_dim/n_frames,1))(x)
		for i in range(len(n_filts)):
			print i

			x = LocallyConnected2D(n_filts[i],filt_dims[i],
					padding='valid')(x)
			x = _bn_act(x,activation=activation)
			if pooling[i] != None:
				pooling_type, win_size, stride = pooling[i]
				if pooling_type == 'max':
					x = MaxPooling2D(win_size,strides=stride,padding='same')(x)
				if pooling_type == 'avg':
					x = AveragePooling2D(win_size,strides=stride,padding='same')(x)
		x = Flatten()(x)
	if block_width != width:
		x = Dense(block_width)(x)
	for i in range(nBlocks):
		y = make_dense_res_block(x,block_depth,block_width,BN=BN,drop=dropout,regularize=regularize)
		if shortcut:
			x = add([x,y])
		else:
			x = y
		if dropout:
			x = Dropout(dropout)(x)
		if BN:
			x = _bn_act(x,activation=activation)
		else:
			x = Activation(activation)(x)

	if exp_boost:
		x = Dense(output_dim)(x)
		z = Lambda(lambda x : K.exp(x))(x)
	if quad_boost:
		x = Dense(output_dim)(x)
		a = 0.001
		b = 0.4
		z = Lambda(lambda x : a * K.pow(x,3) + b)(x)
	else:
		z = Dense(output_dim, name='out')(x)
		z = Activation('softmax', name='softmax')(z)
	model = Model(inputs=inp, outputs=z)
	if parallelize:
		model = make_parallel(model, len(CUDA_VISIBLE_DEVICES.split(',')))
	# opt = Adam(lr=25/(np.sqrt(width * output_dim)))
	if opt == 'sgd':
		opt = SGD
	if opt == 'adam':
		opt = Adam
	if opt == 'adagrad':
		opt = Adagrad
	opt = opt(lr=lr)
	# opt = SGD(lr=1/(np.sqrt(input_dim * width)), decay=1e-6, momentum=0.9, nesterov=True)
	model.compile(optimizer=opt,
	              loss='sparse_categorical_crossentropy',
	              metrics=['accuracy'])
	return model

def resnet_wrapper(input_dim,output_dim,depth,width,reshape_layer):
	builder = resnet.ResnetBuilder()
	model = builder.build_resnet_18(input_dim, output_dim,reshape_layer)
	x = model.get_layer(name='flatten_1').get_output_at(-1)
	for i in range(depth):
		x = Dense(width,activation='relu')(x)
	softmax = Dense(output_dim,activation='softmax')(x)
	model = Model(inputs=model.inputs, outputs=softmax)
	opt = Adam(lr=10/np.sqrt(input_dim * output_dim))
	# opt = SGD(lr=1/(np.sqrt(input_dim * width)), decay=1e-6, momentum=0.9, nesterov=True)
	model.compile(optimizer=opt,
	              loss='sparse_categorical_crossentropy',
	              metrics=['accuracy'])
	return model
def DBN_DNN(inp,nClasses,depth,width,batch_size=2048):
	RBMs = []
	weights = []
	bias = []
	# batch_size = inp.shape
	nEpoches = 5
	if len(inp.shape) == 3:
		inp = inp.reshape((inp.shape[0] * inp.shape[1],inp.shape[2]))
	sigma = np.std(inp)
	# sigma = 1
	rbm = GBRBM(n_visible=inp.shape[-1],n_hidden=width,learning_rate=0.002, momentum=0.90, use_tqdm=True,sample_visible=True,sigma=sigma)
	rbm.fit(inp,n_epoches=15,batch_size=batch_size,shuffle=True)
	RBMs.append(rbm)
	for i in range(depth - 1):
		print 'training DBN layer', i
		rbm = BBRBM(n_visible=width,n_hidden=width,learning_rate=0.02, momentum=0.90, use_tqdm=True)
		for e in range(nEpoches):
			batch_size *= 1 + (e*0.5)
			n_batches = (inp.shape[-2] / batch_size) + (1 if inp.shape[-2]%batch_size != 0 else 0)
			for j in range(n_batches):
				stdout.write("\r%d batch no %d/%d epoch no %d/%d" % (int(time.time()),j+1,n_batches,e,nEpoches))
				stdout.flush()
				b = np.array(inp[j*batch_size:min((j+1)*batch_size, inp.shape[0])])
				for r in RBMs:
					b = r.transform(b)
				rbm.partial_fit(b)
		RBMs.append(rbm)
	for r in RBMs:
		(W,_,Bh) = r.get_weights()
		weights.append(W)
		bias.append(Bh)
	model = mlp1(x_train.shape[1],nClasses,depth-1,width)
	print len(weights), len(model.layers)
	assert len(weights) == len(model.layers) - 1
	for i in range(len(weights)):
		W = [weights[i],bias[i]]
		model.layers[i].set_weights(W)
	return model
# def gen_data(active):
		

def preTrain(model,modelName,x_train,y_train,meta,skip_layers=[],outEqIn=False,fit_generator=None):
	print model.summary()
	layers = model.layers
	output = layers[-1]
	outdim = output.output_shape[-1]
	for i in range(len(layers) - 1):
		if i in skip_layers:
			print 'skipping layer ',i
			continue
		if len(model.layers[i].get_weights()) == 0:
			print 'skipping layer ',i
			continue
		last = model.layers[i].get_output_at(-1)
		if outEqIn:
			preds = Dense(outdim)(last)
		else:
			preds = Dense(outdim,activation='softmax')(last)
		model_new = Model(model.input,preds)
		for j in range(len(model_new.layers) - 2):
			print "untrainable layer ",j
			model_new.layers[j].trainable=False
		model_new.compile(optimizer='adam',
	              loss='sparse_categorical_crossentropy',
	              metrics=['accuracy'])
		print model_new.summary()
		batch_size = 2048
		if fit_generator == None:
			model_new.fit(x_train,y_train,epochs=1,batch_size=2048)
		else:
			history = model.fit_generator(fit_generator(x_train,y_train,batch_size),
											steps_per_epoch=(meta['nFrames_Train'])/batch_size, epochs=3)
		# model.fit_generator(gen_bracketed_data(x_train,y_train,meta['framePos_Train'],4),
		# 								steps_per_epoch=len(meta['framePos_Train']), epochs=3,
		# 								callbacks=[ModelCheckpoint('%s_CP.h5' % modelName,monitor='loss',mode='min')])
		# model.fit_generator(gen_data(x_train,y_train,batch_size),
		# 					steps_per_epoch = x_train.shape[0] / batch_size,
		# 					epochs = 1)
		model.layers[i].set_weights(model_new.layers[-2].get_weights())
	for l in model.layers:
		l.trainable = True
	return model

def trainNtest(model,x_train,y_train,x_test,y_test,meta,
				modelName,testOnly=False,pretrain=False, batch_size=512,
				init_epoch=0, fit_generator=None, ctc_train=False, n_epochs=50):
	print 'TRAINING MODEL:',modelName
	if not testOnly:
		if pretrain:
			print 'pretraining model...'
			model = preTrain(model,modelName,x_train,y_train,meta,fit_generator=fit_generator)
		if ctc_train:
			model = ctc_model(model)
		print model.summary()
		print 'starting fit...'
		callback_arr = [ModelCheckpoint('%s_CP.h5' % modelName,save_best_only=True,verbose=1),
						ReduceLROnPlateau(patience=5,factor=0.5,min_lr=10**(-6), verbose=1),
						CSVLogger(modelName+'.csv',append=True)]
		
		if fit_generator == None:
			history = model.fit(x_train,y_train,epochs=n_epochs,batch_size=batch_size,
								initial_epoch=init_epoch,
								validation_data=(x_test,y_test),
								callbacks=callback_arr)
		else:
			history = model.fit_generator(fit_generator(x_train,y_train,batch_size),
											steps_per_epoch= (meta['nFrames_Train'])/batch_size, epochs=n_epochs,
											validation_data=fit_generator(x_test,y_test,batch_size),
											validation_steps = (meta['nFrames_Dev']) / batch_size,
											callbacks=callback_arr)
		model = Model(inputs=[model.get_layer(name='x').input],
						outputs=[model.get_layer(name='softmax').output])
		print model.summary()
		model.compile(loss='sparse_categorical_crossentropy',
						optimizer='adam',
						metrics=['accuracy'])
		print 'saving model...'
		model.save(modelName+'.h5')
		# model.save_weights(modelName+'_W.h5')
		print(history.history.keys())
		print history.history['lr']
		print 'plotting graphs...'
		# summarize history for accuracy
		fig, ax1 = plt.subplots()
		ax1.plot(history.history['acc'])
		ax1.plot(history.history['val_acc'])
		ax2 = ax1.twinx()
		ax2.plot(history.history['loss'],color='r')
		ax2.plot(history.history['val_loss'],color='g')
		plt.title('model loss & accuracy')
		ax1.set_ylabel('accuracy')
		ax2.set_ylabel('loss')
		ax1.set_xlabel('epoch')
		ax1.legend(['training acc', 'testing acc'])
		ax2.legend(['training loss', 'testing loss'])
		fig.tight_layout()
		plt.savefig(modelName+'.png')
		plt.clf()
	else:
		model = load_model(modelName)
		print 'scoring...'
		score = model.evaluate_generator(gen_bracketed_data(x_test,y_test,meta['framePos_Dev'],4),
										len(meta['framePos_Dev']))
		print score
