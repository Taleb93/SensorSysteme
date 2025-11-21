import pandas as pd
import matplotlib.pyplot as plt

# === 1️⃣ Daten einlesen ===
# Datei enthält: time_us, voltage_V, current_A (Tab-getrennt)
df = pd.read_csv("dataVint_stuf1.txt", sep=r"\s+", header=None,
                 names=["time_us", "voltage_V", "current_A"])

# Mikrosekunden → Sekunden
df["time_s"] = df["time_us"] * 1e-6

# === 2️⃣ Zeit auf 0 setzen & in Millisekunden umrechnen ===
t0 = df["time_s"].iloc[0]
df["time_ms"] = (df["time_s"] - t0) * 1000.0   # Zeit in Millisekunden
print(f"Zeit-Offset entfernt, Start war bei {t0:.6f} s")

# === 3️⃣ Kombinierter Plot (Spannung & Strom) ===
fig, ax1 = plt.subplots(figsize=(10, 6))

# Spannung (blau, linke Achse)
ax1.plot(df["time_ms"], df["voltage_V"], color='b', label="Spannung (V)")
ax1.set_xlabel("Zeit (ms)")
ax1.set_ylabel("Spannung (V)", color='b')
ax1.tick_params(axis='y', labelcolor='b')

# Strom (rot, rechte Achse)
ax2 = ax1.twinx()
ax2.plot(df["time_ms"], df["current_A"], color='r', label="Strom (A)")
ax2.set_ylabel("Strom (A)", color='r')
ax2.tick_params(axis='y', labelcolor='r')

# Legenden zusammenführen
lines_1, labels_1 = ax1.get_legend_handles_labels()
lines_2, labels_2 = ax2.get_legend_handles_labels()
ax2.legend(lines_1 + lines_2, labels_1 + labels_2, loc="upper right")

plt.title("Spannung und Strom über der Zeit (t₀ = 0 ms)")
plt.grid(True)
plt.tight_layout()
plt.show()

# === 4️⃣ Getrennte Plots (Spannung & Strom separat) ===
fig, (axV, axI) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)

axV.plot(df["time_ms"], df["voltage_V"], color='b')
axV.set_ylabel("Spannung (V)")
axV.grid(True)

axI.plot(df["time_ms"], df["current_A"], color='r')
axI.set_ylabel("Strom (A)")
axI.set_xlabel("Zeit (ms)")
axI.grid(True)

plt.suptitle("Gemessene Spannung und Strom (ab t₀ = 0 ms)")
plt.tight_layout()
plt.show()
