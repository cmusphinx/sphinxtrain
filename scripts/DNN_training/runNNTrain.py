from argparse import ArgumentParser
import numpy as np

parser = ArgumentParser(description="Train a Keras neural network model.",
							usage='%(prog)s [options] \nUse --help for option list')
parser.add_argument('-train_data',type=str, required=True,
						help="the training data for the neural network as a saved numpy array of 2D numpy arrays")
parser.add_argument('-train_labels',type=str, required=True,
						help="the training labels for the neural network as a saved numpy array of 1D numpy arrays")
parser.add_argument('-val_data',type=str, required=True,
						help="the validation data for the neural network as a saved numpy array of 2D numpy arrays")
parser.add_argument('-val_labels',type=str, required=True,
						help="the validation labels for the neural network as a saved numpy array of 1D numpy arrays")
parser.add_argument('-nn_config',type=str, required=True,
						help='file containing the neural network configuration information (look at sample_mlp.cfg)')
parser.add_argument('-context_win',type=int, required=False, default=5,
						help='number of contextual frames to include from before and after the target frame (defaults to 5)')
parser.add_argument('-cuda_device_id', type=str, required=False, default="",
						help="The CUDA-capable GPU device to use for execution. If none is specified, the code will execute on the CPU. If specifying multiple devices, separate the id's with commas")
parser.add_argument('-pretrain', nargs='?', required=False, default=False, const=True,
						help='Perform layer-wise pretraining of the MLP before starting training. (Use only with dense MLPs)')
parser.add_argument('-keras_model', type=str, required=False,
						help="Keras model to be trained in hd5 format (must be compatible with keras.load_model)")
parser.add_argument('-model_name', type=str, required=True,
						help='Name to be assigned to the output files')
args = vars(parser.parse_args())

import os
CUDA_VISIBLE_DEVICES = args["cuda_device_id"]
os.environ["CUDA_VISIBLE_DEVICES"] = CUDA_VISIBLE_DEVICES
from NNTrain import *
from utils import *
from keras.models import load_model

def read_config(filename):
	with open(filename) as f:
		lines = f.readlines()
		lines = filter(lambda x: x[0] != '#' and len(x) > 2, lines)
	args = {}
	for l in lines:
		split = l.split()
		if split[0] in args:
			args[split[0]].append(split[1:])
		else:
			args[split[0]] = split[1:] if len(split) > 1 else []
		if len(args[split[0]]) == 1:
			args[split[0]] = args[split[0]][0]
	return args

def init_model(args,input_dim,output_dim, nframes):
	print args
	nn_type = args['type']
	
	if nn_type == 'mlp':
		model = mlp4(input_dim, 
			output_dim, 
			int(args['depth']),
			int(args['width']),
			nframes,
			BN = 'batch_norm' in args,
			dropout = float(args.setdefault('dropout',False)),
			activation = args.setdefault('activation','sigmoid'))
	if nn_type == 'resnet':
		model = mlp4(input_dim,
						output_dim,
						int(args['n_blocks']),
						int(args['width']), 
						nframes,
						block_depth=int(args['block_depth']), 
						dropout=float(args.setdefault('dropout',False)), 
						BN='batch_norm' in args,  
						shortcut=True)
	if nn_type == 'conv' or nn_type == 'conv+resnet':
		print args
		assert(len(args['conv']) == len(args['pooling']) or 
				(type(args['conv']) == str and 
				type(args['pooling'] == str)))
		filts = []
		filt_dims = []
		pooling = []
		max='max'
		avg='avg'
		if type(args['conv']) == str:
			conv = [args['conv']]
			pool = [args['pooling']]
		else:
			conv = args['conv']
			pool = args['pooling']
		for i in range(len(conv)):
			filt, dims = eval(conv[i])
			filts.append(int(filt))
			filt_dims.append(dims)

			pooling.append(eval(pool[i]))
		if nn_type == 'conv':
			model = mlp4(input_dim,
							output_dim,
							int(args['depth']),
							int(args['width']),
							nframes,
							n_filts=filts,
							filt_dims=filt_dims,
							pooling=pooling,
							conv=True)
		else:
			model = mlp4(input_dim,
							output_dim,
							int(args['depth']),
							int(args['width']),
							nframes,
							block_depth=int(args['block_depth']),
							n_filts=filts,
							filt_dims=filt_dims,
							pooling=pooling,
							conv=True,
							shortcut=True)

	if 'ctc_loss' in args:
		model = ctc_model(model)
	return model

x_train = np.load(args['train_data'])
y_train = np.load(args['train_labels'])

x_test = np.load(args['val_data'])
y_test = np.load(args['val_labels'])

nClasses = max(map(np.max,y_train)) + 1

meta = {}
meta['nFrames_Train'] = sum(map(lambda x: x.shape[0], x_train))
meta['nFrames_Dev'] = sum(map(lambda x: x.shape[0], x_test))
meta['state_freq_train'] = np.zeros(nClasses)
for u in y_train:
	for r in u:
		meta['state_freq_train'][r] += 1

# model = mlp1(x_train[0].shape[-1] * 11, nClasses,3,2048,BN=True,regularize=False,lin_boost=False)
conf = read_config(args['nn_config'])
context_len = args['context_win']
if args['keras_model'] != None:
	model = load_model(args['keras_model'])
else:
	if 'ctc_loss' in conf:
		model = init_model(conf, (None,x_train[0].shape[-1],),
					nClasses + 1, 2 * context_len + 1)
		fg = gen_bracketed_data(for_CTC=True, n_states=nClasses)
		# trainNtest(model,x_train,y_train,x_test,y_test,meta,args['model_name'],batch_size=2,ctc_train=False,fit_generator=fg, pretrain=args['pretrain'])
	else:
		model = init_model(conf, (x_train[0].shape[-1] * (2 * context_len + 1),),
					nClasses, 2 * context_len + 1)
		fg = gen_bracketed_data(context_len=context_len)
trainNtest(model,x_train,y_train,x_test,y_test,meta,args['model_name'],batch_size=int(conf['batch_size']),ctc_train=False,fit_generator=fg, pretrain=args['pretrain'])

# parser.add_argument('ctldir', metavar='-ctldir',type=str, nargs=1,
# 						help="the directory for the control files, prepended to each file path in ctllist")
# parser.add_argument('ctllist', metavar='-ctllist',type=str,nargs=1,
# 						help='list of input files, each representing an utterance')
# parser.add_argument('inext', metavar='-inext', type=str, nargs=1,
# 						help='the extension of the control files, appended to each file path in ctllist')
# parser.add_argument('infmt', metavar='-infmt', type=str, nargs=1,
# 						default='yes', choices=['mswav','mfc'],
# 						help='format of the files in the ctllist')
# parser.add_argument('nfilts', metavar='-nfilts',type=int, nargs=1,
# 						required=False, help="number of features in the input files. Only used if infmt is mfc")
# parser.add_argument('stsegdir', metavar='-stsegdir',type=str,nargs=1,
# 						help='the directory in which forced alignments are stored')
# parser.add