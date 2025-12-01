import tkinter as tk
from tkinter import filedialog, ttk
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np

class RocketAnalyzer:
    def __init__(self, root):
        self.root = root
        self.root.title("Rocket Flight Analyzer")
        self.root.geometry("1000x700")

        self.fig, self.ax = plt.subplots(figsize=(10, 6))
        self.canvas = FigureCanvasTkAgg(self.fig, master=root)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        btn_frame = ttk.Frame(root)
        btn_frame.pack(fill=tk.X, padx=10, pady=5)
        ttk.Button(btn_frame, text="Load Flight CSV", command=self.load_file).pack(side=tk.LEFT, padx=5)

        self.info_label = ttk.Label(root, text="No file loaded")
        self.info_label.pack(pady=5)

    def load_file(self):
        path = filedialog.askopenfilename(filetypes=[("CSV files", "*.csv")])
        if not path: return

        df = pd.read_csv(path)
        self.df = df
        self.info_label.config(text=f"Loaded: {path.split('/')[-1]}")

        self.ax.clear()
        time = df['time_s']
        alt = df['altitude_m']

        self.ax.plot(time, alt, label='Altitude', linewidth=2)
        self.ax.set_xlabel("Time (s)")
        self.ax.set_ylabel("Altitude (m AGL)")
        self.ax.grid(True)
        self.ax.legend()

        
        apogee_idx = alt.idxmax()
        apogee_time = time.iloc[apogee_idx]
        apogee_alt = alt.iloc[apogee_idx]
        self.ax.scatter(apogee_time, apogee_alt, color='red', s=100, label=f'Apogee {apogee_alt:.1f}m')
        self.ax.legend()

        
        velocity = np.gradient(alt, time)
        accel = np.gradient(velocity, time)

        
        post_apogee = accel[apogee_idx:]
        if len(post_apogee) > 10:
            drag_idx = apogee_idx + np.argmin(post_apogee[:500])  
            if accel[drag_idx] < -25:  
                self.ax.axvline(time.iloc[drag_idx], color='orange', linestyle='--', label='Drogue Deploy?')

        self.canvas.draw()

if __name__ == "__main__":
    root = tk.Tk()
    app = RocketAnalyzer(root)

    root.mainloop()
