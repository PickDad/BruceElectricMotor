# Import necessary libraries
import matplotlib.pyplot as plt
import pandas as pd
import sympy as sp
import scipy as sc
import numpy as np
import math

# Back EMF Waveforms (Insert file path of CSV file from oscilloscope capture of 3phase Line-Line voltages)
# The following code assumes that the CSV file has columns named 'time', 'Vab', 'Vbc', and 'Vca'.
backEMF = pd.DataFrame(pd.read_csv('Test_Results/20250421/25042101.csv'))

time = backEMF['time'].values
sample_step= (time[1]-time[0])
sample_size = time.size

print(f"Sample Period:\t{sample_step}\nSample Size:\t{sample_size}")

# Calculate the Fast Fourier Transform (FFT) of the back EMF signals
Vab_backEMF_fft = abs(sc.fft.fft(backEMF['Vab'].values))
Vbc_backEMF_fft = abs(sc.fft.fft(backEMF['Vbc'].values))
Vca_backEMF_fft = abs(sc.fft.fft(backEMF['Vca'].values))
fft_freq = sc.fft.fftfreq(sample_size, d=sample_step)

# Find the peaks of the FFTs
Vab_peakFreq, _ = sc.signal.find_peaks(Vab_backEMF_fft, height=10)
Vbc_peakFreq, _ = sc.signal.find_peaks(Vbc_backEMF_fft, height=10)
Vca_peakFreq, _ = sc.signal.find_peaks(Vca_backEMF_fft, height=10)
print(f"Peak Frequency Vab:\t{fft_freq[Vab_peakFreq]}")
print(f"Peak Frequency Vbc:\t{fft_freq[Vbc_peakFreq]}")
print(f"Peak Frequency Vca:\t{fft_freq[Vca_peakFreq]}")

# Plot the FFT results
plt.plot(fft_freq, Vab_backEMF_fft, label='FFT Vab Back EMF')
plt.plot(fft_freq, Vbc_backEMF_fft, label='FFT Vbc Back EMF')
plt.plot(fft_freq, Vca_backEMF_fft, label='FFT Vca Back EMF')
plt.title('FFT Back EMF')
plt.xlabel('Frequency (Hz)')
plt.ylabel('FFT Amplitude')
plt.xlim(0,500)
plt.legend()
plt.grid()
plt.show()

# Filter the back EMF results
freq_cutoff = 50
b, a = sc.signal.butter(3, freq_cutoff, 'low', fs=1/sample_step)
Vab_filt = sc.signal.filtfilt(b, a, backEMF['Vab'].values)
Vbc_filt = sc.signal.filtfilt(b, a, backEMF['Vbc'].values)
Vca_filt = sc.signal.filtfilt(b, a, backEMF['Vca'].values)

# Plot the filtered back EMF results
plt.plot(backEMF['time'].values, Vab_filt, label='Filtered Vab Back EMF')
plt.plot(backEMF['time'].values, Vbc_filt, label='Filtered Vbc Back EMF')
plt.plot(backEMF['time'].values, Vca_filt, label='Filtered Vca Back EMF')
plt.title('Filtered Back EMF Waveform')
plt.xlabel('Time (s)')
plt.ylabel('Voltage (V)')
plt.legend()
plt.grid()
plt.show()

# Find Line-Nuetral Voltages (Change code to meet specific needs)
Vc= np.sqrt(3)*Vca_filt

# Find the peaks of L-N Voltage, set height to detection threshold
Vc_peaks, _ = sc.signal.find_peaks(Vc, height=0)

Vc_peak_times = time[Vc_peaks]
Vc_peak_mag = Vc[Vc_peaks]

print(f"Va Peak Times:\t{Vc_peak_times}")
print(f"Va Peak Magnitudes:\t{Vc_peak_mag}")

# Set the period of interest for the integral calculation
Vc_int = Vc[Vc_peaks[1]:Vc_peaks[2]]
Vc_int_times = time[Vc_peaks[1]:Vc_peaks[2]]

# Calculate the frequencies of the back EMF to verify results with experiment
T_avg = 0
for i in range(1,len(Vc_peak_times)-1):
    T = Vc_peak_times[i+1] - Vc_peak_times[i]
    T_avg = (T_avg + T)
    print(f"Period {i}: {T}")
T_avg = T_avg / (len(Vc_peak_times)-2)
Vc_freq = 1/T_avg
RotorRPM = Vc_freq * 60
print(f"Average Period:\t{T_avg} S\nFrequency:\t{Vc_freq} Hz\nRotor RPM:\t{RotorRPM} RPM")

# Plot the L-N voltage and highlight the region to be integrated
plt.plot(time, Vc, label='Va')
plt.plot(Vc_int_times, Vc_int, label='Va Integral', color='orange')
plt.plot(Vc_peak_times, Vc_peak_mag, 'x', label='Va Peaks')
plt.title('Filtered Back EMF Waveform at 1250 RPM')
plt.xlabel('Time [s]')
plt.ylabel('Voltage [V]')
plt.grid()
plt.legend()
plt.show()

#Inductance Calculation
# Calculate the inductance to compare to experimental results and to find the relative permeability of the Iron-filled PLA
H = 14.73e-3
W = 10.28e-3
L = 13.08e-3
r = (1.29e-3)/2
N = 15
Inductance = 19e-6/2
geometry = -2*(W+H)+2*math.sqrt(W**2+H**2)-H*math.log((H+math.sqrt(W**2+H**2))/W)-W*math.log((W+math.sqrt(W**2+H**2))/H)+H*math.log((2*H)/r)+W*math.log((2*W)/r)
mu_r = (sc.constants.mu_0/(sc.constants.pi*Inductance))*geometry*N**2
print(f"Relative Permeability:\t{mu_r}")

#Flux Linkage Calculation
flux_linkage = abs(sc.integrate.trapezoid(Vc_int, Vc_int_times))
print(f"Flux Linkage:\t{flux_linkage}")

# Magnetic Flux Calculation
flux = flux_linkage / N
print(f"Flux:\t{flux}")

# Calculate the inductance and flux linkage for a range of turns and plot
N_array = np.arange(0, 150, 1)
L_array = ((sc.constants.mu_0*mu_r*N_array**2)/sc.constants.pi)*geometry
flux_linkage_array = flux*N_array

plt.plot(N_array, L_array, label='Inductance')
plt.plot(N_array, flux_linkage_array, label='Flux Linkage')
plt.title('Inductance and Flux Linkage vs Turns')
plt.xlabel('Turns')
plt.ylabel('Inductance (H) / Flux Linkage (Wb)')
plt.legend()
plt.grid()
plt.show()

# Save the inductance and flux linkage data to a CSV file
Value_Table = pd.DataFrame({
    'Turns': N_array,
    'Inductance': L_array,
    'Flux Linkage': flux_linkage_array
})
Value_Table.to_csv('Test_Results/20250405/Inductance_Flux_Linkage.csv', index=False)