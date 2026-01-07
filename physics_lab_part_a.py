import matplotlib.pyplot as plt
import numpy as np
from scipy import stats

# Data from Part A - Force vs Acceleration (constant mass)
F = np.array([1.962, 2.943, 3.924, 4.905, 5.886])  # Force [N]
a = np.array([1.93, 2.64, 3.21, 4.22, 5.81])       # Acceleration [m/s²]

# Linear regression
slope, intercept, r_value, p_value, std_err = stats.linregress(F, a)
r_squared = r_value**2

# Create trendline
F_line = np.linspace(min(F) - 0.5, max(F) + 0.5, 100)
a_line = slope * F_line + intercept

# Create the plot
plt.figure(figsize=(10, 7))
plt.style.use('seaborn-v0_8-whitegrid')

# Plot data points
plt.scatter(F, a, color='#2E86AB', s=120, zorder=5, edgecolors='black', linewidth=1.5, label='Measured Data')

# Plot trendline
plt.plot(F_line, a_line, color='#E94F37', linewidth=2.5, linestyle='--', label='Linear Trendline')

# Labels and title
plt.xlabel('Net Force F [N]', fontsize=14, fontweight='bold')
plt.ylabel('Acceleration a [m/s²]', fontsize=14, fontweight='bold')
plt.title('Part A: Acceleration vs. Net Force\n(Constant System Mass)', fontsize=16, fontweight='bold')

# Add equation and R² to plot
equation_text = f'a = {slope:.4f}·F + {intercept:.4f}\n$R^2$ = {r_squared:.4f}'
plt.text(0.05, 0.95, equation_text, transform=plt.gca().transAxes, fontsize=12,
         verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

# Theoretical slope calculation
# Total mass = cart (260g) + basket (100g) + 8 weights (800g) = 1160g = 1.16 kg
m_total = 1.16  # kg
theoretical_slope = 1 / m_total

# Add theoretical comparison
theory_text = f'Theoretical slope: 1/m = {theoretical_slope:.4f}\nMeasured slope: {slope:.4f}\nError: {abs(slope - theoretical_slope)/theoretical_slope * 100:.1f}%'
plt.text(0.95, 0.25, theory_text, transform=plt.gca().transAxes, fontsize=11,
         verticalalignment='top', horizontalalignment='right',
         bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))

plt.legend(loc='lower right', fontsize=11)
plt.grid(True, alpha=0.3)
plt.tight_layout()

# Save the plot
plt.savefig('part_a_graph.png', dpi=150, bbox_inches='tight')
plt.show()

print("\n" + "="*50)
print("Part A - Analysis Results")
print("="*50)
print(f"Linear Regression: a = {slope:.4f}·F + {intercept:.4f}")
print(f"R² = {r_squared:.4f}")
print(f"Measured slope (1/m): {slope:.4f} kg⁻¹")
print(f"Theoretical slope (1/m_total): {theoretical_slope:.4f} kg⁻¹")
print(f"Relative error: {abs(slope - theoretical_slope)/theoretical_slope * 100:.1f}%")
print("="*50)

