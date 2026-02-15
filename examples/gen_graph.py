import matplotlib.pyplot as plt
import os

# Data
languages = ['Python', 'JavaScript', 'Axeon']
times = [998.2, 53.3, 24.8]
colors = ['#FFD43B', '#F7DF1E', '#3498DB']  # Python yellow, JS yellow, Axeon blue

plt.figure(figsize=(10, 6))
bars = plt.bar(languages, times, color=colors)

# Log scale for better visibility because Python is so much slower
plt.yscale('log')

plt.title('Performance Comparison: Numeric Loop (10M Iterations)', fontsize=16)
plt.ylabel('Execution Time (ms) - Log Scale', fontsize=12)
plt.grid(axis='y', linestyle='--', alpha=0.7)

# Add data labels
for bar in bars:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2, yval, f'{yval}ms', 
             va='bottom', ha='center', fontsize=12, fontweight='bold')

# Save to artifacts directory
output_path = '/home/kali/.gemini/antigravity/brain/39d56e16-5ff1-4255-9638-7a1f22ef2249/performance_comparison.png'
plt.savefig(output_path, dpi=300, bbox_inches='tight')
print(f"Graph saved to {output_path}")
