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
    
    print(f"{core_count} 核心数: 总计算时间 {total_time:.2f}ms, "
          f"数据点数 {data_points}, 平均每步 {total_time/data_points:.2f}ms")

### 计算加速比
cpu_threads = [1, 2, 4, 8, 16]
cpu_speedup = []
baseline_time = results[1]['total_time']

for thread_count in cpu_threads:
  if thread_count in results:
    speedup = baseline_time / results[thread_count]['total_time']
    cpu_speedup.append(speedup)
  else:
    cpu_speedup.append(0)

gpu_blocks = [32, 64, 128, 256, 512, 1024]
gpu_speedup = []
gpu_baseline_time = results[32]['total_time']

for block_size in gpu_blocks:
  if block_size in results:
    speedup = gpu_baseline_time / results[block_size]['total_time']
    gpu_speedup.append(speedup)
  else:
    gpu_speedup.append(0)

## 绘图
fig, axes = plt.subplots(1, 2, figsize=(20, 8), facecolor='white')

### 线程数-加速比折线图
ax = axes[0]

ax.plot(cpu_threads, cpu_speedup, 'o-', linewidth=2, markersize=8, 
        color='blue', label='CPU 线程加速比')
ax.set_facecolor('0.9')
ax.legend(fontsize=12)
ax.set_xlabel('CPU 线程数')
ax.set_ylabel('加速比')
ax.set_title('CPU 线程数与加速比的关系', fontsize=14)
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)

### GPU 块大小-加速比折线图
ax = axes[1]

ax.plot(gpu_blocks, gpu_speedup, 's-', linewidth=2, markersize=8, 
        color='red', label='GPU 块大小加速比')
ax.set_facecolor('0.9')
ax.legend(fontsize=12)
ax.set_xlabel('GPU 块大小')
ax.set_ylabel('加速比')
ax.set_title('GPU 块大小与加速比的关系', fontsize=14)
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)


## 导出
plt.tight_layout(pad=3.0)
plt.savefig('data/svg/speedup_analysis.svg', dpi=300, bbox_inches='tight')
plt.savefig('data/img/speedup_analysis.png', dpi=300, bbox_inches='tight')
plt.show()

