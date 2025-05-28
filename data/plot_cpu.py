from utils import *

## plot 配置
plt.rc('font', **font)
thread_colors = dict(zip([1, 2, 4, 8, 16], colors5))

## 数据处理
files = {
  'data/cpu/cpu_threads1.csv': 1,
  'data/cpu/cpu_threads2.csv': 2,
  'data/cpu/cpu_threads4.csv': 4,
  'data/cpu/cpu_threads8.csv': 8,
  'data/cpu/cpu_threads16.csv': 16
}

### 读取
data_dict = {}
for file_path, thread_count in files.items():
  try:
    df = pd.read_csv(file_path)
    df['threads'] = thread_count
    data_dict[thread_count] = df
    print(f"成功读取 {file_path}: {len(df)} 行数据")
  except Exception as e:
    print(f"读取文件 {file_path} 时出错: {e}")

### 数据清洗
cleaned_data_dict = {}
for thread_count, df in data_dict.items():
  df['physics_update_elapsed_time'] = df['physics_update_elapsed_time'] / 1000
  df['render_elapsed_time'] = df['render_elapsed_time'] / 1000

  df_clean = remove_outliers(df, ['physics_update_elapsed_time', 'render_elapsed_time'])
  df_smooth = smooth_data(df_clean, ['physics_update_elapsed_time', 'render_elapsed_time'],
                          window_size=250)
  cleaned_data_dict[thread_count] = df_smooth
  
  print(f"{thread_count}线程: 原始数据{len(df)}行 -> 清洗后{len(df_clean)}行 "
        f"(移除{len(df) - len(df_clean)}个离群值)")
    
data_dict = cleaned_data_dict

## 绘制图表
fig, axes = plt.subplots(2, 2, figsize=(20, 15), facecolor='white')

### 粒子数-计算时间折线图
ax = axes[0, 0]

for thread_count in sorted(data_dict.keys()):
  df = data_dict[thread_count]
  ax.plot(df['object_counts'], df['physics_update_elapsed_time'], 
          color=thread_colors[thread_count], linewidth=0.5, alpha=0.3, zorder=5)
  ax.plot(df['object_counts'], df['physics_update_elapsed_time_smooth'], 
           color=thread_colors[thread_count], linewidth=1.5, alpha=0.9,
           zorder=10, label=f'{thread_count} 线程')

ax.set_facecolor('0.9')
ax.legend(fontsize=10)
ax.set_xlabel('生成粒子数')
ax.set_ylabel('单次计算时间 (ms)')
ax.set_title('不同线程数下粒子数与计算时间的关系', fontsize=14)
ax.set_xlim(0, 30000)
ax.set_ylim(0, 40)
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)

### 粒子数-渲染时间折线图
ax = axes[0, 1]

for thread_count in sorted(data_dict.keys()):
  df = data_dict[thread_count]
  ax.plot(df['object_counts'], df['render_elapsed_time'], 
          color=thread_colors[thread_count], linewidth=0.5, alpha=0.3, zorder=5)
  ax.plot(df['object_counts'], df['render_elapsed_time_smooth'], 
           color=thread_colors[thread_count], linewidth=1.5, alpha=0.9,
           zorder=10, label=f'{thread_count} 线程')

ax.set_facecolor('0.9')
ax.legend(fontsize=10)
ax.set_xlabel('生成粒子数')
ax.set_ylabel('单次渲染时间 (ms)')
ax.set_title('不同线程数下粒子数与渲染时间的关系', fontsize=14)
ax.set_xlim(0, 30000)
ax.set_ylim(0, 1.25)
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)

### 线程数-计算时间箱线图
ax = axes[1, 0]

physics_data = []
thread_labels = []
for thread_count in sorted(data_dict.keys()):
  physics_data.append(data_dict[thread_count]['physics_update_elapsed_time'])
  thread_labels.append(f'{thread_count}线程')

box_plot1 = ax.boxplot(physics_data, labels=thread_labels, patch_artist=True, showmeans=True)
for patch, color in zip(box_plot1['boxes'], colors5):
  patch.set_facecolor(color)
  patch.set_alpha(0.7)

ax.set_facecolor('0.9')
ax.legend(fontsize=10)
ax.set_xlabel('线程数')
ax.set_ylabel('单次计算时间 (ms)')
ax.set_title('不同线程数的计算时间分布', fontsize=14)
ax.set_ylim(0, 40)
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)

### 线程数-渲染时间箱线图
ax = axes[1, 1]

render_data = []
for thread_count in sorted(data_dict.keys()):
  render_data.append(data_dict[thread_count]['render_elapsed_time'])

box_plot2 = ax.boxplot(render_data, labels=thread_labels, patch_artist=True, showmeans=True)
for patch, color in zip(box_plot2['boxes'], colors5):
  patch.set_facecolor(color)
  patch.set_alpha(0.7)

ax.set_facecolor('0.9')
ax.legend(fontsize=10)
ax.set_xlabel('线程数')
ax.set_ylabel('单次渲染时间 (ms)')
ax.set_title('不同线程数的渲染时间分布', fontsize=14)
ax.set_ylim(0, 1.25)
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)

## 导出与打印
plt.tight_layout(pad=3.0)
plt.savefig('data/svg/cpu_threads_analysis.svg', dpi=300)
plt.savefig('data/img/cpu_threads_analysis.png', dpi=300)
plt.show()

print("\n=== 数据统计摘要 ===")
for thread_count in sorted(data_dict.keys()):
  df = data_dict[thread_count]
  print(f"\n{thread_count} 线程:")
  print(f"  计算时间 - 平均: {df['physics_update_elapsed_time'].mean():.2f}ms, "
        f"标准差: {df['physics_update_elapsed_time'].std():.2f}ms")
  print(f"  渲染时间 - 平均: {df['render_elapsed_time'].mean():.2f}ms, "
        f"标准差: {df['render_elapsed_time'].std():.2f}ms")
