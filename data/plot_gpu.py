from utils import *

## plot 配置
plt.rc('font', **font)
block_colors = dict(zip([32, 64, 128, 256, 512, 1024], colors6))

## 数据处理
files = {
  'data/gpu/gpu_block_size32.csv': 32,
  'data/gpu/gpu_block_size64.csv': 64,
  'data/gpu/gpu_block_size128.csv': 128,
  'data/gpu/gpu_block_size256.csv': 256,
  'data/gpu/gpu_block_size512.csv': 512,
  'data/gpu/gpu_block_size1024.csv': 1024
}

### 读取
data_dict = {}
for file_path, block_size in files.items():
  try:
    df = pd.read_csv(file_path)
    df['block_size'] = block_size
    data_dict[block_size] = df
    print(f"成功读取 {file_path}: {len(df)} 行数据")
  except Exception as e:
    print(f"读取文件 {file_path} 时出错: {e}")

### 数据清洗
cleaned_data_dict = {}
for block_size, df in data_dict.items():
  df['physics_update_elapsed_time'] = df['physics_update_elapsed_time'] / 1000
  df['render_elapsed_time'] = df['render_elapsed_time'] / 1000

  df_clean = remove_outliers(df, ['physics_update_elapsed_time', 'render_elapsed_time'])
  df_smooth = smooth_data(df_clean, ['physics_update_elapsed_time', 'render_elapsed_time'],
                          window_size=500)
  cleaned_data_dict[block_size] = df_smooth
  
  print(f"Block Size {block_size}: 原始数据{len(df)}行 -> 清洗后{len(df_clean)}行 -> 平滑处理完成")

data_dict = cleaned_data_dict

### 合并
all_data = pd.concat(data_dict.values(), ignore_index=True)

## 绘制图表
fig, axes = plt.subplots(2, 2, figsize=(20, 15), facecolor='white')

### 粒子数-物理计算时间折线图
ax = axes[0, 0]

for block_size in sorted(data_dict.keys()):
  df = data_dict[block_size]
  ax.plot(df['object_counts'], df['physics_update_elapsed_time'], 
          color=block_colors[block_size], linewidth=0.5, alpha=0.3, zorder=5)
  ax.plot(df['object_counts'], df['physics_update_elapsed_time_smooth'], 
          color=block_colors[block_size], linewidth=1.5, alpha=0.9,
          zorder=10, label=f'Block Size {block_size}')

ax.set_facecolor('0.9')
ax.legend(fontsize=10)
ax.set_xlabel('生成粒子数')
ax.set_ylabel('单次物理计算时间 (ms)')
ax.set_title('不同Block Size下粒子数与物理计算时间的关系', fontsize=14)
ax.set_xlim(0, max(all_data['object_counts']))
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)


### 粒子数-渲染时间折线图
ax = axes[0, 1]

for block_size in sorted(data_dict.keys()):
  df = data_dict[block_size]
  ax.plot(df['object_counts'], df['render_elapsed_time'], 
          color=block_colors[block_size], linewidth=0.5, alpha=0.3, zorder=5)
  ax.plot(df['object_counts'], df['render_elapsed_time_smooth'], 
          color=block_colors[block_size], linewidth=1.5, alpha=0.9,
          zorder=10, label=f'Block Size {block_size}')

ax.set_facecolor('0.9')
ax.legend(fontsize=10)
ax.set_xlabel('生成粒子数')
ax.set_ylabel('单次渲染时间 (ms)')
ax.set_title('不同Block Size下粒子数与渲染时间的关系', fontsize=14)
ax.set_xlim(0, max(all_data['object_counts']))
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)

### Block Size-物理计算时间箱线图
ax = axes[1, 0]

physics_data = []
block_labels = []
for block_size in sorted(data_dict.keys()):
  physics_data.append(data_dict[block_size]['physics_update_elapsed_time'])
  block_labels.append(f'{block_size}')

box_plot1 = ax.boxplot(physics_data, labels=block_labels, patch_artist=True, showmeans=True)
for patch, color in zip(box_plot1['boxes'], colors6):
  patch.set_facecolor(color)
  patch.set_alpha(0.7)

ax.set_facecolor('0.9')
ax.set_xlabel('Block Size')
ax.set_ylabel('单次物理计算时间 (ms)')
ax.set_title('不同Block Size的物理计算时间分布', fontsize=14)
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)

### Block Size-渲染时间箱线图
ax = axes[1, 1]

render_data = []
for block_size in sorted(data_dict.keys()):
  render_data.append(data_dict[block_size]['render_elapsed_time'])

box_plot2 = ax.boxplot(render_data, labels=block_labels, patch_artist=True, showmeans=True)
for patch, color in zip(box_plot2['boxes'], colors6):
  patch.set_facecolor(color)
  patch.set_alpha(0.7)

ax.set_facecolor('0.9')
ax.set_xlabel('Block Size')
ax.set_ylabel('单次渲染时间 (ms)')
ax.set_title('不同Block Size的渲染时间分布', fontsize=14)
ax.grid(linestyle='--', linewidth=0.5, color='.25', zorder=-10)

## 导出与打印
plt.tight_layout(pad=3.0)
plt.savefig('data/svg/gpu_block_size_analysis.svg', dpi=300)
plt.savefig('data/img/gpu_block_size_analysis.png', dpi=300)
plt.show()

print("\n=== GPU Block Size 数据统计摘要 ===")
for block_size in sorted(data_dict.keys()):
  df = data_dict[block_size]
  print(f"\nBlock Size {block_size}:")
  print(f"  物理计算时间 - 平均: {df['physics_update_elapsed_time'].mean():.2f}ms, "
        f"标准差: {df['physics_update_elapsed_time'].std():.2f}ms")
  print(f"  渲染时间 - 平均: {df['render_elapsed_time'].mean():.2f}ms, "
        f"标准差: {df['render_elapsed_time'].std():.2f}ms")
  print(f"  GPU总耗时 - 平均: {df['gpu_elapsed_time'].mean():.2f}ms, "
        f"标准差: {df['gpu_elapsed_time'].std():.2f}ms")
