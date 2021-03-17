import statistics as Statistics
import math as Math

# ECG data
ecg = open("__ecg data file here!__","r") # ECG data file handler
ecg_data_list = ecg.read().split()
ecg_data_list = map(float, ecg_data_list)
ecg_data_list = list(ecg_data_list) 

# Get difference between 2 values in ECG tuple
def difference(ecg_tuple):
    return ecg_tuple[0]-ecg_tuple[1]

# Get stats for data set
def stats_suite(ecg_data):

    # Get min, max, median, average
    print(f"Min: {min(ecg_data)}")
    print(f"Median: {Statistics.median(ecg_data)}")
    print(f"Max: {max(ecg_data)}")
    print(f"Average: {Statistics.mean(ecg_data)}")
    print(f"STDev: {Statistics.stdev(ecg_data)}")


    # Get min, max, median, average of neighbor difference
    dif_list = list(zip(ecg_data[1:], ecg_data))
    dif_list = list(map(difference, dif_list))
    print(f"Dif. Min: {min(dif_list)}")
    print(f"Dif. Median: {Statistics.median(dif_list)}")
    print(f"Dif. Max: {max(dif_list)}")
    print(f"Dif. Average: {Statistics.mean(dif_list)}")
    print(f"Dif. STDev: {Statistics.stdev(dif_list)}")

stats_suite(ecg_data_list)