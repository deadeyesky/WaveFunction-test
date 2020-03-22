# Import the functions file
import functions
import os
import sys
import matplotlib.pyplot as plt

# Begin by entering the file path
#path = input("Enter filepath -> ")
path = "/home/ultronium/Documents/Atom/My Products/Frequency Data/Truss Prism Data/Trial 1/"
log_path = "/home/ultronium/Documents/Atom/My Products/Frequency Data/Log_Folder"

# Generates list of files in path
csv_files = os.listdir(path)

# Define variables
frequency = functions.namesList(csv_files) # Independent variable
rangeList1 = [] # Going to be sorted from first to last
rangeList2 = [] # Going to be sorted from first to last
meanList1 = [] # Going to be sorted from first to last
meanList2 = [] # Going to be sorted from first to last

#print(frequency)
#print(csv_files)

# Take the directory and append the ranges from their respective columns
for file in functions.sort(csv_files):
    analog1Range = functions.getRange(functions.convertToFloat(functions.columnList(path + file, 0)))
    analog2Range = functions.getRange(functions.convertToFloat(functions.columnList(path + file, 1)))
    rangeList1.append(analog1Range)
    rangeList2.append(analog2Range)
    analog1Mean = functions.mean(functions.convertToFloat(functions.columnList(path + file, 0)))
    analog2Mean = functions.mean(functions.convertToFloat(functions.columnList(path + file, 1)))
    meanList1.append(analog1Mean)
    meanList2.append(analog2Mean)
    print("File: " + file)

#print(functions.sort(csv_files))
#print(rangeList1)
#print(rangeList2)

difference_mean = functions.subtract(meanList1, meanList2)
difference_range = functions.subtract(rangeList1, rangeList2)
divide_mean = functions.divide(meanList2, meanList1)
divide_range = functions.divide(rangeList1, rangeList2)

#print(divide_mean)

plt.plot(frequency, divide_mean, linestyle = '-')
plt.axis([0, 5000, 0.5, 1])
#plt.errorbar(frequency, divide_mean, yerr = 0.05, fmt='o')
#plt.title('Hexagonal Prism Trial 1 Vibration Absorption Spectrum')
plt.ylabel('Proportion of Incoming Noise to Outgoing Noise Energy (%)')
plt.xlabel('Frequency (Hz)')
plt.show()
