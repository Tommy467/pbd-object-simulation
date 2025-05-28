from utils import *

## plot 配置
plt.rc('font', **font)

## 数据处理
files = {
  'data/cpu/cpu_threads1.csv': 1,
  'data/cpu/cpu_threads2.csv': 2,
  'data/cpu/cpu_threads4.csv': 4,
  'data/cpu/cpu_threads8.csv': 8,
  'data/cpu/cpu_threads16.csv': 16,
  'data/gpu/gpu_block_size32.csv': 32,
  'data/gpu/gpu_block_size64.csv': 64,
  'data/gpu/gpu_block_size128.csv': 128,
  'data/gpu/gpu_block_size256.csv': 256,
  'data/gpu/gpu_block_size512.csv': 512,
  'data/gpu/gpu_block_size1024.csv': 1024
}

### 读取数据
data_dict = {}
for file_path, core_count in files.items():
  try:
    df = pd.read_csv(file_path, usecols=['object_counts', 'physics_update_elapsed_time'])
    df['core'] = core_count
    data_dict[core_count] = df
    print(f"成功读取 {file_path}: {len(df)} 行数据")
  except Exception as e:
    print(f"读取文件 {file_path} 时出错: {e}")

### 计算总计算时间
results = {}
for core_count, df in data_dict.items():
    total_time, data_points = calculate_total_physics_time(df, 30000)
    results[core_count] = {
        'total_time': total_time,
        'data_points': data_points,
        'avg_time_per_step': total_time / data_points if data_points > 0 else 0
    }
    
    print(f"{core_count} 线程: 总计算时间 {total_time:.2f}ms, "
          f"数据点数 {data_points}, 平均每步 {total_time/data_points:.2f}ms")


# ### 2. 线程数 vs 平均单步计算时间折线图
# avg_times = [thread_results[t]['avg_time_per_step'] for t in threads]

# ax2.plot(threads, avg_times, 's-', linewidth=3, markersize=8, 
#          color='#4ECDC4', markerfacecolor='#4ECDC4', markeredgecolor='white', 
#          markeredgewidth=2, alpha=0.8)

# for i, (thread, time) in enumerate(zip(threads, avg_times)):
#     ax2.annotate(f'{time:.2f}ms', 
#                 (thread, time), 
#                 textcoords="offset points", 
#                 xytext=(0,10), 
#                 ha='center', fontsize=10, weight='bold')

# ax2.set_xlabel('线程数', fontsize=14, weight='bold')
# ax2.set_ylabel('平均单步计算时间 (ms)', fontsize=14, weight='bold')
# ax2.set_title('线程数与平均单步计算时间关系\n(前30000个粒子对象)', fontsize=16, weight='bold', pad=20)
# ax2.grid(True, alpha=0.3, linestyle='--')
# ax2.set_facecolor('#F8F9FA')

# # 设置x轴刻度
# ax2.set_xticks(threads)
# ax2.set_xticklabels([f'{t}' for t in threads])

## 导出
plt.tight_layout(pad=3.0)
plt.savefig('data/svg/speedup_analysis.svg', dpi=300, bbox_inches='tight')
plt.savefig('data/img/speedup_analysis.png', dpi=300, bbox_inches='tight')
plt.show()

