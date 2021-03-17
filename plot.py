# Import plotting modole
import matplotlib.pyplot as mat_plot
mat_plot.rcParams['font.sans-serif'] = ['Arial']
mat_plot.rcParams['font.size'] = 18

# ECG data
ecg = open("__ecg data file here!__","r") # ECG data file handler
ecg_data_list = ecg.read().split()
ecg_data_list = map(float, ecg_data_list)
ecg_data_list = list(ecg_data_list)

# Create x-axis scaling values to ms
ms_count = 0
x_axis = []
for x in range(5000):
    x_axis.append(ms_count)
    ms_count += 4

mat_plot.xlabel("Time (ms)")
mat_plot.ylabel("Heart Activity")
mat_plot.plot(ecg_data_list)
mat_plot.show()