# Model creation mostly guided by: https://github.com/tensorflow/docs/blob/master/site/en/r1/tutorials/sequences/recurrent_quickdraw.md

import numpy as np
import torch
import torch.nn.functional as F
from torch.utils.data import Dataset, DataLoader
from torch.utils.tensorboard import SummaryWriter
import json
from datetime import datetime
import functools
import sklearn as sks
from os import listdir
from os.path import isfile, join
import argparse


# CONSTANTS
batch_size = 8
eval_batch_size = 8
dropout_rate = 0.3
num_layers = 3
num_nodes = 128

class QuickDrawDataset(Dataset):
    """Quick, Draw! data subset."""

    def __init__(self, data, classes, train):
        """
        Arguments:
            data (list): List of all the parsed data with the readData() function.
            classes (dict): Dictionary with all the classes and how many of each there are.
            train (bool): Says if the dataset is used for training or testing.
        """
        self.data = data
        self.classes = classes
        self.train = train

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        return self.data[idx]

def quickDrawCollateFn(batch, batch_size):
    shapes = [sample["shape"] for sample in batch]
    maxLen = max([shape[0] for shape in shapes])

    ## Makes a dictionary of lists
    newBatch = {
        "ink": torch.zeros((batch_size, maxLen, 3)),
        "shape": torch.zeros((batch_size, 2), dtype=int),
        "length": torch.zeros((batch_size), dtype=int),
        "className": [],
        "classIndex": torch.zeros((batch_size), dtype=int),
        "maxLen": maxLen
    }
    for i, sample in enumerate(batch):
        newBatch["className"].append(sample["className"])
        newBatch["classIndex"][i] = sample["classIndex"]
        newBatch["shape"][i] = torch.FloatTensor(list(sample["shape"]))
        newBatch["length"][i] = sample["shape"][0]

        # Makes a copy of the tensor
        newInk = F.pad(sample["ink"], (0, 0, 0, maxLen - sample["shape"][0]))
        newBatch["ink"][i] = newInk
    
    return newBatch

# Thanks to: https://discuss.pytorch.org/t/pytorch-equivalent-for-tf-sequence-mask/39036/2
def sequence_mask(lengths, maxlen = None, dtype=torch.bool):
    if maxlen is None:
        maxlen = lengths.max()
    mask = ~(torch.ones((len(lengths), maxlen)).cumsum(dim=1).t() > lengths).t()
    mask.type(dtype)
    return mask

class QuickDrawRNN(torch.nn.Module):

    def __init__(self, classes):
        super(QuickDrawRNN, self).__init__()

        # Init data
        classCnt = len(classes)

        # 3x 1D Convolutions
        
        # Filters: [48, 64, 96]
        # Length of convolutional filters: [5, 5, 3]
        self.conv = torch.nn.Sequential(
            torch.nn.Conv1d( 3, 48, 5, stride=1, padding=2),  # Should we disable bias?
            torch.nn.Dropout(p=dropout_rate),
            torch.nn.Conv1d(48, 64, 5, stride=1, padding=2),
            torch.nn.Dropout(p=dropout_rate),
            torch.nn.Conv1d(64, 96, 3, stride=1, padding=1)
        )
        
        # Bidirectional LSTM

        # Num layers: num_layers (3)
        # Num nodes: num_nodes (128)
        # Dropout = dropout_rate if TRAIN else 0
        # Direction = bidirectional
        self.lstm = torch.nn.LSTM(
            96, 
            num_nodes, 
            num_layers=num_layers, 
            bias=True,    # Should this be false?
            batch_first=True, 
            dropout=dropout_rate, 
            bidirectional=True
        )

        # Fully Connected

        # Input: 2 * num_nodes (256)
        # Output: Number of classes
        print("Classes:", classes)
        print("Class count:", classCnt)
        
        self.fc = torch.nn.Linear(num_nodes * 2, classCnt)
        

    def forward(self, inks, lengths):
        
        # conv
        inks = self.conv(inks.permute(0, 2, 1))
        
        # permute inks back
        inks = inks.permute(0, 2, 1)

        # Inks should now be of shape: (B, L, convFilters[3] (default 96))
        
        # lstm
        inks, _ = self.lstm(inks)

        # Inks should now be of shape: (B, L, 2 * num_nodes (default 2 * 128, the 'times 2' is because bidir LSTM doubles the features/nodes))

        # mask to remove the data past the initial length of each drawing
        mask = torch.tile(
            torch.unsqueeze(sequence_mask(lengths, inks.shape[1]), 2),
            (1, 1, inks.shape[2])
        )

        inks_maked = torch.where(mask, inks, torch.zeros_like(inks))

        inks = torch.sum(inks_maked, dim=1)

        # Inks should now be of shape: (B, 2 * num_nodes)

        # fc
        inks = self.fc(inks)

        # Inks should now be of shape: (B, num_classes)

        return inks



def set_cuda_as_primary():
    if torch.cuda.is_available():
        print("Full power!")
        dev = torch.set_default_device("cuda")
    else:
        print("Regular power..")
        dev = torch.set_default_device("cpu")

    print(torch.get_default_device())

def get_classes(classesFilePath):
    classes = []
    classToIndex = {}
    with open(classesFilePath) as file:
        for i, line in enumerate(file):
            cls = line.strip()
            classes.append(cls)
            classToIndex[cls] = i

    return classes, classToIndex

def parseLine(ndjsonLine):
    """Parse an ndjson line and return ink (as np array) and classname."""
    sample = json.loads(ndjsonLine)
    class_name = sample["word"]
    inkarray = sample["drawing"]
    stroke_lengths = [len(stroke[0]) for stroke in inkarray]
    total_points = sum(stroke_lengths)
    np_ink = np.zeros((total_points, 3), dtype=np.float32)
    current_t = 0
    for stroke in inkarray:
        for i in [0, 1]:
            np_ink[current_t:(current_t + len(stroke[0])), i] = stroke[i]
        current_t += len(stroke[0])
        np_ink[current_t - 1, 2] = 1  # stroke_end
      
    # Preprocessing.
    # 1. Size normalization.
    lower = np.min(np_ink[:, 0:2], axis=0)
    upper = np.max(np_ink[:, 0:2], axis=0)
    scale = upper - lower
    scale[scale == 0] = 1
    np_ink[:, 0:2] = (np_ink[:, 0:2] - lower) / scale
    
    # 2. Compute deltas.
    np_ink[1:, 0:2] -= np_ink[0:-1, 0:2]
    np_ink = np_ink[1:, :]

    return torch.from_numpy(np_ink), class_name

def readData(filePath, classToIndex):
    samples = []
    with open(filePath) as file:
        for line in file:
            features = {}
            features["ink"], features["className"] = parseLine(line)

            # Define the shape of the ink
            features["shape"] = features["ink"].shape

            features["classIndex"] = classToIndex[features["className"]]

            samples.append(features)

    return samples

if __name__ == "__main__":
    # Use the GPU instead of the CPU for PyTorch
    set_cuda_as_primary()

    # Define the classes
    model_root_path = "models/model_20250411_222609_1"
    classes, classToIndex = get_classes(model_root_path + "_classes")

    # Load the model
    qd_model = QuickDrawRNN(classes)
    qd_model.load_state_dict(torch.load(model_root_path, weights_only=True))
    qd_model.eval()

    # Parse the arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('input_file_path', type=str,
                        help='The path to the input ndjson file')

    args = parser.parse_args()
    file_path = args.input_file_path

    # Read the data
    samples = readData(file_path, classToIndex)

    # Create the loader
    qd_eval_dataset = QuickDrawDataset(samples, classes, False)
    qd_eval_dataloader = DataLoader(qd_eval_dataset, batch_size=1, shuffle=False, 
                                    num_workers=0, generator=torch.Generator(device='cuda'),
                                    collate_fn=functools.partial(quickDrawCollateFn, batch_size=1))
    
    # Evaluate the data
    qd_model.train(False)
    for i, batch in enumerate(qd_eval_dataloader):
        logits = qd_model(batch["ink"], batch["length"])
    
        predicted_labels = torch.argmax(logits, dim=1)
        actual_labels = batch["classIndex"]
    
        print(logits)
        print("Result:   ", predicted_labels)
        print("Expected: ", actual_labels)
        print()
        print(" --> PREDICTED CLASS: ", classes[predicted_labels[0].item()])
        print(classes[predicted_labels[0].item()]) # This line is here to skip parsing in Unreal
        print()

    if len(samples) == 0:
        print("Empty drawing sent for evaluation")
        print("Empty") # This line is here to skip parsing in Unreal

    """

    To keep in mind (taken from the bottom of the main reference page: https://github.com/tensorflow/docs/blob/master/site/en/r1/tutorials/sequences/recurrent_quickdraw.md )

    "When training the model for 1M steps you can expect to get an accuracy of approximately of approximately 70% on the top-1 candidate. 
    Note that this accuracy is sufficient to build the quickdraw game because of the game dynamics the user will be able to adjust their 
    drawing until it is ready. Also, the game does not use the top-1 candidate only but accepts a drawing as correct if the target category 
    shows up with a score better than a fixed threshold."
    
    """
    
