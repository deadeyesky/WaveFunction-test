# Import modules
import csv
import natsort
import statistics
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

#-----------Standard Mathematical Operators------------#
# Subtract list elements from other list elements
def subtract (list1, list2):
    return np.subtract(list1, list2)

# Divide list elements from other list elements
def divide (list1, list2):
    return [float(ai) / bi for ai, bi in zip(list1, list2)]

#----------------Statistical Calculations--------------#
# Calculate mean from list
def mean (list):
     return statistics.mean(list)

# Define the range function
def getRange(list):
    return max(list) - min(list) # Finds the range by subtracting the maximum by the minimum


#---------List Operations and File Functions-----------#
# Sorts list elements that are strings
def sort (list):
    return natsort.natsorted(list, reverse = False)

# Converts list string elements to float
def convertToFloat (list):
    list = [float(i) for i in list]
    return list

# List names of files and create a list of those names
def namesList (file_list):
    file_list = ' '.join(file_list).replace('.CSV','').split() # Takes out the .csv extensions
    # print(file_list)
    list = [int(x) for x in file_list] # Represents the names of the list as integers
    list.sort() # Sorts the elements in the list
    return list

# Takes the column selected and turns it into a list for a selected file
def columnList(path, column):
    list = []
    with open(path, "r") as csv_file:
        csv_reader = csv.reader(csv_file, delimiter = ',')
        for lines in csv_reader:
            list.append(lines[column])
    return list

def read_csv(filename):
    df = pd.read_csv(filename)
    return df

# Adds a new column of data to the .csv log file
def addCsvColumn (log_path, difference_list, header):
    with open(log_path + 'difference_range.csv', 'wb') as f:
        f.write("\n".join(difference_list))
    f.close()

def fname(arg):
    # Write results to a .csv file when a new file is tested
    with open(log_path + 'difference_mean.csv', 'wb') as f:
        f.write("\n".join(difference_mean))
    f.close()

    with open(log_path + 'difference_range.csv', 'wb') as f:
        f.write("\n".join(difference_range))
    f.close()
    pass
