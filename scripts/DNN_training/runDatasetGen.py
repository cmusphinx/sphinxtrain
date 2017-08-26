from argparse import ArgumentParser

parser = ArgumentParser(description="Generate numpy array from features and alignments produced by Sphinx",
							usage='%(prog)s [options] \nUse --help for option list')
parser.add_argument('-train_fileids', type=str, required=True,
						help="list of training files")
parser.add_argument('-val_fileids',type=str,required=True,
						help='list of validation files')
parser.add_argument('-test_fileids',type=str,required=False,
						help='list of test files')
parser.add_argument('-nfilts',type=int,required=True,
						help='number of filters used for extracting features. (The dimensionality of the feature vector)')
parser.add_argument('-feat_dir', type=str, required=True,
						help='the directory where feature files are stored (prepended to filepaths in the train, val and test filelists when looking for features)')
parser.add_argument('-feat_ext',type=str,required=True,
						help='extension to be appended to each file path when looking for feature files')
parser.add_argument('-stseg_dir',type=str,required=True,
						help='directory where the state-segmentation for each feature file is stored (prepended to filepaths in the train, val and test filelists when looking for labels)')
parser.add_argument('-stseg_ext',type=str,required=True,
						help='extension to be appended to each file path when looking for state-segmentation files')
parser.add_argument('-mdef',type=str,required=True,
						help='path to the mdef file for the Sphinx model. Needed to map phones/triphones in segmentation to state labels')
parser.add_argument('-outfile_prefix',type=str,default="", required=False,
						help='prepended to the names of the output files')
parser.add_argument('-keep_utts',nargs='?',required=False, default=False, const=True,
						help='store features and labels in a 3D array in which each index points to the list of features/labels for one utterance')
args = vars(parser.parse_args())

from genLabels import genDataset
genDataset(args['train_fileids'],
			args['val_fileids'],
			args['test_fileids'],
			args['nfilts'],
			args['feat_dir'],
			args['feat_ext'],
			args['stseg_dir'],
			args['stseg_ext'],
			args['mdef'],
			args['outfile_prefix'],
			keep_utts=args['keep_utts'])

