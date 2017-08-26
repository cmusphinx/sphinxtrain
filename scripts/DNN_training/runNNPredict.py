from argparse import ArgumentParser

parser = ArgumentParser(description="Generate Predictions from a Keras Model and save them in PockeSphinx readable .sen files.",
							usage='%(prog)s [options] \nUse --help for option list')
parser.add_argument('-keras_model', type=str, required=True,
						help="Keras model to be used for prediction in hd5 format (must be compatible with keras.load_model)")
parser.add_argument('-ctldir',type=str, required=True,
						help="the directory for the control files, prepended to each file path in ctllist")
parser.add_argument('-ctllist', type=str,required=True,
						help='list of input files, each representing an utterance')
parser.add_argument('-inext', type=str, required=True,
						help='the extension of the control files, appended to each file path in ctllist')
parser.add_argument('-outdir', type=str,required=True,
						help='Directory where the predictions are stored. The structure of this directory will be identical to ctldir')
parser.add_argument('-outext', type=str, required=True,
						help='the extension of the output files')
parser.add_argument('-nfilts', type=int, required=True,
						help='dimensionality of the feature vectors')
parser.add_argument('-acoustic_weight',type=float,required=False,
						help='The weight to scale the predictions by. Sometimes needed to get meaningful predictions from PocketSphinx')
parser.add_argument('-context_win',type=int, required=False, default=0,
						help='number of contextual frames to include from before and after the target frame (defaults to 5)')
parser.add_argument('-cuda_device_id', type=str, required=False, default="",
						help="The CUDA-capable GPU device to use for execution. If none is specified, the code will execute on the CPU. If specifying multiple devices, separate the id's with commas")
args = vars(parser.parse_args())

import os
CUDA_VISIBLE_DEVICES = args["cuda_device_id"]
os.environ["CUDA_VISIBLE_DEVICES"] = CUDA_VISIBLE_DEVICES
from keras.models import load_model
from utils import *

model = load_model(keras_model,custom_objects={'dummy_loss':dummy_loss, 
													'decoder_dummy_loss':decoder_dummy_loss,
													'ler':ler})
# model = Model(inputs=[model.get_layer(name='x').input],
# 					outputs=[model.get_layer(name='softmax').output])
print model.summary()
getPredsFromFilelist(model,args['ctllist'],
						args['ctldir'],
						args['inext'],
						args['outdir'],
						args['outext'],
						context_len=args['context_win'], 
						# data_preproc_fn = lambda x: pad_sequences([x],maxlen=1000,dtype='float32',padding='post').reshape(1,1000,x.shape[-1]),
						# data_postproc_fn = lambda x: x[:,range(138)] / np.sum(x[:,range(138)], axis=1).reshape(-1,1),
						weight=args['acoustic_weight'] if args['acoustic_weight'] != None else 0.1,
						n_feat=args['nfilts'])